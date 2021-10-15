#include "cpu/memory.h"
#include "cpu/mmu.h"
#include "cpu/reg.h"
#include "debug.h"
#include "device.h"
#include "monitor.h"
#include "napis.h"

void parse_args(int, const char *[]);
void init_events();
void init_sdl();
void init_mmio();
int init_monitor(void);
void cpu_exec(uint64_t n);

void napi_init(int argc, const char *argv[]) {
  parse_args(argc, argv);

  init_sdl();
  init_mmio();
  init_monitor();
  init_events();
}

void napi_exec(uint64_t n) { cpu_exec(n); }

uint32_t napi_mmio_peek(uint32_t paddr, int len) {
  return paddr_peek(paddr, len);
}

uint32_t napi_get_instr() { return get_current_instr(); }

uint32_t napi_get_pc() { return get_current_pc(); }

void *napi_map_dev(
    const char *name, uint32_t addr, unsigned size) {
  device_t *dev = get_device_list_head();
  for (; dev; dev = dev->next) {
    if (strcmp(dev->name, name) == 0) break;
  }
  if (!dev) return NULL;
  return dev->map(addr, size);
}

bool napi_addr_is_valid(uint32_t addr) {
  return find_device(addr);
}

void napi_set_irq(int irqno, bool val) {
  nemu_set_irq(irqno, val);
}

void napi_dump_states() {
  eprintf(">>>>>> nemu instrs\n");
  kdbg_print_instr_queue();
  eprintf(">>>>>> nemu registers\n");
  kdbg_print_registers();
  eprintf("====== nemu states end ======\n");
}

uint32_t napi_get_gpr(int i) { return cpu.gpr[i]; }

void napi_set_gpr(int i, uint32_t val) { cpu.gpr[i] = val; }

extern unsigned woop_log_cycles_st;
extern unsigned woop_log_cycles_ed;
extern bool woop_enable_bug;

unsigned napi_get_woop_log_cycles_st() {
  return woop_log_cycles_st;
}

unsigned napi_get_woop_log_cycles_ed() {
  return woop_log_cycles_ed;
}

bool napi_get_woop_enable_bug_flag() {
  return woop_enable_bug;
}
