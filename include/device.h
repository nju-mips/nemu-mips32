#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>
#include "device.h"
#include "memory.h"


void *ddr_map(uint32_t vaddr, uint32_t size);
void *unmapped_map(uint32_t vaddr, uint32_t size);

typedef uint32_t (*read_func) (paddr_t addr, int len);
typedef void (*write_func)(paddr_t addr, int len, uint32_t data);

uint32_t unmapped_read(paddr_t addr, int len);
void unmapped_write(paddr_t addr, int len, uint32_t data);
uint32_t ddr_read(paddr_t addr, int len);
void ddr_write(paddr_t addr, int len, uint32_t data);
uint32_t uartlite_read(paddr_t addr, int len);
void uartlite_write(paddr_t addr, int len, uint32_t data);
void gpio_write(paddr_t addr, int len, uint32_t data);
uint32_t vga_read(paddr_t addr, int len);
void vga_write(paddr_t addr, int len, uint32_t data);
uint32_t invalid_read(paddr_t addr, int len);
void invalid_write(paddr_t addr, int len, uint32_t data);
uint32_t kb_read(paddr_t addr, int len);

// DDR
#ifdef __ARCH_MIPS32_NPC__
#define DDR_BASE (0x10000000)
#else
#define DDR_BASE (0x1000000)
#endif

#define DDR_SIZE (256 * 1024 * 1024)

// UART
#ifdef ENABLE_PAGING
#define UARTLITE_ADDR 0xbfd03000
#else
#define UARTLITE_ADDR 0x40001000
#endif
#define UARTLITE_Rx     0x0
#define UARTLITE_Tx     0x4
#define UARTLITE_STAT   0x8
#define UARTLITE_CTRL   0xC
#define UARTLITE_SIZE   0x10

// KEYBOARD
#define KB_ADDR 0xbfd04000
#define KB_CODE 0x0
#define KB_STAT 0x4
#define KB_SIZE 0x10

// VGA
#define SCR_W 400
#define SCR_H 300
#define WINDOW_W (SCR_W * 2)
#define WINDOW_H (SCR_H * 2)
#define VGA_HZ 25
#define TIMER_HZ 100

#endif
