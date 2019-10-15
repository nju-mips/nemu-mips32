#ifndef DEVICE_H
#define DEVICE_H

#include <SDL/SDL.h>
#include <stdint.h>
#include <stdbool.h>

#include "device.h"
#include "memory.h"

static inline void check_ioaddr(
    uint32_t addr, uint32_t len, uint32_t size, const char *dev) {
  CPUAssert(addr <= size && len <= size && addr + len <= size,
      "%s: address(0x%08x) is out of side or unaligned", dev, addr);
}

void *vaddr_map(vaddr_t vaddr, uint32_t size);
void load_rom(uint32_t entry);

typedef struct device_t {
  const char *name;
  uint32_t start, end;
  void (*init)();
  uint32_t (*read)(paddr_t addr, int len);
  void (*write)(paddr_t addr, int len, uint32_t data);
  void *(*map)(uint32_t vaddr, uint32_t size);
  uint32_t (*peek)(paddr_t addr, int len);
  struct device_t *next;
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

device_t *get_device_list_head();
void register_device(device_t *dev);

#define DEF_DEV(name)                                                 \
  extern device_t name;                                               \
  __attribute__((constructor)) static void register_device_##name() { \
    register_device(&name);                                           \
  }                                                                   \
  device_t name

#endif

void serial_enqueue(int ch);
void serial_enqueue_SDLKey(SDL_EventType type, SDLKey key);
int serial_dequeue();
int serial_queue_top();
bool serial_queue_is_empty();
bool serial_queue_is_full();

void ulite_set_irq();
void stop_cpu_when_ulite_send(const char *string);
