#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>
#include "device.h"
#include "memory/memory.h"

typedef uint32_t (*read_func) (paddr_t addr, int len);
typedef void (*write_func)(paddr_t addr, int len, uint32_t data);

uint32_t ddr_read(paddr_t addr, int len);
void ddr_write(paddr_t addr, int len, uint32_t data);
uint32_t input_read(paddr_t addr, int len);
void input_write(paddr_t addr, int len, uint32_t data);
void gpio_write(paddr_t addr, int len, uint32_t data);
uint32_t vga_read(paddr_t addr, int len);
void vga_write(paddr_t addr, int len, uint32_t data);
uint32_t invalid_read(paddr_t addr, int len);
void invalid_write(paddr_t addr, int len, uint32_t data);

// DDR
#define DDR_BASE (0x10000000)
#define DDR_SIZE (256 * 1024 * 1024)

// VGA
#define SCR_W 400
#define SCR_H 300
#define WINDOW_W (SCR_W * 2)
#define WINDOW_H (SCR_H * 2)
#define VGA_HZ 25
#define TIMER_HZ 100

#endif
