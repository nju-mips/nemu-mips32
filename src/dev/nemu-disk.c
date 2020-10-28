#include "device.h"
#include "utils/file.h"

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#define NEMUDISK_SIZE (256 * 1024 * 1024) // 0x08000000

#define ND_BUF_SR 0
#define ND_ADDR_SR 4
#define ND_LEN_SR 8
#define ND_OP_SR 12
#define ND_END 16

#define ND_OP_RD 0
#define ND_OP_WR 1

static const char *nd_disk_file = NULL;
static int nd_fd = -1;
static uint32_t nd_regs[ND_END / 4];

/* Memory accessing interfaces */

static uint32_t nemudisk_read(paddr_t addr, int len) {
  check_ioaddr(addr, len, NEMUDISK_SIZE, "nemudisk.read");
  switch (addr) {
  case ND_LEN_SR: return get_file_size(nd_disk_file);
  default: return 0;
  }
  return 0;
}

static void nemudisk_write(
    paddr_t addr, int len, uint32_t data) {
  switch (addr) {
  case ND_BUF_SR:
  case ND_ADDR_SR:
  case ND_LEN_SR:
    nd_regs[addr >> 2] = data;
    kdbg_print_backtraces();
    break;
  case ND_OP_SR: {
    if (nd_fd < 0) break;

    uint32_t nd_buf = nd_regs[ND_BUF_SR >> 2];
    uint32_t nd_addr = nd_regs[ND_ADDR_SR >> 2];
    uint32_t nd_len = nd_regs[ND_LEN_SR >> 2];
    int nd_op = data;

    void *p = vaddr_map(nd_buf, nd_len);
    assert (p && "bad arguments received by nemudisk");
    if (nd_op == ND_OP_RD) {
      /* read */
      lseek(nd_fd, nd_addr, SEEK_SET);
      int curl = read(nd_fd, p, nd_len);
      (void)curl;
    } else if (nd_op == ND_OP_WR) {
      /* write */
      lseek(nd_fd, nd_addr, SEEK_SET);
      int curl = write(nd_fd, p, nd_len);
      (void)curl;
    }
  }
  }
}

static void nemudisk_set_blkio_file(const char *file) {
  nd_disk_file = file;
  nd_fd = open(file, O_RDWR);
}

DEF_DEV(nemudisk_dev) = {
    .name = "nemu-disk",
    .start = CONFIG_NEMU_DISK_BASE,
    .size = 0x1000,
    .read = nemudisk_read,
    .write = nemudisk_write,
    .peek = nemudisk_read,
    .set_blkio_file = nemudisk_set_blkio_file,
};
