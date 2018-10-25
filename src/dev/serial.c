#include <SDL/SDL.h>
#include <stdbool.h>

#include "nemu.h"
#include "device.h"

#define UARTLITE_Rx     0x0
#define UARTLITE_Tx     0x4
#define UARTLITE_STAT   0x8
#define UARTLITE_CTRL   0xC

static uint32_t uartlite_ctrl_reg = 0;

/* status */
#define SR_TX_FIFO_FULL		(1 << 3) /* transmit FIFO full */
#define SR_TX_FIFO_EMPTY	(1 << 2) /* transmit FIFO empty */
#define SR_RX_FIFO_VALID_DATA	(1 << 0) /* data in receive FIFO */
#define SR_RX_FIFO_FULL		(1 << 1) /* receive FIFO full */

/* ctrl */
#define ULITE_CONTROL_RST_TX	0x01
#define ULITE_CONTROL_RST_RX	0x02

/////////////////////////////////////////////////////////////////
//                      serial simulation                      //
/////////////////////////////////////////////////////////////////

#define SERIAL_QUEUE_LEN 1024
static int serial_queue[SERIAL_QUEUE_LEN];
static int serial_f = 0, serial_r = 0;


// wait to be completed
const char *SDLK_to_ascii(SDLKey key) {
  switch(key) {
	case SDLK_ESCAPE: return "\e";
	case SDLK_F1: return "\eOP"; case SDLK_F2: return "\eOQ";
	case SDLK_F3: return "\eOR"; case SDLK_F4: return "\eOS";
	case SDLK_F5: return "\e[15~"; case SDLK_F6: return "\e[17~";
	case SDLK_F7: return "\e[18~"; case SDLK_F8: return "\e[19~";
	case SDLK_F9: return "\e[20~"; case SDLK_F10: return "\e[21~";
	case SDLK_F11: return "\e[22~"; case SDLK_F12: return "\e[24~";
	case SDLK_BACKQUOTE: return "`";
	case SDLK_1: return "1"; case SDLK_2: return "2"; case SDLK_3: return "3";
	case SDLK_4: return "4"; case SDLK_5: return "5"; case SDLK_6: return "6";
	case SDLK_7: return "7"; case SDLK_8: return "8"; case SDLK_9: return "9";
	case SDLK_0: return "0";
	case SDLK_MINUS: return "-";
	case SDLK_EQUALS: return "=";
	case SDLK_BACKSPACE: return "\x8";
	case SDLK_TAB: return "\t";
	case SDLK_q: return "q";
	case SDLK_w: return "w";
	case SDLK_e: return "e";
	case SDLK_r: return "r";
	case SDLK_t: return "t";
	case SDLK_y: return "y";
	case SDLK_u: return "u";
	case SDLK_i: return "i";
	case SDLK_o: return "o";
	case SDLK_p: return "p";
	case SDLK_LEFTBRACKET: return "{";
	case SDLK_RIGHTBRACKET: return "}";
	case SDLK_SLASH: return "\\";
	case SDLK_a: return "a";
	case SDLK_s: return "s";
	case SDLK_d: return "d";
	case SDLK_f: return "f";
	case SDLK_g: return "g";
	case SDLK_h: return "h";
	case SDLK_j: return "j";
	case SDLK_k: return "k";
	case SDLK_l: return "l";
	case SDLK_SEMICOLON: return ";";
	case SDLK_QUOTE: return "'";
	case SDLK_RETURN: return "\n";
	case SDLK_LSHIFT: return ""; // cannot be escaped
	case SDLK_z: return "z";
	case SDLK_x: return "x";
	case SDLK_c: return "c";
	case SDLK_v: return "v";
	case SDLK_b: return "b";
	case SDLK_n: return "n";
	case SDLK_m: return "m";
	case SDLK_COMMA: return ";";
	case SDLK_PERIOD: return ".";
	case SDLK_BACKSLASH: return "/";
	case SDLK_RSHIFT: return ""; // cannot be escaped
	case SDLK_LCTRL: return ""; // cannot be escaped
	case SDLK_LALT: return ""; // cannot be escaped
	case SDLK_SPACE: return " ";
	case SDLK_RALT: return ""; // cannot be escaped
	case SDLK_RCTRL: return ""; // cannot be escaped
	case SDLK_INSERT: return "\e[2~";
	case SDLK_HOME: return "\e[1~";
	case SDLK_PAGEUP: return "\e[5~";
	case SDLK_DELETE: return "\x7f";
	case SDLK_END: return "\e[4~";
	case SDLK_PAGEDOWN: return "\e[6~";
	case SDLK_UP: return "\e[A";
	case SDLK_LEFT: return "\e[D";
	case SDLK_DOWN: return "\e[B";
	case SDLK_RIGHT: return "\e[C";
	case SDLK_KP_DIVIDE: return "/";
	case SDLK_KP_MULTIPLY: return "*";
	case SDLK_KP_MINUS: return "-";
	case SDLK_KP_PLUS: return "+";
	case SDLK_KP7: return "7"; case SDLK_KP8: return "8"; case SDLK_KP9: return "9";
	case SDLK_KP4: return "4"; case SDLK_KP5: return "5"; case SDLK_KP6: return "6";
	case SDLK_KP1: return "1"; case SDLK_KP2: return "2"; case SDLK_KP3: return "3";
	case SDLK_KP0: return "0";
	case SDLK_KP_EQUALS: return "=";
	case SDLK_KP_ENTER: return "\n";
	default: return "";
  }
};

void serial_enqueue_ascii(char ch) {
  int next = (serial_r + 1) % SERIAL_QUEUE_LEN;
  if(next != serial_f) { // if not full
	serial_queue[serial_r] = ch;
	serial_r = next;
  }
}

void serial_enqueue(SDL_EventType type, SDLKey key) {
  if(key < 0 || key >= SDLK_LAST) return;

  static bool sdlk_state[SDLK_LAST];
  // bool shift = false;
  if(type == SDL_KEYDOWN) {
	if(sdlk_state[(int)key])
	  return;
	sdlk_state[(int)key] = true;
  }

  if(type == SDL_KEYUP) {
	sdlk_state[(int)key] = false;
	return;
  }

  const char *p = SDLK_to_ascii(key);
  while(p && *p) {
	int next = (serial_r + 1) % SERIAL_QUEUE_LEN;
	if(next != serial_f) { // if not full
	  serial_queue[serial_r] = *p;
	  serial_r = next;
	} else {
	  break;
	}
	p ++;
  }
}

#define check_input(addr, len) \
  CoreAssert(addr >= 0 && addr <= SERIAL_SIZE, \
	  "UART: address(0x%08x) is out side", addr); \

uint32_t serial_peek(paddr_t addr, int len) {
  check_input(addr, len);
  switch (addr) {
	case UARTLITE_Rx:
	  return serial_queue[serial_f];
	case UARTLITE_STAT:
	  return serial_f == serial_r ? 0 : 1;
	case UARTLITE_CTRL:
	  return uartlite_ctrl_reg;
	default:
	  CoreAssert(false, "uart: address(0x%08x) is not readable", addr);
	  break;
  }
  return 0;
}

uint32_t serial_read(paddr_t addr, int len) {
  check_input(addr, len);
  uint32_t data = serial_peek(addr, len);
  if(addr == UARTLITE_Rx && serial_f != serial_r)
	serial_f = (serial_f + 1) % SERIAL_QUEUE_LEN;
  return data;
}

void serial_write(paddr_t addr, int len, uint32_t data) {
  check_input(addr, len);
  switch (addr) {
	case UARTLITE_Tx:
	  putchar((char)data);
	  fflush(stdout);
	  break;
	case UARTLITE_CTRL:
	  uartlite_ctrl_reg = data;
	  break;
	default:
	  CoreAssert(false, "uart: address(0x%08x) is not writable", addr);
	  break;
  }
}

