#include "device.h"

#define PERF_SIZE 0x1000

DEF_DEV(perf_dev) = {
    .name = "perf-counter",
    .start = CONFIG_PERF_COUNTER_BASE,
    .end = CONFIG_PERF_COUNTER_BASE + PERF_SIZE,
};
