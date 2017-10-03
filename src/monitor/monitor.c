#include "nemu.h"
#include <unistd.h>

#define ENTRY_START 0x10000000

void init_difftest();
void init_regex();
void init_wp_pool();
void init_device();

void reg_test();
void init_qemu_reg();
bool gdb_memcpy_to_qemu(uint32_t, void *, int);

FILE *log_fp = NULL;
static char *log_file = NULL;
static char *img_file = NULL;
static int is_batch_mode = false;

static inline void init_log() {
#ifdef DEBUG
  if (log_file == NULL) return;
  log_fp = fopen(log_file, "w");
  Assert(log_fp, "Can not open '%s'", log_file);
#endif
}

static inline void welcome() {
  printf("Welcome to NEMU!\n");
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("For help, type \"help\"\n");
}

static inline void load_img() {
  long size;
  Assert(img_file, "Need an image file");
  int ret;

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  Log("The image is %s", img_file);

  fseek(fp, 0, SEEK_END);
  size = ftell(fp);

  fseek(fp, 0, SEEK_SET);
  // load into ddr
  // be careful about memory mapping
  ret = fread(ddr, size, 1, fp);
  assert(ret == 1);

  fclose(fp);
}

static inline void restart() {
  /* Set the initial instruction pointer. */
  cpu.pc = ENTRY_START;
}

static inline void parse_args(int argc, char *argv[]) {
  int o;
  while ( (o = getopt(argc, argv, "-bli:")) != -1) {
    switch (o) {
      case 'b': is_batch_mode = true; break;
      case 'l': log_file = optarg; break;
      case 'i':
                if (img_file != NULL) Log("too much argument '%s', ignored", optarg);
                else img_file = optarg;
                break;
      default:
                panic("Usage: %s [-b] [-l log_file] [-i img_file]", argv[0]);
    }
  }
}

int init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */

  /* Parse arguments. */
  parse_args(argc, argv);

  /* Open the log file. */
  init_log();

  /* Load the image to memory. */
  load_img();

  /* Initialize this virtual computer system. */
  restart();

  /* Compile the regular expressions. */
  // init_regex();

  /* Initialize the watchpoint pool. */
  // init_wp_pool();

  /* Initialize devices. */
  init_device();

  /* Display welcome message. */
  welcome();

  return is_batch_mode;
}
