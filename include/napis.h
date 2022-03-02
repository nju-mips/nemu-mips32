#ifndef NAPIS_H
#define NAPIS_H

#include <stdint.h>
#include <stdbool.h>

void napi_init(int argc, const char *argv[]);
void napi_exec(uint64_t n);
uint32_t napi_mmio_peek(uint32_t paddr, int len);
uint32_t napi_get_instr();
uint32_t napi_get_pc();
void *napi_map_dev(
    const char *name, uint32_t addr, unsigned size);
bool napi_addr_is_valid(uint32_t addr);
void napi_set_irq(int irqno, bool val);
void napi_dump_states();
uint32_t napi_get_gpr(int i);
void napi_set_gpr(int i, uint32_t val);
bool napi_cpu_is_end();

void napi_ulite_set_data(int data);
int napi_ulite_get_data();
void napi_stop_cpu_when_ulite_send(const char *data);

#endif
