#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>
#include "device.h"
#include "memory.h"


void *ddr_map(uint32_t vaddr, uint32_t size);
void *unmapped_map(uint32_t vaddr, uint32_t size);

typedef uint32_t (*read_func) (paddr_t addr, int len);
typedef void (*write_func)(paddr_t addr, int len, uint32_t data);

#ifdef __ARCH_LOONGSON__
uint32_t confreg_read(paddr_t addr, int len);
void confreg_write(paddr_t addr, int len, uint32_t data);
#endif

#ifdef ENABLE_PAGING
uint32_t unmapped_read(paddr_t addr, int len);
void unmapped_write(paddr_t addr, int len, uint32_t data);
#endif

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

// CONFREG
#ifdef __ARCH_LOONGSON__
#define CONFREG_BASE 0xFFFF0000
#define CONFREG_SIZE (0x00010000 - 1) // to avoid overflow

#define CONFREG_CR0_ADDR       0x8000 
#define CONFREG_CR1_ADDR       0x8004 
#define CONFREG_CR2_ADDR       0x8008 
#define CONFREG_CR3_ADDR       0x800c 
#define CONFREG_CR4_ADDR       0x8010 
#define CONFREG_CR5_ADDR       0x8014 
#define CONFREG_CR6_ADDR       0x8018 
#define CONFREG_CR7_ADDR       0x801c 

#define CONFREG_LED_RG_GREEN   0x1
#define CONFREG_LED_RG_RED     0x2

#define CONFREG_LED_ADDR       0xf000 
#define CONFREG_LED_RG0_ADDR   0xf004 // 1 is green, 2 is red
#define CONFREG_LED_RG1_ADDR   0xf008 // 1 is green, 2 is red
#define CONFREG_NUM_ADDR       0xf010 
#define CONFREG_SWITCH_ADDR    0xf020 
#define CONFREG_BTN_KEY_ADDR   0xf024
#define CONFREG_BTN_STEP_ADDR  0xf028
#define CONFREG_TIMER_ADDR     0xe000 

#define CONFREG_VIRTUAL_UART_ADDR 0xfff0
#define CONFREG_SIMU_FLAG_ADDR    0xfff4 
#define CONFREG_OPEN_TRACE_ADDR   0xfff8
#define CONFREG_NUM_MONITOR_ADDR  0xfffc
#endif

// DDR
#ifdef __ARCH_MIPS32_NPC__
#define DDR_BASE (0x10000000)
#elif defined __ARCH_LOONGSON__
#define DDR_BASE (0x80000000)
#else
#define DDR_BASE (0x1000000)
#endif

#ifdef __ARCH_LOONGSON__
/* map from 0x80000000 to 0xc0000000 */
#define DDR_SIZE (512 * 1024 * 1024)
#else
#define DDR_SIZE (256 * 1024 * 1024)
#endif

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
#define VGA_BASE 0x50000000
#define VGA_SIZE 0x100000

#define SCR_W 400
#define SCR_H 300
#define WINDOW_W (SCR_W * 2)
#define WINDOW_H (SCR_H * 2)
#define VGA_HZ 25
#define TIMER_HZ 100

// GPIO
#define GPIO_BASE 0x40000000
#define GPIO_SIZE 0x1000

#endif
