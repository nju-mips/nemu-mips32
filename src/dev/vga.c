#include <SDL/SDL.h>
#include <stdio.h>

#include "device.h"

#define VGA_SIZE 0x100000
#define VMEM_SIZE (WINDOW_H * WINDOW_W)

extern SDL_Surface *screen;

static inline uint32_t get_x(uint32_t addr) { return (addr / 4) % SCR_W; }

static inline uint32_t get_y(uint32_t addr) { return (addr / 4) / SCR_W; }

static uint32_t vga_read(paddr_t addr, int len) {
  check_ioaddr(addr, len, VMEM_SIZE, "VGA.read");

  uint32_t offset = addr % 4;
  uint32_t x = get_x(addr);
  uint32_t y = get_y(addr);
  uint32_t(*ptr)[WINDOW_W] = screen->pixels;
  assert(ptr && x < SCR_W && y < SCR_H);

  return *((uint32_t *)((void *)&ptr[2 * y][2 * x] + offset)) &
         (~0u >> ((4 - len) << 3));
}

static void vga_write(paddr_t addr, int len, uint32_t data) {
  check_ioaddr(addr, len, VMEM_SIZE, "VGA.write");

  uint32_t offset = addr % 4;
  uint32_t x = get_x(addr);
  uint32_t y = get_y(addr);
  uint32_t(*ptr)[WINDOW_W] = screen->pixels;
  assert(ptr && x < SCR_W && y < SCR_H);

  memcpy((void *)&ptr[2 * y][2 * x] + offset, &data, len);
  memcpy((void *)&ptr[2 * y + 1][2 * x] + offset, &data, len);
  memcpy((void *)&ptr[2 * y][2 * x + 1] + offset, &data, len);
  memcpy((void *)&ptr[2 * y + 1][2 * x + 1] + offset, &data, len);
}

DEF_DEV(vga_dev) = {
    .name = "VGA",
    .start = CONFIG_VGA_BASE,
    .end = CONFIG_VGA_BASE + VGA_SIZE,
    .read = vga_read,
    .write = vga_write,
};
