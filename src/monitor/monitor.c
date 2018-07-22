#include "nemu.h"
#include "monitor/monitor.h"
#include "device.h"
#include <elf.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

static uint32_t entry_start = 0x10000000;

char *elf_file = NULL;
static char *img_file = NULL;

int is_batch_mode = false;
int print_commit_log = false;

void *ddr_map(uint32_t vaddr, uint32_t size);

size_t get_file_size(const char *img_file) {
  struct stat file_status;
  lstat(img_file, &file_status);
  return file_status.st_size;
}

void *read_file(const char *filename) {
  size_t size = get_file_size(filename);
  int fd = open(filename, O_RDONLY);
  if(fd == -1) return NULL;

  // malloc buf which should be freed by caller
  void *buf = malloc(size);
  int len = 0;
  while(len < size) {
	len += read(fd, buf, size - len);
  }
  return buf;
}

void load_elf() {
  Assert(elf_file, "Need an elf file");
  Log("The elf is %s", elf_file);

  const uint32_t elf_magic = 0x464c457f;

  void *buf = read_file(elf_file);
  Assert(buf, "elf file '%s' cannot be opened for read\n", elf_file);

  Elf32_Ehdr *elf = buf;

  uint32_t *p_magic = buf;
  assert(*p_magic == elf_magic);

  for(int i = 0; i < elf->e_phnum; i++) {
	  Elf32_Phdr *ph = (void*)buf + i * elf->e_phentsize + elf->e_phoff;
	  if(ph->p_type != PT_LOAD) { continue; }

	  void *p_vaddr = ddr_map(ph->p_vaddr, ph->p_memsz);
	  memcpy(p_vaddr, buf + ph->p_offset, ph->p_filesz); 
	  memset(p_vaddr + ph->p_filesz, 0, ph->p_memsz - ph->p_filesz);
  }

  entry_start = elf->e_entry;
  free(buf);
}


static inline void load_img() {
  Assert(img_file, "Need an image file");
  Log("The image is %s", img_file);

  // load into ddr
  // be careful about memory mapping
  int fd = open(img_file, O_RDONLY);
  Assert(fd > 0, "Can not open '%s'", img_file);
  int ret = read(fd, ddr, DDR_SIZE);
  Assert(ret > 0, "read fails\n");
  close(fd);

  // assume img_file is xxx.bin and elf_file is xxx
  char *end = strrchr(img_file, '.');
  if(end) {
	  *end = 0;
	  elf_file = img_file;
  }
}

void sigint_handler(int no) {
  nemu_state = NEMU_STOP;
}

static inline void parse_args(int argc, char *argv[]) {
  int o;
  while ( (o = getopt(argc, argv, "-bci:e:")) != -1) {
    switch (o) {
      case 'b': is_batch_mode = true; break;
      case 'c': print_commit_log = true; break;
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
      default:
                panic("Usage: %s [-b] [-c] [-i img_file] [-e elf_file]", argv[0]);
    }
  }
}

int init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */

  /* Parse arguments. */
  parse_args(argc, argv);

  /* Load the image to memory. */
  if(elf_file) {
	load_elf();
  } else {
	load_img();
  }

  if(!is_batch_mode) signal(SIGINT, sigint_handler);

  /* Initialize this virtual computer system. */
  init_cpu(entry_start);

  return is_batch_mode;
}
