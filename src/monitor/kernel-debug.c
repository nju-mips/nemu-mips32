#include <assert.h>
#include <elf.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>

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

  for (int i = 0; i < elf->e_shnum; i++) {
    Elf32_Shdr *sh = (void *)buf + i * elf->e_shentsize + elf->e_shoff;
    if (sh->sh_type != SHT_PROGBITS) { continue; }
    if (!(sh->sh_flags & SHF_ALLOC)) continue;

    void *ptr = vaddr_map(sh->sh_addr, sh->sh_size);
    for (int i = 0; i < sh->sh_size; i += 4) {
      uint32_t *loaded = ptr + i;
      uint32_t *standard = buf + sh->sh_offset + i;
      if (*loaded != *standard) {
        printf("inconsistent@%08x: %08x <> %08x\n", sh->sh_addr + i, *loaded,
            *standard);
      }
    }
  }

  free(buf);
}
