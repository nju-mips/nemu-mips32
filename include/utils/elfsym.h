#ifndef ELFSYM_H
#define ELFSYM_H

#include <elf.h>
#include <stdint.h>

#define ADDR2SYM_CACHE_BITS 12
#define NR_ADDR2SYM_CACHE_ENTRIES (1 << ADDR2SYM_CACHE_BITS)

typedef struct {
  const char *file;
  char *strtab;
  Elf32_Sym *symtab;
  int nr_symtab_entry;
  int strtab_size;
  struct {
    const char *sym;
    uint32_t addr;
  } *addr2sym_cache;
} elfsym_t;

void elfsym_release(elfsym_t *elfsym);
void elfsym_load(elfsym_t *elfsym, const char *file);
uint32_t elfsym_get_addr(elfsym_t *elfsym, const char *sym);
const char *elfsym_find_symbol(
    elfsym_t *elfsym, uint32_t addr);
void elfsym_optimize_find_symbol(elfsym_t *elfsym);

void elfperf_start();
void elfperf_record(uint32_t pc);
void elfperf_report();

extern elfsym_t elfsym;

#endif
