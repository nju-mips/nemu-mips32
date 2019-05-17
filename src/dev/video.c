#include "device.h"
#include "nemu.h"
#include <SDL/SDL.h>
#include <stdio.h>

/////////////////////////////////////////////////////////////////
//                       vga simulation //
/////////////////////////////////////////////////////////////////

#define VMEM_SIZE (4 * WINDOW_H * WINDOW_W)

extern SDL_Surface *screen;

static uint8_t vmem[VMEM_SIZE];

uint32_t vga_read(paddr_t addr, int len) {
  check_ioaddr(addr, VMEM_SIZE, "VGA");
  return *((uint32_t *)&vmem[addr]) &
         (~0u >> ((4 - len) << 3));
}

void vga_write(paddr_t addr, int len, uint32_t data) {
  check_ioaddr(addr, VMEM_SIZE, "VGA");
  memcpy(&vmem[addr], &data, len);
}

static inline uint32_t RGB_M12_to_M32(uint32_t color) {
  return 0xFF000000 | ((color & 0xF00) << (8 + 4)) // R
         | ((color & 0x0F0) << (4 + 4))            // G
         | ((color & 0x00F) << (0 + 4));           // B
}

static inline void draw_pixel(int x, int y,
                              uint32_t color) {
  uint32_t(*pixel_buf)[WINDOW_W] = screen->pixels;
  assert(x >= 0 && x < WINDOW_W && y >= 0 && y < WINDOW_H);
  pixel_buf[y][x] = RGB_M12_to_M32(color);
}

void update_screen() {
  uint16_t(*vmem_ptr)[SCR_W] = (void *)vmem;

  for (int i = 0; i < SCR_H; i++) {
    for (int j = 0; j < SCR_W; j++) {
      uint16_t color = vmem_ptr[i][j];
      draw_pixel(2 * j, 2 * i, color);
      draw_pixel(2 * j, 2 * i + 1, color);
      draw_pixel(2 * j + 1, 2 * i, color);
      draw_pixel(2 * j + 1, 2 * i + 1, color);
    }
  }
  SDL_Flip(screen);
}

device_t vga_dev = {
    .name = "VGA",
    .start = VGA_BASE,
    .end = VGA_BASE + VGA_SIZE,
    .read = vga_read,
    .write = vga_write,
};
