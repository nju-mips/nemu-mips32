#ifndef DEVICE_H
#define DEVICE_H

#include "device.h"
#include "memory.h"
#include <stdint.h>

static inline void check_ioaddr(uint32_t addr,
                                uint32_t size,
                                const char *dev) {
  CPUAssert(
      addr < size,
      "%s: address(0x%08x) is out of side or unaligned",
      dev, addr);
}

void *paddr_map(uint32_t paddr, uint32_t size);

/* uartlite protocol */

typedef struct {
  const char *name;
  uint32_t start, end;
  uint32_t (*read)(paddr_t addr, int len);
  void (*write)(paddr_t addr, int len, uint32_t data);
  void *(*map)(uint32_t vaddr, uint32_t size);
  uint32_t (*peek)(paddr_t addr, int len);
} device_t;

/*
void *spi_map(uint32_t vaddr, uint32_t size);
uint32_t spi_read(paddr_t addr, int len);
void spi_write(paddr_t addr, int len, uint32_t data);
*/

#define KSEG0_BASE 0x80000000
#define KSEG1_BASE 0xA0000000

// block ram
#define BRAM_BASE 0x1fc00000
#define BRAM_SIZE (1024 * 1024)

// DDR
#define DDR_BASE (0x00000000)
#define DDR_SIZE (128 * 1024 * 1024) // 0x08000000

// UART
#define SERIAL_ADDR 0x1fe50000
#define SERIAL_SIZE 0x10

// SPI
#define SPI_ADDR 0x1fe80000
#define SPI_SIZE 0x1000

// KEYBOARD
#define KB_ADDR 0x1fe94000
#define KB_CODE 0x0
#define KB_STAT 0x4
#define KB_SIZE 0x10

// perf counter
#define PERF_ADDR 0x1fe95000
#define PERF_SIZE 0x1000

// bad phsical address
#define BADP_ADDR 0x1fe96000
#define BADP_SIZE 0x1000

// emaclite
#define MAC_ADDR 0x1ff00000
#define MAC_SIZE 0x10000

// VGA
#define VGA_BASE 0x10400000
#define VGA_SIZE 0x100000

#define SCR_W 400
#define SCR_H 300
#define WINDOW_W (SCR_W * 2)
#define WINDOW_H (SCR_H * 2)
#define VGA_HZ 25
#define TIMER_HZ 100

// GPIO
#define GPIO_BASE 0x10000000
#define GPIO_SIZE 0x1000

#endif
