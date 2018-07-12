#include "nemu.h"
#include <SDL/SDL.h>

/* serial port */
#define SERIAL_PORT ((volatile char *)0x40001000)
#define Rx 0x0
#define Tx 0x04
#define STAT 0x08
#define CTRL 0x0c
#define SCANCODE 0x10
#define SCANCODE_STAT 0x14

#define SERIAL_QUEUE_LEN 1024
static int serial_queue[SERIAL_QUEUE_LEN];
static int serial_f = 0, serial_r = 0;

void serial_enqueue(char ch) {
  int next = (serial_r + 1) % SERIAL_QUEUE_LEN;
  if(next != serial_f) { // if not full
	serial_queue[serial_r] = ch;
	serial_r = next;
  }
}


#define KEYBOARD_QUEUE_LEN 1024
static int keyboard_queue[KEYBOARD_QUEUE_LEN];
static int keyboard_f = 0, keyboard_r = 0;


struct {
  int downcode, upcode;
} SDLK_to_scancode[SDLK_LAST] = {
	[SDLK_ESCAPE]	= { 0x76, 0xF076 },
	[SDLK_F1]	= { 0x05, 0xF005 },
	[SDLK_F2]	= { 0x06, 0xF006 },
	[SDLK_F3]	= { 0x04, 0xF004 },
	[SDLK_F4]	= { 0x0C, 0xF00C },
	[SDLK_F5]	= { 0x03, 0xF003 },
	[SDLK_F6]	= { 0x0B, 0xF00B },
	[SDLK_F7]	= { 0x83, 0xF083 },
	[SDLK_F8]	= { 0x0A, 0xF00A },
	[SDLK_F9]	= { 0x01, 0xF001 },
	[SDLK_F10]	= { 0x09, 0xF009 },
	[SDLK_F11]	= { 0x78, 0xF078 },
	[SDLK_F12]	= { 0x07, 0xF007 },
	[SDLK_SCROLLOCK]	= { 0x7E, 0xF07E },
	[SDLK_BACKQUOTE]	= { 0x0E, 0xF00E },
	[SDLK_1]	= { 0x16, 0xF016 },
	[SDLK_2]	= { 0x1E, 0xF01E },
	[SDLK_3]	= { 0x26, 0xF026 },
	[SDLK_4]	= { 0x25, 0xF025 },
	[SDLK_5]	= { 0x2E, 0xF02E },
	[SDLK_6]	= { 0x36, 0xF036 },
	[SDLK_7]	= { 0x3D, 0xF03D },
	[SDLK_8]	= { 0x3E, 0xF03E },
	[SDLK_9]	= { 0x46, 0xF046 },
	[SDLK_0]	= { 0x45, 0xF045 },
	[SDLK_MINUS]	= { 0x4E, 0xF04E },
	[SDLK_EQUALS]	= { 0x55, 0xF055 },
	[SDLK_BACKSPACE]	= { 0x66, 0xF066 },
	[SDLK_TAB]	= { 0x0D, 0xF00D },
	[SDLK_q]	= { 0x15, 0xF015 },
	[SDLK_w]	= { 0x1D, 0xF01D },
	[SDLK_e]	= { 0x24, 0xF024 },
	[SDLK_r]	= { 0x2D, 0xF02D },
	[SDLK_t]	= { 0x2C, 0xF02C },
	[SDLK_y]	= { 0x35, 0xF035 },
	[SDLK_u]	= { 0x3C, 0xF03C },
	[SDLK_i]	= { 0x43, 0xF043 },
	[SDLK_o]	= { 0x44, 0xF044 },
	[SDLK_p]	= { 0x4D, 0xF04D },
	[SDLK_LEFTBRACKET]	= { 0x54, 0xF054 },
	[SDLK_RIGHTBRACKET]	= { 0x5B, 0xF05B },
	[SDLK_SLASH]	= { 0x5D, 0xF05D },
	[SDLK_CAPSLOCK]	= { 0x58, 0xF058 },
	[SDLK_a]	= { 0x1C, 0xF01C },
	[SDLK_s]	= { 0x1B, 0xF01B },
	[SDLK_d]	= { 0x23, 0xF023 },
	[SDLK_f]	= { 0x2B, 0xF02B },
	[SDLK_g]	= { 0x34, 0xF034 },
	[SDLK_h]	= { 0x33, 0xF033 },
	[SDLK_j]	= { 0x3B, 0xF03B },
	[SDLK_k]	= { 0x42, 0xF042 },
	[SDLK_l]	= { 0x4B, 0xF04B },
	[SDLK_SEMICOLON]	= { 0x4C, 0xF04C },
	[SDLK_QUOTE]	= { 0x52, 0xF052 },
	[SDLK_RETURN]	= { 0x5A, 0xF05A },
	[SDLK_LSHIFT]	= { 0x12, 0xF012 },
	[SDLK_z]	= { 0x1A, 0xF01A },
	[SDLK_x]	= { 0x22, 0xF022 },
	[SDLK_c]	= { 0x21, 0xF021 },
	[SDLK_v]	= { 0x2A, 0xF02A },
	[SDLK_b]	= { 0x32, 0xF032 },
	[SDLK_n]	= { 0x31, 0xF031 },
	[SDLK_m]	= { 0x3A, 0xF03A },
	[SDLK_COMMA]	= { 0x41, 0xF041 },
	[SDLK_PERIOD]	= { 0x49, 0xF049 },
	[SDLK_BACKSLASH]	= { 0x4A, 0xF04A },
	[SDLK_RSHIFT]	= { 0x59, 0xF059 },
	[SDLK_LCTRL]	= { 0x14, 0xF014 },
	[SDLK_LALT]	= { 0x11, 0xF011 },
	[SDLK_SPACE]	= { 0x29, 0xF029 },
	[SDLK_RALT]	= { 0xE011, 0xE0F011 },
	[SDLK_RCTRL]	= { 0xE014, 0xE0F014 },
	[SDLK_INSERT]	= { 0xE070, 0xE0F070 },
	[SDLK_HOME]	= { 0xE06C, 0xE0F06C },
	[SDLK_PAGEUP]	= { 0xE07D, 0xE0F07D },
	[SDLK_DELETE]	= { 0xE071, 0xE0F071 },
	[SDLK_END]	= { 0xE069, 0xE0F069 },
	[SDLK_PAGEDOWN]	= { 0xE07A, 0xE0F07A },
	[SDLK_UP]	= { 0xE075, 0xE0F075 },
	[SDLK_LEFT]	= { 0xE06B, 0xE0F06B },
	[SDLK_DOWN]	= { 0xE072, 0xE0F072 },
	[SDLK_RIGHT]	= { 0xE074, 0xE0F074 },
	[SDLK_NUMLOCK]	= { 0x77, 0xF077 },
	[SDLK_KP_DIVIDE]	= { 0xE04A, 0xE0F04A },
	[SDLK_KP_MULTIPLY]	= { 0x7C, 0xF07C },
	[SDLK_KP_MINUS]	= { 0x7B, 0xF07B },
	[SDLK_KP_PLUS]	= { 0x79, 0xF079 },
	[SDLK_KP7]	= { 0x6C, 0xF06C },
	[SDLK_KP8]	= { 0x75, 0xF075 },
	[SDLK_KP9]	= { 0x7D, 0xF07D },
	[SDLK_KP4]	= { 0x6B, 0xF06B },
	[SDLK_KP5]	= { 0x73, 0xF073 },
	[SDLK_KP6]	= { 0x74, 0xF074 },
	[SDLK_KP1]	= { 0x69, 0xF069 },
	[SDLK_KP2]	= { 0x72, 0xF072 },
	[SDLK_KP3]	= { 0x7A, 0xF07A },
	[SDLK_KP0]	= { 0x70, 0xF070 },
	[SDLK_KP_EQUALS]	= { 0x71, 0xF071 },
	[SDLK_KP_ENTER]	= { 0xE05A, 0xE0F05A },
};

void keyboard_enqueue(SDL_KeyboardEvent *key) {
  if(key->keysym.sym >= SDLK_LAST) return;
  int scancode = key->type == SDL_KEYUP ?
	SDLK_to_scancode[key->keysym.sym].upcode :
	SDLK_to_scancode[key->keysym.sym].downcode;

  if(scancode == 0) return;

  int next = (keyboard_r + 1) % KEYBOARD_QUEUE_LEN;
  if(next != keyboard_f) { // if not full
	keyboard_queue[keyboard_r] = scancode;
	keyboard_r = next;
  }
}

#define check_input(addr, len) \
  Assert(addr >= 0 && addr <= SCANCODE_STAT, \
	  "input: address(0x%08x) is out side", addr); \
	  Assert(len == 1 || len == 4, "input only allow byte read/write");

uint32_t input_read(paddr_t addr, int len) {
  /* CTRL not yet implemented, only allow byte read/write */
  check_input(addr, len);
  switch (addr) {
	case SCANCODE_STAT:
	  return keyboard_f == keyboard_r ? 0 : 1;
	case SCANCODE: {
	  uint32_t code = keyboard_queue[keyboard_f];
	  keyboard_f = (keyboard_f + 1) % KEYBOARD_QUEUE_LEN;
	  return code;
	}
	case Rx: {
	  char ch = serial_queue[serial_f];
	  serial_f = (serial_f + 1) % SERIAL_QUEUE_LEN;
	  return ch;
	}
	case STAT:
	  return serial_f == serial_r ? 0 : 1;
	default:
	  Assert(false, "input: address(0x%08x) is not readable", addr);
	  break;
  }
  return 0;
}

void input_write(paddr_t addr, int len, uint32_t data) {
  check_input(addr, len);
  switch (addr) {
	case Tx:
	  putchar((char)data);
	  break;
	default:
	  Assert(false, "input: address(0x%08x) is not writable", addr);
	  break;
  }
}

