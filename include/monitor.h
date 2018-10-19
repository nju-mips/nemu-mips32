#ifndef __MONITOR_H__
#define __MONITOR_H__

typedef enum { NEMU_STOP, NEMU_RUNNING, NEMU_END } nemu_state_t;
typedef enum {
	MODE_GDB    = 0, /* default work mode */
	MODE_BATCH  = (1 << 0),
   	MODE_DIFF   = (1 << 1) | MODE_BATCH, /* 0x2 | MODE_BATCH */
	MODE_LOG    = (1 << 2) | MODE_BATCH, /* print_registers */
} work_mode_t;

#ifdef __cplusplus

static inline work_mode_t &operator |=(work_mode_t &mode, int extra) {
  return mode = static_cast<work_mode_t>(static_cast<int>(mode) | extra);
}

#endif

extern nemu_state_t nemu_state;
extern work_mode_t work_mode;

#endif
