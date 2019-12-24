#include <stdlib.h>

#include "device.h"
#include "fifo.h"

#include "flash-m25p80.c"

#define R_DGIER (0x1c / 4)
#define R_DGIER_IE (1 << 31)

#define R_IPISR (0x20 / 4)
#define IRQ_DRR_NOT_EMPTY (1 << (31 - 23))
#define IRQ_DRR_OVERRUN (1 << (31 - 26))
#define IRQ_DRR_FULL (1 << (31 - 27))
#define IRQ_TX_FF_HALF_EMPTY (1 << 6)
#define IRQ_DTR_UNDERRUN (1 << 3)
#define IRQ_DTR_EMPTY (1 << (31 - 29))

#define R_IPIER (0x28 / 4)
#define R_SRR (0x40 / 4)
#define R_SPICR (0x60 / 4)
#define R_SPICR_TXFF_RST (1 << 5)
#define R_SPICR_RXFF_RST (1 << 6)
#define R_SPICR_MTI (1 << 8)

#define R_SPISR (0x64 / 4)
#define SR_SLAVE_MODE_SELECT (1 << 5)
#define SR_TX_FULL (1 << 3)
#define SR_TX_EMPTY (1 << 2)
#define SR_RX_FULL (1 << 1)
#define SR_RX_EMPTY (1 << 0)

#define R_SPIDTR (0x68 / 4)
#define R_SPIDRR (0x6C / 4)
#define R_SPISSR (0x70 / 4)
#define R_TX_FF_OCY (0x74 / 4)
#define R_RX_FF_OCY (0x78 / 4)
#define R_MAX (0x7C / 4)

#define SPI_SIZE (R_MAX * 4)

static Flash flash;

static uint32_t xlnx_spi_regs[R_MAX];

static fifo_type(uint8_t, 1024) spi_tx_fifo;
static fifo_type(uint8_t, 1024) spi_rx_fifo;

static void txfifo_reset() {
  fifo_reset(spi_tx_fifo);

  xlnx_spi_regs[R_SPISR] &= ~SR_TX_FULL;
  xlnx_spi_regs[R_SPISR] |= SR_TX_EMPTY;
}

static void rxfifo_reset() {
  fifo_reset(spi_rx_fifo);

  xlnx_spi_regs[R_SPISR] |= SR_RX_EMPTY;
  xlnx_spi_regs[R_SPISR] &= ~SR_RX_FULL;
}

static void xlnx_spi_update_cs() {
  for (int i = 0; i < 1; ++i) {
    m25p80_cs(&flash, 1);
    // qemu_set_irq(s->cs_lines[i], !(~xlnx_spi_regs[R_SPISSR] & 1 << i));
  }
}

static void xlnx_spi_update_irq() {
#if 0
  uint32_t pending;

  xlnx_spi_regs[R_IPISR] |=
      (!fifo_is_empty(spi_rx_fifo) ? IRQ_DRR_NOT_EMPTY : 0) |
      (fifo_is_full(spi_rx_fifo) ? IRQ_DRR_FULL : 0);

  pending = xlnx_spi_regs[R_IPISR] & xlnx_spi_regs[R_IPIER];

  pending = pending && (xlnx_spi_regs[R_DGIER] & R_DGIER_IE);
  pending = !!pending;

  /* This call lies right in the data paths so don't call the
     irq chain unless things really changed.  */
  if (pending != s->irqline) {
    s->irqline = pending;
    qemu_set_irq(s->irq, pending);
  }
#endif
}

static void xlnx_spi_do_reset() {
  memset(xlnx_spi_regs, 0, sizeof xlnx_spi_regs);

  rxfifo_reset();
  txfifo_reset();

  xlnx_spi_regs[R_SPICR] |= 0x180;
  xlnx_spi_regs[R_SPISR] |= SR_SLAVE_MODE_SELECT;
  xlnx_spi_regs[R_SPISSR] = ~0;
  xlnx_spi_update_irq();
  xlnx_spi_update_cs();
}

static inline int spi_master_enabled() {
  return !(xlnx_spi_regs[R_SPICR] & R_SPICR_MTI);
}

static void spi_flush_txfifo() {
  while (!fifo_is_empty(spi_tx_fifo)) {
    uint32_t rx = 0;
    uint32_t tx = (uint32_t)fifo_pop(spi_tx_fifo);

    if (!(xlnx_spi_regs[R_SPISSR] & 1)) rx = m25p80_transfer8(&flash, tx);

    if (fifo_is_full(spi_rx_fifo)) {
      xlnx_spi_regs[R_IPISR] |= IRQ_DRR_OVERRUN;
    } else {
      fifo_push(spi_rx_fifo, (uint8_t)rx);
      if (fifo_is_full(spi_rx_fifo)) {
        xlnx_spi_regs[R_SPISR] |= SR_RX_FULL;
        xlnx_spi_regs[R_IPISR] |= IRQ_DRR_FULL;
      }
    }

    xlnx_spi_regs[R_SPISR] &= ~SR_RX_EMPTY;
    xlnx_spi_regs[R_SPISR] &= ~SR_TX_FULL;
    xlnx_spi_regs[R_SPISR] |= SR_TX_EMPTY;

    xlnx_spi_regs[R_IPISR] |= IRQ_DTR_EMPTY;
    xlnx_spi_regs[R_IPISR] |= IRQ_DRR_NOT_EMPTY;
  }
}

static uint32_t xlnx_spi_read(uint32_t addr, int size) {
  uint32_t r = 0;

  addr >>= 2;
  switch (addr) {
  case R_SPIDRR:
    if (fifo_is_empty(spi_rx_fifo)) { return 0xdeadbeef; }

    xlnx_spi_regs[R_SPISR] &= ~SR_RX_FULL;
    r = fifo_pop(spi_rx_fifo);
    if (fifo_is_empty(spi_rx_fifo)) { xlnx_spi_regs[R_SPISR] |= SR_RX_EMPTY; }
    break;

  case R_SPISR: r = xlnx_spi_regs[addr]; break;

  default:
    if (addr < R_MAX) { r = xlnx_spi_regs[addr]; }
    break;
  }
  xlnx_spi_update_irq();
  return r;
}

static void xlnx_spi_write(uint32_t addr, int len, uint32_t data) {
  addr >>= 2;
  switch (addr) {
  case R_SRR:
    if (data == 0xa) { xlnx_spi_do_reset(); }
    break;

  case R_SPIDTR:
    xlnx_spi_regs[R_SPISR] &= ~SR_TX_EMPTY;
    fifo_push(spi_tx_fifo, (uint8_t)data);
    if (fifo_is_full(spi_tx_fifo)) { xlnx_spi_regs[R_SPISR] |= SR_TX_FULL; }
    if (!spi_master_enabled()) { goto done; }
    spi_flush_txfifo();
    break;

  case R_SPISR: break;

  case R_IPISR:
    /* Toggle the bits.  */
    xlnx_spi_regs[addr] ^= data;
    break;

  /* Slave Select Register.  */
  case R_SPISSR:
    xlnx_spi_regs[addr] = data;
    xlnx_spi_update_cs();
    break;

  case R_SPICR:
    /* FIXME: reset irq and sr state to empty queues.  */
    if (data & R_SPICR_RXFF_RST) { rxfifo_reset(); }

    if (data & R_SPICR_TXFF_RST) { txfifo_reset(); }
    data &= ~(R_SPICR_RXFF_RST | R_SPICR_TXFF_RST);
    xlnx_spi_regs[addr] = data;

    if (!(data & R_SPICR_MTI)) { spi_flush_txfifo(); }
    break;

  default:
    if (addr < R_MAX) { xlnx_spi_regs[addr] = data; }
    break;
  }

done:
  xlnx_spi_update_irq();
}

static void xlnx_spi_init() {
  m25p80_init(&flash);

  xlnx_spi_do_reset();
}

DEF_DEV(xlnx_spi_dev) = {
    .name = "xilinx-spi",
    .start = CONFIG_XLNX_SPI_BASE,
    .size = SPI_SIZE,
    .init = xlnx_spi_init,
    .read = xlnx_spi_read,
    .write = xlnx_spi_write,
};
