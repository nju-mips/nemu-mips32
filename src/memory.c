#include "device.h"
#include "memory.h"
#include "monitor.h"
#include "nemu.h"
#include <SDL/SDL.h>
#include <stdlib.h>

static device_t *memory_regions[1024 * 1024]; /* 8 MB */

static inline uint32_t mr_index(uint32_t addr) {
  return addr / (4 * 1024);
}

void register_device(device_t *dev) {
  assert(dev && (dev->start & 0xFFF) == 0);
  // assert((dev->end & 0xFFF) == 0);

  uint32_t _4KB = 4 * 1024;
  for (uint32_t i = dev->start; i < dev->end; i += _4KB) {
    assert(memory_regions[mr_index(i)] == NULL);
    memory_regions[mr_index(i)] = dev;
  }

  if (dev->init) dev->init();
}

device_t *find_device(paddr_t addr) {
  return memory_regions[mr_index(iomap(addr))];
}

void *vaddr_map(paddr_t addr, uint32_t len) {
  // only unmapped address can be map
  Assert(is_unmapped(addr),
         "addr %08x should be unmapped\n", addr);

  device_t *dev = find_device(addr);
  Assert(dev,
         "address(0x%08x) is out of bound, pc(0x%08x)\n",
         addr, cpu.pc);
  Assert(dev->map,
         "cannot find map handler for address(0x%08x), "
         "pc(0x%08x)\n",
         addr, cpu.pc);
  return dev->map(iomap(addr) - dev->start, len);
}

uint32_t vaddr_read_safe(vaddr_t addr, int len) {
  addr = prot_addr(addr, MMU_LOAD);
  device_t *dev = find_device(addr);
  if (!dev || !dev->read) return 0;
  return dev->read(addr - dev->start, len);
}

void vaddr_write_safe(vaddr_t addr, int len,
                      uint32_t data) {
  addr = prot_addr(addr, MMU_STORE);
  device_t *dev = find_device(addr);
  if (!dev || !dev->write) return;
  dev->write(addr - dev->start, len, data);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  addr = prot_addr(addr, MMU_LOAD);
  device_t *dev = find_device(addr);
  CPUAssert(dev && dev->read, "bad addr %08x\n", addr);
  return dev->read(addr - dev->start, len);
}

uint32_t paddr_peek(paddr_t addr, int len) {
  addr = prot_addr(addr, MMU_LOAD);
  device_t *dev = find_device(addr);
  CPUAssert(dev && dev->peek, "bad addr %08x\n", addr);
  return dev->peek(addr - dev->start, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  addr = prot_addr(addr, MMU_STORE);
  device_t *dev = find_device(addr);
  CPUAssert(dev && dev->write, "bad addr %08x\n", addr);
  return dev->write(addr - dev->start, len, data);
}

void init_mmio() {
#define X(dev)         \
  extern device_t dev; \
  register_device(&dev);

  DEVOP(X);
}
