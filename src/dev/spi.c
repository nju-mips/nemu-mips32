#include <SDL/SDL.h>
#include <stdbool.h>

#include "common.h"
#include "nemu.h"
#include "device.h"

#define SPI_ADDR 0x1fe80000
#define SPI_SIZE 0x1000

#if 0
/* SPI Control Register (spicr), [1] p9, [2] p8 */
#define SPICR_LSB_FIRST		BIT(9)
#define SPICR_MASTER_INHIBIT	BIT(8)
#define SPICR_MANUAL_SS		BIT(7)
#define SPICR_RXFIFO_RESEST	BIT(6)
#define SPICR_TXFIFO_RESEST	BIT(5)
#define SPICR_CPHA		BIT(4)
#define SPICR_CPOL		BIT(3)
#define SPICR_MASTER_MODE	BIT(2)
#define SPICR_SPE		BIT(1)
#define SPICR_LOOP		BIT(0)

/* SPI Status Register (spisr), [1] p11, [2] p10 */
#define SPISR_SLAVE_MODE_SELECT	BIT(5)
#define SPISR_MODF		BIT(4)
#define SPISR_TX_FULL		BIT(3)
#define SPISR_TX_EMPTY		BIT(2)
#define SPISR_RX_FULL		BIT(1)
#define SPISR_RX_EMPTY		BIT(0)

/* SPI Data Transmit Register (spidtr), [1] p12, [2] p12 */
#define SPIDTR_8BIT_MASK	GENMASK(7, 0)
#define SPIDTR_16BIT_MASK	GENMASK(15, 0)
#define SPIDTR_32BIT_MASK	GENMASK(31, 0)

/* SPI Data Receive Register (spidrr), [1] p12, [2] p12 */
#define SPIDRR_8BIT_MASK	GENMASK(7, 0)
#define SPIDRR_16BIT_MASK	GENMASK(15, 0)
#define SPIDRR_32BIT_MASK	GENMASK(31, 0)

/* SPI Slave Select Register (spissr), [1] p13, [2] p13 */
#define SPISSR_MASK(cs)		(1 << (cs))
#define SPISSR_ACT(cs)		~SPISSR_MASK(cs)
#define SPISSR_OFF		~0UL

/* SPI Software Reset Register (ssr) */
#define SPISSR_RESET_VALUE	0x0a

#define XILSPI_MAX_XFER_BITS	8
#define XILSPI_SPICR_DFLT_ON	(SPICR_MANUAL_SS | SPICR_MASTER_MODE | \
				SPICR_SPE | SPICR_MASTER_INHIBIT)
#define XILSPI_SPICR_DFLT_OFF	(SPICR_MASTER_INHIBIT | SPICR_MANUAL_SS)

#ifndef CONFIG_XILINX_SPI_IDLE_VAL
#define CONFIG_XILINX_SPI_IDLE_VAL	GENMASK(7, 0)
#endif

#define XILINX_SPI_QUAD_MODE	2

#define XILINX_SPI_QUAD_EXTRA_DUMMY	3
#define SPI_QUAD_OUT_FAST_READ		0x6B

#define SRR     0x40  // write
#define SPICR   0x60  // rw,  0x180
#define SPISR   0x64  // r,   0x25
#define SPIDTR  0x68  // w
#define SPIDRR  0x6c  // r
#define SPISSR  0x70  // rw
#define SPITFOR 0x74  // r
#define SPIRFOR 0x78  // r

struct xilinx_spi_regs {
	u32 __space0__[7];
	u32 dgier;	/* Device Global Interrupt Enable Register (DGIER) */
	u32 ipisr;	/* IP Interrupt Status Register (IPISR) */
	u32 __space1__;
	u32 ipier;	/* IP Interrupt Enable Register (IPIER) */
	u32 __space2__[5];

	u32 srr;	/* !!  Softare Reset Register (SRR) */

	u32 __space3__[7];

	u32 spicr;	/* !!  SPI Control Register (SPICR) */
	u32 spisr;	/* !!  SPI Status Register (SPISR) */
	u32 spidtr;	/* !!  SPI Data Transmit Register (SPIDTR) */
	u32 spidrr;	/* !!  SPI Data Receive Register (SPIDRR) */
	u32 spissr;	/* !!  SPI Slave Select Register (SPISSR) */

	u32 spitfor;	/* SPI Transmit FIFO Occupancy Register (SPITFOR) */
	u32 spirfor;	/* SPI Receive FIFO Occupancy Register (SPIRFOR) */
};

static int flash;
static struct xilinx_spi_regs spi_regs;

void init_spi(const char *filename) {
  flash = open(filename, O_RDONLY);
  Assert(flash > 0, "failed to open '%s' as flash\n", filename);

  spi_regs.spicr = 0x180;
  spi_regs.spisr = 0x25;
}

uint32_t ddr_read(paddr_t addr, int len) {
  check_ddr(addr, len);
  return *((uint32_t *)((uint8_t *)ddr + addr)) & (~0u >> ((4 - len) << 3));
}

void ddr_write(paddr_t addr, int len, uint32_t data) {
  switch(addr) {
	case SRR:    spi_regs.srr = data;    break;
	case SPICR:  spi_regs.spicr = data;  break;
	case SPIDTR: spi_regs.spidtr = data; break; // data transfer
	case SPISSR: spi_regs.spissr = data; break;
  }
}
#endif
