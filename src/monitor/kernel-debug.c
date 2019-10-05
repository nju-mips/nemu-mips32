#include <assert.h>
#include <elf.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>

#include "device.h"
#include "memory.h"
#include "syscalls.h"

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

void dump_string(uint32_t addr, uint32_t limit) {
  putchar('"');
  for (int i = 0; i < limit; i++) {
    char ch = dbg_vaddr_read(addr + i, 1);
    if (ch == 0) break;
    putchar(ch);
  }
  putchar('"');
}

void dump_syscall(uint32_t v0, uint32_t a0, uint32_t a1, uint32_t a2) {
  switch (v0) {
  case __NR_read: printf("read(%d, 0x%08x, %d)\n", a0, a1, a2); break;
  case __NR_write: printf("write(%d, 0x%08x, %d)\n", a0, a1, a2); break;
  case __NR_open:
    printf("open(");
    dump_string(a0, 40);
    printf(", %d)\n", a1);
    break;
  case __NR_uname: printf("uname(0x%08x)\n", a0); break;
  case __NR_brk: printf("brk(0x%08x)\n", a0); break;
  case __NR_readlink:
    printf("readlink(");
    dump_string(a0, 40);
    printf(", 0x%08x, %d)\n", a1, a2);
    break;
  case __NR_getuid: printf("getuid()\n"); break;
  case __NR_geteuid: printf("geteuid()\n"); break;
  case __NR_getpid: printf("getpid()\n"); break;
  case __NR_mkdir:
    printf("mkdir(");
    dump_string(a0, 40);
    printf(", %d)\n", a1);
    break;
  case __NR_mknod:
    printf("mknod(");
    dump_string(a0, 40);
    printf(", %d, ...)\n", a1);
    break;
  case __NR_fork: printf("fork()\n"); break;
  case __NR_mount:
    printf("mount(");
    dump_string(a0, 40);
    printf(", ");
    dump_string(a1, 40);
    printf(")\n");
    break;
  default: printf("syscall(%d)\n", v0); break;
  }
}
