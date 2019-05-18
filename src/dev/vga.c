#include "device.h"
#include "nemu.h"
#include <SDL/SDL.h>
#include <stdio.h>

#define VGA_BASE 0x10400000
#define VGA_SIZE 0x100000
#define VMEM_SIZE (WINDOW_H * WINDOW_W)

extern SDL_Surface *screen;

typedef struct {
  uint8_t *ptr[4];
} four_pixel_t;

static inline uint32_t get_x(uint32_t addr) {
  return (addr / 4) % SCR_W;
}

static inline uint32_t get_y(uint32_t addr) {
  return (addr / 4) / SCR_W;
}

static inline four_pixel_t get_pixels(uint32_t addr) {
  uint32_t offset = addr % 4;
  uint32_t x = get_x(addr);
  uint32_t y = get_y(addr);
  uint32_t(*pixel_buf)[WINDOW_W] = screen->pixels;
  assert (pixel_buf && x < SCR_W && y < SCR_H);

  return (four_pixel_t) {
	{
	  (uint8_t *)&pixel_buf[2 * y][2 * x] + offset,
	  (uint8_t *)&pixel_buf[2 * y + 1][2 * x] + offset,
	  (uint8_t *)&pixel_buf[2 * y][2 * x + 1] + offset,
	  (uint8_t *)&pixel_buf[2 * y + 1][2 * x + 1] + offset,
	}
  };
}

static uint32_t vga_read(paddr_t addr, int len) {
  check_ioaddr(addr, VMEM_SIZE, "VGA");

  uint32_t offset = addr % 4;
  uint32_t x = get_x(addr);
  uint32_t y = get_y(addr);
  uint32_t(*pixel_buf)[WINDOW_W] = screen->pixels;
  assert (pixel_buf && x < SCR_W && y < SCR_H);

  return *((uint32_t *)((void *)&pixel_buf[2 * y][2 * x]
		   + addr)) & (~0u >> ((4 - len) << 3));
}

static void vga_write(paddr_t addr, int len, uint32_t data) {
  check_ioaddr(addr, VMEM_SIZE, "VGA");

  four_pixel_t pixels = get_pixels(addr);
  memcpy(pixels.ptr[0], &data, len);
  memcpy(pixels.ptr[1], &data, len);
  memcpy(pixels.ptr[2], &data, len);
  memcpy(pixels.ptr[3], &data, len);
}

device_t vga_dev = {
    .name = "VGA",
    .start = VGA_BASE,
    .end = VGA_BASE + VGA_SIZE,
    .read = vga_read,
    .write = vga_write,
};
