#ifndef __MONITOR_H__
#define __MONITOR_H__

typedef enum { NEMU_STOP, NEMU_RUNNING, NEMU_END } nemu_state_t;
typedef enum {
	MODE_GDB    = 0, /* default work mode */
	MODE_BATCH  = (1 << 0),
   	MODE_DIFF   = (1 << 1) | MODE_BATCH, /* 0x2 | MODE_BATCH */
	MODE_LOG    = (1 << 2) | MODE_BATCH, /* print_registers */
} work_mode_t;

extern nemu_state_t nemu_state;
extern work_mode_t work_mode;

#endif
