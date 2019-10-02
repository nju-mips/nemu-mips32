#include <malloc.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <elf.h>

#include "device.h"
#include "memory.h"

size_t get_file_size(const char *img_file);
void *read_file(const char *filename);

void check_kernel_image(const char *image) {
  void *buf = read_file(image);
  assert(buf);

  Elf32_Ehdr *elf = buf;

  const uint32_t elf_magic = 0x464c457f;
  uint32_t *p_magic = buf;
  assert(*p_magic == elf_magic);

  for (int i = 0; i < elf->e_phnum; i++) {
    Elf32_Phdr *ph = (void *)buf + i * elf->e_phentsize + elf->e_phoff;
    if (ph->p_type != PT_LOAD) { continue; }

    void *ptr = vaddr_map(ph->p_vaddr, ph->p_filesz);
    for (int i = 0; i < ph->p_filesz; i += 4) {
      uint32_t *loaded = ptr + i;
      uint32_t *standard = buf + ph->p_offset + i;
      if (*loaded != *standard) {
        printf("inconsistent@%08x: %08x <> %08x\n", ph->p_vaddr + i, *loaded,
            *standard);
      }
    }
  }

  free(buf);
}
