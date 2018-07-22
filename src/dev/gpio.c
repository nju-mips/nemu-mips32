#include <stdlib.h>
#include "nemu.h"
#include "monitor.h"


#define check_gpio(addr, len) \
  Assert(addr == 0, \
	  "address(0x%08x) is out side GPIO", addr); \
	  Assert(len == 1, "GPIO only allow byte read/write");

#define ANSI_WIDTHOR_RED     "\x1b[31m"
#define ANSI_WIDTHOR_GREEN   "\x1b[32m"
#define ANSI_WIDTHOR_YELLOW  "\x1b[33m"
#define ANSI_WIDTHOR_BLUE    "\x1b[34m"
#define ANSI_WIDTHOR_MAGENTA "\x1b[35m"
#define ANSI_WIDTHOR_CYAN    "\x1b[36m"
#define ANSI_WIDTHOR_RESET   "\x1b[0m"


void gpio_write(paddr_t addr, int len, uint32_t data) {
  check_gpio(addr, len);
  if ((unsigned char)data == 0) {
	eprintf(ANSI_WIDTHOR_GREEN "HIT GOOD TRAP\n" ANSI_WIDTHOR_RESET);
  }
  else
	eprintf(ANSI_WIDTHOR_RED "HIT BAD TRAP code: %d\n" ANSI_WIDTHOR_RESET, (unsigned char)data == 0);
  nemu_state = NEMU_END;
  // directly exit, so that we will not print one more commit log
  // which makes it easier for crosschecking.
  if(work_mode & MODE_BATCH) exit(0);
}
