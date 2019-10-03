#include <stdlib.h>

#include "device.h"
#include "monitor.h"

#define GPIO_BASE 0x10000000
#define GPIO_SIZE 0x1000

static void gpio_write(paddr_t addr, int len, uint32_t data) {
  check_ioaddr(addr, len, 4, "GPIO.write");
  if (data == 0) {
    eprintf(ANSI_WIDTHOR_GREEN
            "HIT GOOD TRAP\n" ANSI_WIDTHOR_RESET);
  } else {
    eprintf(ANSI_WIDTHOR_RED
            "HIT BAD TRAP code: %d\n" ANSI_WIDTHOR_RESET,
            data);
  }

  nemu_state = NEMU_END;
  // directly exit, so that we will not print one more
  // commit log which makes it easier for crosschecking.
  if (work_mode & MODE_BATCH) nemu_exit();
}

device_t gpio_dev = {
    .name = "GPIO",
    .start = GPIO_BASE,
    .end = GPIO_BASE + GPIO_SIZE,
    .write = gpio_write,
};
