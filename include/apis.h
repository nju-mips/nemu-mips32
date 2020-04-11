#ifndef APIS_H
#define APIS_H

#include "cpu/memory.h"
#include "cpu/mmu.h"
#include "cpu/reg.h"
#include "dev/device.h"
#include "monitor.h"

extern work_mode_t work_mode;
extern tlb_entry_t tlb[NR_TLB_ENTRY];

/* APIs exported by nemu-mips32 */
extern CPU_state cpu;
extern void cpu_exec(uint64_t);
extern void print_registers();
extern void print_instr_queue(void);
extern work_mode_t parse_args(int argc, const char *argv[]);
extern work_mode_t init_monitor(void);
extern void init_sdl(void);
extern void init_mmio(void);
extern void init_events(void);
extern uint32_t paddr_peek(paddr_t addr, int len);
extern uint64_t get_current_time();
extern uint32_t get_current_pc();
extern uint32_t get_current_instr();
extern device_t *get_device_list_head();
extern void nemu_set_irq(int irqno, bool val);
/* APIs exported by nemu-mips32 */

#endif
