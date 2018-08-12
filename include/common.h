#ifndef __COMMON_H__
#define __COMMON_H__

#define DEBUG

/* You will define this macro in PA2 */
//#define HAS_IOE

#include "debug.h"
#include "macro.h"

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

typedef uint32_t rtlreg_t;

typedef uint32_t paddr_t;
typedef uint32_t vaddr_t;

typedef uint16_t ioaddr_t;

/* VT 102 colored text */
#define ANSI_WIDTHOR_RED     "\e[31m"
#define ANSI_WIDTHOR_GREEN   "\e[32m"
#define ANSI_WIDTHOR_YELLOW  "\e[33m"
#define ANSI_WIDTHOR_BLUE    "\e[34m"
#define ANSI_WIDTHOR_MAGENTA "\e[35m"
#define ANSI_WIDTHOR_CYAN    "\e[36m"
#define ANSI_WIDTHOR_RESET   "\e[0m"

#endif
