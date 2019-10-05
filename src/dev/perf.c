#if CONFIG_PERFCOUNTER
#  include "device.h"

#  define PERF_SIZE 0x1000

DEF_DEV(perf_dev) = {
    .name = "perf-counter",
    .start = CONF_PERF_COUNTER_BASE,
    .end = CONF_PERF_COUNTER_BASE + PERF_SIZE,
};
#endif
