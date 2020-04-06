#include "common.h"
#include "elfsym.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

void elfsym_release(elfsym_t *elfsym) {
  if (elfsym->strtab) {
    free(elfsym->strtab);
    elfsym->strtab = NULL;
  }
  if (elfsym->symtab) {
    free(elfsym->symtab);
    elfsym->symtab = NULL;
  }

  elfsym->nr_symtab_entry = 0;
  elfsym->strtab_size = 0;
}

void elfsym_load(elfsym_t *elfsym, const char *elf_file) {
  int ret = 0;
  elfsym_release(elfsym);

  elfsym->file = elf_file;
  FILE *fp = fopen(elf_file, "rb");
  Assert(fp, "Can not open '%s'", elf_file);

  uint8_t buf[sizeof(Elf32_Ehdr)];
  ret = fread(buf, sizeof(Elf32_Ehdr), 1, fp);
  assert(ret == 1);

  /* The first several bytes contain the ELF header. */
  Elf32_Ehdr *elf = (void *)buf;
  char magic[] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};

  /* Check ELF header */
  assert(memcmp(elf->e_ident, magic, 4) == 0);

  /* Load symbol table and string table for future use */

  /* Load section header table */
  uint32_t sh_size = elf->e_shentsize * elf->e_shnum;
  Elf32_Shdr *sh = malloc(sh_size);
  fseek(fp, elf->e_shoff, SEEK_SET);
  ret = fread(sh, sh_size, 1, fp);
  assert(ret == 1);

  /* Load section header string table */
  char *shstrtab = malloc(sh[elf->e_shstrndx].sh_size);
  fseek(fp, sh[elf->e_shstrndx].sh_offset, SEEK_SET);
  ret = fread(shstrtab, sh[elf->e_shstrndx].sh_size, 1, fp);
  assert(ret == 1);

  int i;
  for (i = 0; i < elf->e_shnum; i++) {
    if (sh[i].sh_type == SHT_SYMTAB &&
        strcmp(shstrtab + sh[i].sh_name, ".symtab") == 0) {
      /* Load symbol table from elf_file */
      elfsym->symtab = malloc(sh[i].sh_size);
      fseek(fp, sh[i].sh_offset, SEEK_SET);
      ret = fread(elfsym->symtab, sh[i].sh_size, 1, fp);
      assert(ret == 1);
      elfsym->nr_symtab_entry =
          sh[i].sh_size / sizeof(elfsym->symtab[0]);
    } else if (sh[i].sh_type == SHT_STRTAB &&
               strcmp(shstrtab + sh[i].sh_name,
                   ".strtab") == 0) {
      /* Load string table from exec_file */
      elfsym->strtab_size = sh[i].sh_size;
      elfsym->strtab = malloc(sh[i].sh_size);
      fseek(fp, sh[i].sh_offset, SEEK_SET);
      ret = fread(elfsym->strtab, sh[i].sh_size, 1, fp);
      assert(ret == 1);
    }
  }

  free(sh);
  free(shstrtab);

  assert(elfsym->strtab != NULL && elfsym->symtab != NULL);

  fclose(fp);
}

uint32_t elfsym_get_addr(
    elfsym_t *elfsym, const char *symbol) {
  for (int i = 0; i < elfsym->nr_symtab_entry; i++) {
    if (ELF32_ST_TYPE(elfsym->symtab[i].st_info) !=
        STT_FUNC)
      continue;

    int st_name = elfsym->symtab[i].st_name;
    assert(st_name < elfsym->strtab_size);
    if (strcmp(symbol, elfsym->strtab + st_name) != 0)
      continue;

    return elfsym->symtab[i].st_value;
  }
  return 0;
}

const char *elfsym_find_symbol(
    elfsym_t *elfsym, uint32_t addr) {
  uint32_t idx = addr & ((1 << ADDR2SYM_CACHE_BITS) - 1);
  if (elfsym->addr2sym_cache &&
      elfsym->addr2sym_cache[idx]) {
    return elfsym->addr2sym_cache[idx];
  }

  for (int i = 0; i < elfsym->nr_symtab_entry; i++) {
    bool addr_in_range =
        elfsym->symtab[i].st_value <= addr &&
        addr < elfsym->symtab[i].st_value +
                   elfsym->symtab[i].st_size;
    if (!addr_in_range) continue;

    if (ELF32_ST_TYPE(elfsym->symtab[i].st_info) !=
        STT_FUNC)
      continue;

    int st_name = elfsym->symtab[i].st_name;
    assert(st_name < elfsym->strtab_size);
    if (elfsym->addr2sym_cache)
      elfsym->addr2sym_cache[idx] =
          elfsym->strtab + st_name;
    return elfsym->strtab + st_name;
  }

  const char *notfound = "?";
  if (elfsym->addr2sym_cache)
    elfsym->addr2sym_cache[idx] = notfound;
  return notfound;
}

void elfsym_optimize_find_symbol(elfsym_t *elfsym) {
  uint32_t size =
      sizeof(const char *) * NR_ADDR2SYM_CACHE_ENTRIES;
  elfsym->addr2sym_cache = malloc(size);
  memset(elfsym->addr2sym_cache, 0, size);
}
