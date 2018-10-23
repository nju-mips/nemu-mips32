#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

typedef uint32_t rtlreg_t;

typedef uint32_t paddr_t;
typedef uint32_t vaddr_t;

typedef uint16_t ioaddr_t;

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

#include "debug.h"
#include "macro.h"

#define BIT(nr)		(1 << (nr))
#define GENMASK(h, l) \
	(((~0UL) << (l)) & (~0UL >> (BITS_PER_LONG - 1 - (h))))

#define GENMASK_ULL(h, l) \
	(((~0ULL) << (l)) & (~0ULL >> (BITS_PER_LONG_LONG - 1 - (h))))

/* VT 102 colored text */
#define ANSI_WIDTHOR_RED     "\e[31m"
#define ANSI_WIDTHOR_GREEN   "\e[32m"
#define ANSI_WIDTHOR_YELLOW  "\e[33m"
#define ANSI_WIDTHOR_BLUE    "\e[34m"
#define ANSI_WIDTHOR_MAGENTA "\e[35m"
#define ANSI_WIDTHOR_CYAN    "\e[36m"
#define ANSI_WIDTHOR_RESET   "\e[0m"

#endif
