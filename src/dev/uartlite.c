#if CONFIG_UARTLITE
#  include <SDL/SDL.h>
#  include <stdbool.h>

#  include "device.h"

// UART
#  define Rx 0x0
#  define Tx 0x4
#  define STAT 0x8
#  define CTRL 0xC
#  define UARTLITE_SIZE 0x10

static uint32_t uartlite_ctrl_reg = 0;

/* status */
#  define SR_TX_FIFO_FULL (1 << 3)       /* transmit FIFO full */
#  define SR_TX_FIFO_EMPTY (1 << 2)      /* transmit FIFO empty  */
#  define SR_RX_FIFO_VALID_DATA (1 << 0) /* data in receive FIFO */
#  define SR_RX_FIFO_FULL (1 << 1)       /* receive FIFO full */

/* ctrl */
#  define ULITE_CONTROL_RST_TX 0x01
#  define ULITE_CONTROL_RST_RX 0x02

static uint32_t uartlite_peek(paddr_t addr, int len) {
  switch (addr) {
  case Rx: return serial_queue_top();
  case STAT: return serial_queue_is_empty() ? 0 : 1;
  case CTRL: return uartlite_ctrl_reg;
  default:
    CPUAssert(false, "uart: address(0x%08x) is not readable", addr);
    break;
  }
  return 0;
}

static uint32_t uartlite_read(paddr_t addr, int len) {
  check_ioaddr(addr, len, UARTLITE_SIZE, "serial.read");
  if (addr == Rx) return serial_dequeue();
  return uartlite_peek(addr, len);
}

static void uartlite_write(paddr_t addr, int len, uint32_t data) {
  check_ioaddr(addr, len, UARTLITE_SIZE, "serial.write");
  switch (addr) {
  case Tx:
    putchar((char)data);
    fflush(stdout);
    break;
  case CTRL: uartlite_ctrl_reg = data; break;
  default:
    CPUAssert(false, "uart: address(0x%08x) is not writable", addr);
    break;
  }
}

DEF_DEV(uartlite_dev) = {
    .name = "uartlite",
    .start = CONFIG_UARTLITE_BASE,
    .end = CONFIG_UARTLITE_BASE + UARTLITE_SIZE,
    .peek = uartlite_peek,
    .read = uartlite_read,
    .write = uartlite_write,
    .map = NULL,
};
#endif
