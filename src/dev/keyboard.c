#include <SDL/SDL.h>
#include <stdbool.h>

#include "nemu.h"
#include "device.h"


/////////////////////////////////////////////////////////////////
//                     keyboard simulation                     //
/////////////////////////////////////////////////////////////////

#define KEYBOARD_QUEUE_LEN 1024
static int keyboard_queue[KEYBOARD_QUEUE_LEN];
static int keyboard_f = 0, keyboard_r = 0;


typedef struct {
  int downcode, upcode;
} scancode_t;

scancode_t SDLK_to_scancode(SDLKey key) {
  switch(key) {
	case SDLK_ESCAPE:	return (scancode_t) { 0x76, 0xF076 };
	case SDLK_F1:	return (scancode_t) { 0x05, 0xF005 };
	case SDLK_F2:	return (scancode_t) { 0x06, 0xF006 };
	case SDLK_F3:	return (scancode_t) { 0x04, 0xF004 };
	case SDLK_F4:	return (scancode_t) { 0x0C, 0xF00C };
	case SDLK_F5:	return (scancode_t) { 0x03, 0xF003 };
	case SDLK_F6:	return (scancode_t) { 0x0B, 0xF00B };
	case SDLK_F7:	return (scancode_t) { 0x83, 0xF083 };
	case SDLK_F8:	return (scancode_t) { 0x0A, 0xF00A };
	case SDLK_F9:	return (scancode_t) { 0x01, 0xF001 };
	case SDLK_F10:	return (scancode_t) { 0x09, 0xF009 };
	case SDLK_F11:	return (scancode_t) { 0x78, 0xF078 };
	case SDLK_F12:	return (scancode_t) { 0x07, 0xF007 };
	case SDLK_SCROLLOCK:	return (scancode_t) { 0x7E, 0xF07E };
	case SDLK_BACKQUOTE:	return (scancode_t) { 0x0E, 0xF00E };
	case SDLK_1:	return (scancode_t) { 0x16, 0xF016 };
	case SDLK_2:	return (scancode_t) { 0x1E, 0xF01E };
	case SDLK_3:	return (scancode_t) { 0x26, 0xF026 };
	case SDLK_4:	return (scancode_t) { 0x25, 0xF025 };
	case SDLK_5:	return (scancode_t) { 0x2E, 0xF02E };
	case SDLK_6:	return (scancode_t) { 0x36, 0xF036 };
	case SDLK_7:	return (scancode_t) { 0x3D, 0xF03D };
	case SDLK_8:	return (scancode_t) { 0x3E, 0xF03E };
	case SDLK_9:	return (scancode_t) { 0x46, 0xF046 };
	case SDLK_0:	return (scancode_t) { 0x45, 0xF045 };
	case SDLK_MINUS:	return (scancode_t) { 0x4E, 0xF04E };
	case SDLK_EQUALS:	return (scancode_t) { 0x55, 0xF055 };
	case SDLK_BACKSPACE:	return (scancode_t) { 0x66, 0xF066 };
	case SDLK_TAB:	return (scancode_t) { 0x0D, 0xF00D };
	case SDLK_q:	return (scancode_t) { 0x15, 0xF015 };
	case SDLK_w:	return (scancode_t) { 0x1D, 0xF01D };
	case SDLK_e:	return (scancode_t) { 0x24, 0xF024 };
	case SDLK_r:	return (scancode_t) { 0x2D, 0xF02D };
	case SDLK_t:	return (scancode_t) { 0x2C, 0xF02C };
	case SDLK_y:	return (scancode_t) { 0x35, 0xF035 };
	case SDLK_u:	return (scancode_t) { 0x3C, 0xF03C };
	case SDLK_i:	return (scancode_t) { 0x43, 0xF043 };
	case SDLK_o:	return (scancode_t) { 0x44, 0xF044 };
	case SDLK_p:	return (scancode_t) { 0x4D, 0xF04D };
	case SDLK_LEFTBRACKET:	return (scancode_t) { 0x54, 0xF054 };
	case SDLK_RIGHTBRACKET:	return (scancode_t) { 0x5B, 0xF05B };
	case SDLK_SLASH:	return (scancode_t) { 0x5D, 0xF05D };
	case SDLK_CAPSLOCK:	return (scancode_t) { 0x58, 0xF058 };
	case SDLK_a:	return (scancode_t) { 0x1C, 0xF01C };
	case SDLK_s:	return (scancode_t) { 0x1B, 0xF01B };
	case SDLK_d:	return (scancode_t) { 0x23, 0xF023 };
	case SDLK_f:	return (scancode_t) { 0x2B, 0xF02B };
	case SDLK_g:	return (scancode_t) { 0x34, 0xF034 };
	case SDLK_h:	return (scancode_t) { 0x33, 0xF033 };
	case SDLK_j:	return (scancode_t) { 0x3B, 0xF03B };
	case SDLK_k:	return (scancode_t) { 0x42, 0xF042 };
	case SDLK_l:	return (scancode_t) { 0x4B, 0xF04B };
	case SDLK_SEMICOLON:	return (scancode_t) { 0x4C, 0xF04C };
	case SDLK_QUOTE:	return (scancode_t) { 0x52, 0xF052 };
	case SDLK_RETURN:	return (scancode_t) { 0x5A, 0xF05A };
	case SDLK_LSHIFT:	return (scancode_t) { 0x12, 0xF012 };
	case SDLK_z:	return (scancode_t) { 0x1A, 0xF01A };
	case SDLK_x:	return (scancode_t) { 0x22, 0xF022 };
	case SDLK_c:	return (scancode_t) { 0x21, 0xF021 };
	case SDLK_v:	return (scancode_t) { 0x2A, 0xF02A };
	case SDLK_b:	return (scancode_t) { 0x32, 0xF032 };
	case SDLK_n:	return (scancode_t) { 0x31, 0xF031 };
	case SDLK_m:	return (scancode_t) { 0x3A, 0xF03A };
	case SDLK_COMMA:	return (scancode_t) { 0x41, 0xF041 };
	case SDLK_PERIOD:	return (scancode_t) { 0x49, 0xF049 };
	case SDLK_BACKSLASH:	return (scancode_t) { 0x4A, 0xF04A };
	case SDLK_RSHIFT:	return (scancode_t) { 0x59, 0xF059 };
	case SDLK_LCTRL:	return (scancode_t) { 0x14, 0xF014 };
	case SDLK_LALT:	return (scancode_t) { 0x11, 0xF011 };
	case SDLK_SPACE:	return (scancode_t) { 0x29, 0xF029 };
	case SDLK_RALT:	return (scancode_t) { 0xE011, 0xE0F011 };
	case SDLK_RCTRL:	return (scancode_t) { 0xE014, 0xE0F014 };
	case SDLK_INSERT:	return (scancode_t) { 0xE070, 0xE0F070 };
	case SDLK_HOME:	return (scancode_t) { 0xE06C, 0xE0F06C };
	case SDLK_PAGEUP:	return (scancode_t) { 0xE07D, 0xE0F07D };
	case SDLK_DELETE:	return (scancode_t) { 0xE071, 0xE0F071 };
	case SDLK_END:	return (scancode_t) { 0xE069, 0xE0F069 };
	case SDLK_PAGEDOWN:	return (scancode_t) { 0xE07A, 0xE0F07A };
	case SDLK_UP:	return (scancode_t) { 0xE075, 0xE0F075 };
	case SDLK_LEFT:	return (scancode_t) { 0xE06B, 0xE0F06B };
	case SDLK_DOWN:	return (scancode_t) { 0xE072, 0xE0F072 };
	case SDLK_RIGHT:	return (scancode_t) { 0xE074, 0xE0F074 };
	case SDLK_NUMLOCK:	return (scancode_t) { 0x77, 0xF077 };
	case SDLK_KP_DIVIDE:	return (scancode_t) { 0xE04A, 0xE0F04A };
	case SDLK_KP_MULTIPLY:	return (scancode_t) { 0x7C, 0xF07C };
	case SDLK_KP_MINUS:	return (scancode_t) { 0x7B, 0xF07B };
	case SDLK_KP_PLUS:	return (scancode_t) { 0x79, 0xF079 };
	case SDLK_KP7:	return (scancode_t) { 0x6C, 0xF06C };
	case SDLK_KP8:	return (scancode_t) { 0x75, 0xF075 };
	case SDLK_KP9:	return (scancode_t) { 0x7D, 0xF07D };
	case SDLK_KP4:	return (scancode_t) { 0x6B, 0xF06B };
	case SDLK_KP5:	return (scancode_t) { 0x73, 0xF073 };
	case SDLK_KP6:	return (scancode_t) { 0x74, 0xF074 };
	case SDLK_KP1:	return (scancode_t) { 0x69, 0xF069 };
	case SDLK_KP2:	return (scancode_t) { 0x72, 0xF072 };
	case SDLK_KP3:	return (scancode_t) { 0x7A, 0xF07A };
	case SDLK_KP0:	return (scancode_t) { 0x70, 0xF070 };
	case SDLK_KP_EQUALS:	return (scancode_t) { 0x71, 0xF071 };
	case SDLK_KP_ENTER:	return (scancode_t) { 0xE05A, 0xE0F05A };
	default: return (scancode_t) { 0, 0 };
  }
};

void keyboard_enqueue(SDL_EventType type, SDLKey key) {
  if(key < 0 || key >= SDLK_LAST) return;
  int scancode = type == SDL_KEYUP ?
	SDLK_to_scancode(key).upcode : SDLK_to_scancode(key).downcode;

  if(scancode == 0) return;

  int next = (keyboard_r + 1) % KEYBOARD_QUEUE_LEN;
  if(next != keyboard_f) { // if not full
	keyboard_queue[keyboard_r] = scancode;
	keyboard_r = next;
  }
}

#define check_input(addr, len) \
  CoreAssert(addr >= 0 && addr <= KB_SIZE, \
	  "input: address(0x%08x) is out side", addr); \
  CoreAssert(len == 4, "input only allow byte read/write");

uint32_t kb_read(paddr_t addr, int len) {
  /* CTRL not yet implemented, only allow byte read/write */
  check_input(addr, len);
  switch (addr) {
	case KB_STAT:
	  return keyboard_f == keyboard_r ? 0 : 1;
	case KB_CODE: {
	  uint32_t code = keyboard_queue[keyboard_f];
	  keyboard_f = (keyboard_f + 1) % KEYBOARD_QUEUE_LEN;
	  return code;
	}
	default:
	  CoreAssert(false, "keyboard: address(0x%08x) is not readable", addr);
	  break;
  }
  return 0;
}

