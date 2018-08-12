#include <stdlib.h>
#include "nemu.h"
#include "monitor.h"
#include "device.h"

#ifdef __ARCH_LOONGSON__

/* avoid overflow */
static uint8_t confreg[CONFREG_SIZE + 1];

#define check_confreg(addr, len) \
  CPUAssert(addr <= CONFREG_SIZE, "address(0x%08x) is out side CONFREG", addr);

uint32_t confreg_read(paddr_t addr, int len) {
  check_confreg(addr, len);
  switch(addr) {
	case CONFREG_SIMU_FLAG_ADDR: return 1;
  }
  return *((uint32_t *)((uint8_t *)confreg + addr)) & (~0u >> ((4 - len) << 3));
}

void confreg_write(paddr_t addr, int len, uint32_t data) {
  check_confreg(addr, len);
  memcpy((uint8_t *)confreg + addr, &data, len);

  uint32_t *led_rg0 = (void*)&confreg[CONFREG_LED_RG0_ADDR];
  uint32_t *led_rg1 = (void*)&confreg[CONFREG_LED_RG1_ADDR];

  if(*led_rg0 != *led_rg1) { return; }

  if(*led_rg0 == CONFREG_LED_RG_GREEN) {
	eprintf(ANSI_WIDTHOR_GREEN "HIT GOOD TRAP\n" ANSI_WIDTHOR_RESET);
  } else if(*led_rg0 == CONFREG_LED_RG_RED) {
	eprintf(ANSI_WIDTHOR_RED "HIT BAD TRAP code: %d\n" ANSI_WIDTHOR_RESET, (unsigned char)data == 0);
  } else {
	return;
  }

  nemu_state = NEMU_END;
  if(work_mode & MODE_BATCH) exit(0);
}


#endif
