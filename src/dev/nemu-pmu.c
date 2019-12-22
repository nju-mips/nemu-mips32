#include "device.h"

#define NEMU_PMU_SIZE 0x1000

DEF_DEV(nemu_pmu_dev) = {
    .name = "nemu-pmu",
    .start = CONFIG_NEMU_PMU_BASE,
    .size = NEMU_PMU_SIZE,
};
