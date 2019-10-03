#ifndef DEVICE_H
#define DEVICE_H

#include "device.h"
#include "memory.h"
#include <stdint.h>

static inline void check_ioaddr(uint32_t addr, uint32_t len, uint32_t size,
                                const char *dev) {
  CPUAssert(addr <= size && len <= size && addr + len <= size,
            "%s: address(0x%08x) is out of side or unaligned", dev, addr);
}

void *vaddr_map(vaddr_t vaddr, uint32_t size);
void load_rom(uint32_t entry);

typedef struct {
  const char *name;
  uint32_t start, end;
  void (*init)();
  uint32_t (*read)(paddr_t addr, int len);
  void (*write)(paddr_t addr, int len, uint32_t data);
  void *(*map)(uint32_t vaddr, uint32_t size);
  uint32_t (*peek)(paddr_t addr, int len);
} device_t;

static inline uint32_t mr_index(uint32_t addr) { return addr / (4 * 1024); }

static inline device_t *find_device(paddr_t addr) {
  extern device_t *memory_regions[1024 * 1024];
  return memory_regions[mr_index(ioremap(addr))];
}

#define SCR_W 400
#define SCR_H 300
#define WINDOW_W (SCR_W * 2)
#define WINDOW_H (SCR_H * 2)
#define VGA_HZ 25
#define TIMER_HZ 100

#define DEVOP(_)   \
  _(bram_dev)      \
  _(ddr_dev)       \
  _(gpio_dev)      \
  _(keyboard_dev)  \
  _(mac_dev)       \
  _(serial_dev)    \
  _(vga_dev)       \
  _(blackhole_dev) \
  _(screen_dev)    \
  _(spi_dev)       \
  _(perf_dev)      \
  _(rtc_dev)

#define DECL(_) extern device_t _;
DEVOP(DECL)
#undef DECL

#endif
