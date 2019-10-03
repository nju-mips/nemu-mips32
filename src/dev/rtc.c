#include <stdlib.h>

#include "device.h"

#define RTC_ADDR 0x10002000
#define RTC_SIZE 0x4

uint64_t get_current_time(); // us

static uint32_t rtc_read(paddr_t addr, int len) {
  check_ioaddr(addr, len, RTC_SIZE, "rtc.read");
  return get_current_time() / 1000;
}

device_t rtc_dev = {
    .name = "RTC",
    .start = RTC_ADDR,
    .end = RTC_ADDR + RTC_SIZE,
    .read = rtc_read,
    .peek = rtc_read,
};
