#ifndef COMMON_H
#define COMMON_H

#include "debug.h"

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

typedef uint32_t rtlreg_t;

typedef uint32_t paddr_t;
typedef uint32_t vaddr_t;

typedef uint16_t ioaddr_t;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

#define glue_prim(a, b) a ## b
#define glue(a, b) glue_prim(a, b)

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

/* #define CONFIG_X 0
 * #define CONFIG_Y 1
 * #undef  CONFIG_O
 * CONFIG_IS_ENABLED(X) ==> 0
 * CONFIG_IS_ENABLED(Y) ==> 1
 * CONFIG_IS_ENABLED(O) ==> 0
 * */
#define _2nd_of_N_impl(a, b, ...) b
#define _2nd_of_N(...) _2nd_of_N_impl(__VA_ARGS__, 0)
#define EXPAND(a) glue_prim(EXPAND_, a)
#define EXPAND_0 _, 0
#define EXPAND_1 _, 1
#define CONFIG_IS_ENABLED(v) _2nd_of_N(EXPAND(glue(CONFIG_, v)), 0)

#endif
