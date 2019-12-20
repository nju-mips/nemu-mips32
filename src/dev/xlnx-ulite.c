#include <SDL/SDL.h>
#include <stdbool.h>

#include "device.h"
#include "events.h"
#include "queue.h"

// UART
#define Rx 0x0
#define Tx 0x4
#define STAT 0x8
#define CTRL 0xC
#define XLNX_ULITE_SIZE 0x10

#define XLNX_ULITE_IRQ_NO 4

#define SR_CTRL_INTR_BIT (1 << 4)

/* status */
#define SR_TX_FIFO_FULL (1 << 3)       /* transmit FIFO full */
#define SR_TX_FIFO_EMPTY (1 << 2)      /* transmit FIFO empty  */
#define SR_RX_FIFO_VALID_DATA (1 << 0) /* data in receive FIFO */
#define SR_RX_FIFO_FULL (1 << 1)       /* receive FIFO full */

/* ctrl */
#define ULITE_CONTROL_RST_TX 0x01
#define ULITE_CONTROL_RST_RX 0x02

/* ulite queue */
static queue_type(int, 1024) ulite_q;

void xlnx_ulite_enqueue(int ch) { queue_add(ulite_q, ch); }
int xlnx_ulite_dequeue() { return queue_pop(ulite_q); }
int xlnx_ulite_queue_top() { return queue_top(ulite_q); }
bool xlnx_ulite_queue_is_full() { return queue_is_full(ulite_q); }
bool xlnx_ulite_queue_is_empty() { return queue_is_empty(ulite_q); }

/* ulite queue end */

static uint32_t xlnx_ulite_intr_enabled = 0;
static uint32_t xlnx_ulite_tx_fifo_empty = 0;

static const char *xlnx_ulite_stop_string = NULL;
static const char *xlnx_ulite_stop_string_ptr = NULL;

void xlnx_ulite_set_irq() {
#if CONFIG_INTR
  if (xlnx_ulite_intr_enabled) { set_irq(XLNX_ULITE_IRQ_NO); }
#endif
}

void stop_cpu_when_ulite_send(const char *string) {
  xlnx_ulite_stop_string = string;
  xlnx_ulite_stop_string_ptr = string;
}

static void stop_cpu_check(char ch) {
  if (!xlnx_ulite_stop_string_ptr) return;

  if (*xlnx_ulite_stop_string_ptr == ch) {
    xlnx_ulite_stop_string_ptr++;
    if (*xlnx_ulite_stop_string_ptr == 0) {
      eprintf("ulite recv '%s', stop the cpu\n", xlnx_ulite_stop_string);
      print_frames();
      print_backtrace();
      nemu_state = NEMU_STOP;
    }
  } else {
    xlnx_ulite_stop_string_ptr = xlnx_ulite_stop_string;
  }
}

static uint32_t xlnx_ulite_peek(paddr_t addr, int len) {
  switch (addr) {
  case Rx: return xlnx_ulite_queue_top();
  case STAT: {
    uint32_t status = 0;
    if (!xlnx_ulite_queue_is_empty()) status |= SR_RX_FIFO_VALID_DATA;
    if (xlnx_ulite_intr_enabled) status |= SR_CTRL_INTR_BIT;
    if (xlnx_ulite_tx_fifo_empty) status |= SR_TX_FIFO_EMPTY;
    return status;
  } break;
  case CTRL: return 0;
  default:
    CPUAssert(false, "uart: address(0x%08x) is not readable", addr);
    break;
  }
  return 0;
}

static uint32_t xlnx_ulite_read(paddr_t addr, int len) {
  check_ioaddr(addr, len, XLNX_ULITE_SIZE, "ulite.read");
  if (addr == Rx) {
    clear_irq(XLNX_ULITE_IRQ_NO);
    return xlnx_ulite_dequeue();
  }
  return xlnx_ulite_peek(addr, len);
}

static void xlnx_ulite_write(paddr_t addr, int len, uint32_t data) {
  check_ioaddr(addr, len, XLNX_ULITE_SIZE, "ulite.write");
  switch (addr) {
  case Tx:
    putchar((char)data);
    stop_cpu_check(data);
    fflush(stdout);
    if ((char)data == '\n') {
      xlnx_ulite_tx_fifo_empty |= SR_TX_FIFO_EMPTY;
      // xlnx_ulite_set_irq();
    }
    break;
  case CTRL:
    xlnx_ulite_intr_enabled = data & SR_CTRL_INTR_BIT;
    if (!xlnx_ulite_intr_enabled) {
      xlnx_ulite_tx_fifo_empty = 0;
      clear_irq(XLNX_ULITE_IRQ_NO);
    }
    break;
  default:
    CPUAssert(false, "uart: address(0x%08x) is not writable", addr);
    break;
  }
}

static void xlnx_ulite_on_data(void *data, int len) {
  for (int i = 0; i < len; i++) { xlnx_ulite_enqueue(((char *)data)[i]); }
  xlnx_ulite_set_irq();
}

static void xlnx_ulite_init();

DEF_DEV(xlnx_ulite_dev) = {
    .name = "xilinx-uartlite",
    .init = xlnx_ulite_init,
    .start = CONFIG_XLNX_ULITE_BASE,
    .end = CONFIG_XLNX_ULITE_BASE + XLNX_ULITE_SIZE,
    .peek = xlnx_ulite_peek,
    .read = xlnx_ulite_read,
    .write = xlnx_ulite_write,
    .on_data = xlnx_ulite_on_data,
    .map = NULL,
};

void prepare_ulite_contents() {
  /* send command to uboot */
#if CONFIG_PRELOAD_LINUX
  char cmd[512], *p = cmd;
#  if 1
  p += sprintf(p, "bootm 0x%08x\n", CONFIG_KERNEL_UIMAGE_BASE);
#  else
  extern const char *iface_ipaddr, *iface_gw;
  p += sprintf(p, "set serverip 114.212.81.121\n");
  p += sprintf(p, "set gateway %s\n", iface_gw);
  p += sprintf(p, "set ipaddr %s\n", iface_ipaddr);
  p += sprintf(p, "tftpboot vmlinux\n");
  p += sprintf(p, "ping 114.212.81.121\n");
#  endif
  assert(p < &cmd[sizeof(cmd)]);
  for (p = cmd; *p; p++) xlnx_ulite_enqueue(*p);
#endif
}

static void xlnx_ulite_init() {
  event_add_handler(EVENT_CTRL_C, xlnx_ulite_on_data);
  event_add_handler(EVENT_CTRL_Z, xlnx_ulite_on_data);
  event_add_handler(EVENT_STDIN_DATA, xlnx_ulite_on_data);
  prepare_ulite_contents();
}
