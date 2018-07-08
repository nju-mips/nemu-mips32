#include "nemu.h"

/* serial port */
#define SERIAL_PORT ((volatile char *)0x40001000)
#define Rx 0x0
#define Tx 0x04
#define STAT 0x08
#define CTRL 0x0c

#define KEYDOWN_MASK 0x8000

#define KEY_QUEUE_LEN 1024
static int key_queue[KEY_QUEUE_LEN];
static int key_f = 0, key_r = 0;

void serial_enqueue(char ch) {
  int next = (key_r + 1) % KEY_QUEUE_LEN;
  if(next != key_f) { // if not full
	key_queue[key_r] = ch;
	key_r = next;
  }
}

#define check_uartlite(addr, len) \
  Assert(addr >= 0 && addr <= STAT, \
	  "address(0x%08x) is out side UARTLite", addr); \
	  Assert(len == 1, \
		  "UARTLite only allow byte read/write");

uint32_t uartlite_read(paddr_t addr, int len) {
  /* CTRL not yet implemented, only allow byte read/write */
  check_uartlite(addr, len);
  switch (addr) {
	case Rx: {
	  char ch = key_queue[key_f];
	  key_f = (key_f + 1) % KEY_QUEUE_LEN;
	  return ch;
	}
	case STAT:
	  return key_f == key_r ? 0 : 1;
	default:
	  Assert(false, "UARTLite: address(0x%08x) is not readable", addr);
	  break;
  }
}

void uartlite_write(paddr_t addr, int len, uint32_t data) {
  check_uartlite(addr, len);
  switch (addr) {
	case Tx:
	  putchar((char)data);
	  break;
	default:
	  Assert(false, "UARTLite: address(0x%08x) is not writable", addr);
	  break;
  }
}

