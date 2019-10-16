#include <elf.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>

#include "device.h"
#include "memory.h"
#include "monitor.h"
#include "utils.h"

char *elf_file = NULL;
char *symbol_file = NULL;
static char *img_file = NULL;
// static char *kernel_img = NULL;

vaddr_t elf_entry = CPU_INIT_PC;
work_mode_t work_mode = MODE_GDB;

void load_rom(uint32_t entry) {
  uint32_t *p = vaddr_map(CPU_INIT_PC, 16);
  assert(p);
  p[0] = 0x3c080000 | (entry >> 16);    // lui t0, %hi(entry)
  p[1] = 0x25080000 | (entry & 0xFFFF); // addiu t0, t0, %lo(entry)
  p[2] = 0x01000008;                    // jr t0
  p[3] = 0x00000000;                    // nop
}

void load_elf() {
  Assert(elf_file, "Need an elf file");

  /* set symbol file to elf_file */
  symbol_file = elf_file;
  const uint32_t elf_magic = 0x464c457f;

  void *buf = read_file(elf_file);
  Assert(buf, "elf file '%s' cannot be opened for read\n", elf_file);

  Elf32_Ehdr *elf = buf;

  elf_entry = elf->e_entry;

  uint32_t *p_magic = buf;
  assert(*p_magic == elf_magic);

  for (int i = 0; i < elf->e_phnum; i++) {
    Elf32_Phdr *ph = (void *)buf + i * elf->e_phentsize + elf->e_phoff;
    if (ph->p_type != PT_LOAD) { continue; }

    void *ptr = vaddr_map(ph->p_vaddr, ph->p_memsz);
    memcpy(ptr, buf + ph->p_offset, ph->p_filesz);
    memset(ptr + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
  }

  if (elf->e_entry != CPU_INIT_PC) load_rom(elf->e_entry);

  free(buf);
}

static inline void load_image(const char *img, vaddr_t vaddr) {
  Assert(img, "Need an image file");
  Log("The image is %s\n", img);

  size_t size = get_file_size(img);
  void *buf = read_file(img);
  void *ptr = vaddr_map(vaddr, size);
  memcpy(ptr, buf, size);
  free(buf);
}

static inline void assume_elf_file() {
  /* assume img_file is xxx.bin and elf_file is xxx */
  char *end = strrchr(img_file, '.');
  if (end) {
    *end = 0;
    elf_file = img_file;
  }
}

static void sigint_handler(int no) { nemu_state = NEMU_STOP; }

const struct option long_options[] = {
    {"symbol", 1, NULL, 'S'},
    {"linux", 1, NULL, 'l'},
    // {"diff-with-qemu", 0, NULL, 'D'},
    {"batch", 0, NULL, 'b'},
    {"commit", 0, NULL, 'c'},
    {"image", 1, NULL, 'i'},
    {"elf", 1, NULL, 'e'},
    {"help", 0, NULL, 'h'},
#if CONFIG_ETHERLITE
    {"iface", 1, NULL, 'I'},
#endif
    {NULL, 0, NULL, 0},
};

static void print_help(const char *file) {
  printf("Usage: %s [OPTION...]\n", file);
  printf("\n");
  printf(
      "  -S, --symbol=FILE     use this file to produce "
      "symbols\n");
  printf("  -u, --uImage=FILE     specify uImage file\n");
  printf("  -D, --diff-with-qemu  run diff tests with qemu\n");
  printf("  -b, --batch           run on batch mode\n");
  printf(
      "  -c, --commit          commit all executed "
      "instructions\n");
  printf("  -i, --image=FILE      run with this image file\n");
  printf("  -e, --elf=FILE        run with this elf file\n");
  printf("  -h, --help            print program help info\n");
  printf("\n");
  printf("Report bugs to 141242068@smail.nju.edu.cn.\n");
}

void parse_args(int argc, char *argv[]) {
  int o;
  while (
      (o = getopt_long(argc, argv, "-bcDe:i:S:h", long_options, NULL)) != -1) {
    switch (o) {
    case 'S': symbol_file = optarg; break;
    case 'D': work_mode |= MODE_DIFF; break;
    case 'b': work_mode |= MODE_BATCH; break;
    case 'c': work_mode |= MODE_LOG; break;
    case 'e':
      if (elf_file != NULL)
        Log("too much argument '%s', ignored", optarg);
      else
        elf_file = optarg;
      break;
    case 'i':
      if (img_file != NULL)
        Log("too much argument '%s', ignored", optarg);
      else
        img_file = optarg;
      break;
#if CONFIG_ETHERLITE
    case 'I': {
      extern char *eth_iface;
      eth_iface = optarg;
      printf("set eth_iface to %s\n", eth_iface);
    } break;
#endif
    case 'h':
    default: print_help(argv[0]); exit(0);
    }
  }
}

work_mode_t init_monitor(void) {
  /* Load the image to memory. */
  if (elf_file) {
    load_elf();
  } else {
    load_image(img_file, CPU_INIT_PC);
  }

  if (!(work_mode & MODE_BATCH)) signal(SIGINT, sigint_handler);

#if CONFIG_PRELOAD_LINUX
  load_image(CONFIG_KERNEL_UIMAGE_PATH, CONFIG_KERNEL_UIMAGE_BASE);
#endif

  /* Initialize this virtual computer system. */
  init_cpu(CPU_INIT_PC);

  return work_mode;
}
