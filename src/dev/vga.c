#include "device.h"
#include "nemu.h"
#include <SDL/SDL.h>
#include <stdio.h>

#define VGA_BASE 0x10400000
#define VGA_SIZE 0x100000
#define VMEM_SIZE (WINDOW_H * WINDOW_W)

extern SDL_Surface *screen;

static inline uint32_t read_pixel(unsigned x, unsigned y) {
  uint32_t(*pixel_buf)[WINDOW_W] = screen->pixels;
  assert (pixel_buf && x < SCR_W && y < SCR_H);
  return pixel_buf[2 * y][2 * x];
}

static inline void draw_pixel(unsigned x, unsigned y,
                              uint32_t pixel) {
  uint32_t(*pixel_buf)[WINDOW_W] = screen->pixels;
  assert (pixel_buf && x < SCR_W && y < SCR_H);
  pixel_buf[2 * y][2 * x] = pixel;
  pixel_buf[2 * y + 1][2 * x] = pixel;
  pixel_buf[2 * y][2 * x + 1] = pixel;
  pixel_buf[2 * y + 1][2 * x + 1] = pixel;
}

uint32_t vga_read(paddr_t addr, int len) {
  check_ioaddr(addr, VMEM_SIZE, "VGA");
  return read_pixel((addr / 4) % SCR_W, addr / SCR_W);
}

void vga_write(paddr_t addr, int len, uint32_t data) {
  check_ioaddr(addr, VMEM_SIZE, "VGA");
  draw_pixel((addr / 4) % SCR_W, (addr / 4) / SCR_W, data);
}

device_t vga_dev = {
    .name = "VGA",
    .start = VGA_BASE,
    .end = VGA_BASE + VGA_SIZE,
    .read = vga_read,
    .write = vga_write,
};
