#include "common.h"
#include "cheat.h"
#include "cpu/core.h"


/*
static uint32_t saved_gprs[32];

static void nemu_save_gpr() {
}

static void nemu_check_gpr() {
}


static void nemu_save_mem() {
}

static void nemu_save_cpr0() {
}


static void nemu_check_mem() {
}

static void nemu_check_cpr0() {
}
*/

void cheat_under_nemu(void) {
  switch(cpu.gpr[0]) {
    case NEMU_SAVE_GPR:
	case NEMU_SAVE_MEM:
	case NEMU_SAVE_CPR0:
	case NEMU_CHECK_GPR:
	case NEMU_CHECK_MEM:
	case NEMU_CHECK_CPR0:
	default: break;
  }
}
