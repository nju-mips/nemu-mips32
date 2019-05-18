#include "device.h"
#include "nemu.h"
#include <SDL/SDL.h>
#include <stdio.h>

#define VMEM_SIZE (4 * WINDOW_H * WINDOW_W)

extern SDL_Surface *screen;

uint32_t vga_read(paddr_t addr, int len) {
  check_ioaddr(addr, VMEM_SIZE, "VGA");
  uint32_t *pixel_buf = screen->pixels;
  return *((uint32_t *)&pixel_buf[addr]) &
         (~0u >> ((4 - len) << 3));
}

void vga_write(paddr_t addr, int len, uint32_t data) {
  check_ioaddr(addr, VMEM_SIZE, "VGA");
  uint32_t *pixel_buf = screen->pixels;
  memcpy(&pixel_buf[addr], &data, len);
}

device_t vga_dev = {
    .name = "VGA",
    .start = VGA_BASE,
    .end = VGA_BASE + VGA_SIZE,
    .read = vga_read,
    .write = vga_write,
};
