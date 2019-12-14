#include "utils.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <elf.h>

typedef struct {
  const char *elf_path;
  char *elf_strtab;
  Elf32_Sym *elf_symtab;
  int nr_symtab_entry;
  int elf_strtab_size;
} elf_desc_t;

static char *elf_strtab = NULL;      /* needs free */
static Elf32_Sym *elf_symtab = NULL; /* needs free */
static int nr_symtab_entry = 0;
static int elf_strtab_size = 0;

void elf_symbols_release_memory() {
  if (elf_strtab) {
    free(elf_strtab);
    elf_strtab = NULL;
  }
  if (elf_symtab) {
    free(elf_symtab);
    elf_symtab = NULL;
  }

  nr_symtab_entry = 0;
  elf_strtab_size = 0;
}

void load_elf_symtab(const char *elf_file) {
  int ret = 0;
  elf_symbols_release_memory();

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
      elf_symtab = malloc(sh[i].sh_size);
      fseek(fp, sh[i].sh_offset, SEEK_SET);
      ret = fread(elf_symtab, sh[i].sh_size, 1, fp);
      assert(ret == 1);
      nr_symtab_entry = sh[i].sh_size / sizeof(elf_symtab[0]);
    } else if (sh[i].sh_type == SHT_STRTAB &&
               strcmp(shstrtab + sh[i].sh_name, ".strtab") == 0) {
      /* Load string table from exec_file */
      elf_strtab_size = sh[i].sh_size;
      elf_strtab = malloc(sh[i].sh_size);
      fseek(fp, sh[i].sh_offset, SEEK_SET);
      ret = fread(elf_strtab, sh[i].sh_size, 1, fp);
      assert(ret == 1);
    }
  }

  free(sh);
  free(shstrtab);

  assert(elf_strtab != NULL && elf_symtab != NULL);

  fclose(fp);
}

uint32_t find_addr_of_symbol(const char *symbol) {
  for (int i = 0; i < nr_symtab_entry; i++) {
    if (ELF32_ST_TYPE(elf_symtab[i].st_info) != STT_FUNC) continue;

    int st_name = elf_symtab[i].st_name;
    assert(st_name < elf_strtab_size);
    if (strcmp(symbol, elf_strtab + st_name) != 0) continue;

    return elf_symtab[i].st_value;
  }
  return 0;
}

const char *find_symbol_by_addr(uint32_t addr) {
#if 1 /* for u-boot */
  extern const char *symbol_file;
  bool is_uboot = strstr(symbol_file, "u-boot");
  if (is_uboot && addr < 0xbfc00000)
    addr = addr - 0x87f80000 + 0xbfc00000;
#endif
  for (int i = 0; i < nr_symtab_entry; i++) {
    bool addr_in_range = elf_symtab[i].st_value <= addr &&
                         addr < elf_symtab[i].st_value + elf_symtab[i].st_size;
    if (!addr_in_range) continue;

    if (ELF32_ST_TYPE(elf_symtab[i].st_info) != STT_FUNC) continue;

    int st_name = elf_symtab[i].st_name;
    assert(st_name < elf_strtab_size);
    return elf_strtab + st_name;
  }
  return "";
}
