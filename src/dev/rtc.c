#include "device.h"
#include "monitor.h"
#include "nemu.h"
#include <stdlib.h>

uint64_t get_current_time(); // us

uint32_t rtc_read(paddr_t addr, int len) {
  check_ioaddr(addr, RTC_SIZE, "RTC");
  return get_current_time() / 1000;
}

device_t rtc_dev = {
    .name = "RTC",
    .start = RTC_ADDR,
    .end = RTC_ADDR + RTC_SIZE,
    .read = rtc_read,
};
