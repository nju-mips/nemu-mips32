#if CONFIG_UARTLITE
#  include <SDL/SDL.h>
#  include <stdbool.h>

#  include "device.h"
#  include "events.h"

// UART
#  define Rx 0x0
#  define Tx 0x4
#  define STAT 0x8
#  define CTRL 0xC
#  define ULITE_SIZE 0x10

#  define ULITE_IRQ_NO 4

#  define SR_CTRL_INTR_BIT (1 << 4)

/* status */
#  define SR_TX_FIFO_FULL (1 << 3)       /* transmit FIFO full */
#  define SR_TX_FIFO_EMPTY (1 << 2)      /* transmit FIFO empty  */
#  define SR_RX_FIFO_VALID_DATA (1 << 0) /* data in receive FIFO */
#  define SR_RX_FIFO_FULL (1 << 1)       /* receive FIFO full */

/* ctrl */
#  define ULITE_CONTROL_RST_TX 0x01
#  define ULITE_CONTROL_RST_RX 0x02

/* ulite queue */
#  define ULITE_QUEUE_LEN 1024
static int ulite_queue[ULITE_QUEUE_LEN];
static int ulite_f = 0, ulite_r = 0;

void ulite_enqueue(int ch) {
  int next = (ulite_r + 1) % ULITE_QUEUE_LEN;
  if (next != ulite_f) { // if not full
    ulite_queue[ulite_r] = ch;
    ulite_r = next;
  }
}

int ulite_dequeue() {
  int data = ulite_queue[ulite_f];
  if (ulite_f != ulite_r) ulite_f = (ulite_f + 1) % ULITE_QUEUE_LEN;
  return data;
}

int ulite_queue_top() { return ulite_queue[ulite_f]; }

bool ulite_queue_is_full() {
  int next = (ulite_r + 1) % ULITE_QUEUE_LEN;
  return next != ulite_f;
}

bool ulite_queue_is_empty() { return ulite_f == ulite_r; }

/* ulite queue end */

static uint32_t ulite_intr_enabled = 0;
static uint32_t ulite_tx_fifo_empty = 0;

static const char *ulite_stop_string = NULL;
static const char *ulite_stop_string_ptr = NULL;

void ulite_set_irq() {
#  if CONFIG_INTR
  if (ulite_intr_enabled) { set_irq(ULITE_IRQ_NO); }
#  endif
}

void stop_cpu_when_ulite_send(const char *string) {
  ulite_stop_string = string;
  ulite_stop_string_ptr = string;
}

static void stop_cpu_check(char ch) {
  if (!ulite_stop_string_ptr) return;

  if (*ulite_stop_string_ptr == ch) {
    ulite_stop_string_ptr++;
    if (*ulite_stop_string_ptr == 0) {
      eprintf("ulite recv '%s', stop the cpu\n", ulite_stop_string);
      print_frames();
      print_backtrace();
      nemu_state = NEMU_STOP;
    }
  } else {
    ulite_stop_string_ptr = ulite_stop_string;
  }
}

static uint32_t ulite_peek(paddr_t addr, int len) {
  switch (addr) {
  case Rx: return ulite_queue_top();
  case STAT: {
    uint32_t status = 0;
    if (!ulite_queue_is_empty()) status |= SR_RX_FIFO_VALID_DATA;
    if (ulite_intr_enabled) status |= SR_CTRL_INTR_BIT;
    if (ulite_tx_fifo_empty) status |= SR_TX_FIFO_EMPTY;
    return status;
  } break;
  case CTRL: return 0;
  default:
    CPUAssert(false, "uart: address(0x%08x) is not readable", addr);
    break;
  }
  return 0;
}

static uint32_t ulite_read(paddr_t addr, int len) {
  check_ioaddr(addr, len, ULITE_SIZE, "ulite.read");
  if (addr == Rx) {
    clear_irq(ULITE_IRQ_NO);
    return ulite_dequeue();
  }
  return ulite_peek(addr, len);
}

static void ulite_write(paddr_t addr, int len, uint32_t data) {
  check_ioaddr(addr, len, ULITE_SIZE, "ulite.write");
  switch (addr) {
  case Tx:
    putchar((char)data);
    stop_cpu_check(data);
    fflush(stdout);
    if ((char)data == '\n') {
      ulite_tx_fifo_empty |= SR_TX_FIFO_EMPTY;
      // ulite_set_irq();
    }
    break;
  case CTRL:
    ulite_intr_enabled = data & SR_CTRL_INTR_BIT;
    if (!ulite_intr_enabled) {
      ulite_tx_fifo_empty = 0;
      clear_irq(ULITE_IRQ_NO);
    }
    break;
  default:
    CPUAssert(false, "uart: address(0x%08x) is not writable", addr);
    break;
  }
}

static void ulite_on_data(void *data, int len) {
  for (int i = 0; i < len; i++) { ulite_enqueue(((char *)data)[i]); }
  ulite_set_irq();
}

static void ulite_init();

DEF_DEV(ulite_dev) = {
    .name = "ulite",
    .init = ulite_init,
    .start = CONFIG_UARTLITE_BASE,
    .end = CONFIG_UARTLITE_BASE + ULITE_SIZE,
    .peek = ulite_peek,
    .read = ulite_read,
    .write = ulite_write,
    .on_data = ulite_on_data,
    .map = NULL,
};

void prepare_ulite_contents() {
  /* send command to uboot */
#if CONFIG_PRELOAD_LINUX
  char cmd[512], *p = cmd;
#if 1
  p += sprintf(p, "bootm 0x%08x\n", CONFIG_KERNEL_UIMAGE_BASE);
#else
  p += sprintf(p, "set serverip 192.168.3.1\n");
  p += sprintf(p, "set ipaddr 114.212.81.241\n");
  p += sprintf(p, "tftpboot litenes-mips32-npc.elf\n");
  p += sprintf(p, "ping 127.0.0.1\n");
#endif
  assert (p < &cmd[sizeof(cmd)]);
  for (p = cmd; *p; p++) ulite_enqueue(*p);
#endif
}

static void ulite_init() {
  event_add_handler(EVENT_CTRL_C, ulite_on_data);
  event_add_handler(EVENT_CTRL_Z, ulite_on_data);
  event_add_handler(EVENT_STDIN_DATA, ulite_on_data);
  prepare_ulite_contents();
}

#endif
