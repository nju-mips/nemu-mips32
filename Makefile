NAME = nemu
INC_DIR += ./include
BUILD_DIR ?= ./build
OBJ_DIR ?= $(BUILD_DIR)/obj
BINARY ?= $(BUILD_DIR)/$(NAME)
SHARED ?= $(BUILD_DIR)/$(NAME).a

.DEFAULT_GOAL = app

-include config.mk
-include linux.mk

# Compilation flags
CC = gcc
LD = gcc
AR = ar
INCLUDES  = $(addprefix -I, $(INC_DIR))
CFLAGS   += -O2 -MMD -Wall -Werror -ggdb -fno-strict-aliasing $(INCLUDES)
CFLAGS   += -include include/generated/autoconf.h
# CFLAGS   += -fsanitize=undefined

# Files to be compiled
SRCS = $(shell find src/ -name "*.c")

libs-y += src/cpu/
libs-y += src/monitor/
libs-y += src/utils/

cfiles-y += $(shell find $(libs-y) -name "*.c")
cfiles-y += src/main.c
cfiles-y += src/dev/events.c
cfiles-y += src/dev/register.c
cfiles-y += src/dev/black_hole.c
cfiles-$(CONFIG_BRAM) += src/dev/bram.c
cfiles-$(CONFIG_DDR) += src/dev/ddr.c
cfiles-$(CONFIG_GPIO) += src/dev/gpio.c
cfiles-$(CONFIG_KEYBOARD) += src/dev/keyboard.c
cfiles-$(CONFIG_RTC) += src/dev/rtc.c
cfiles-$(CONFIG_PERF_COUNTER) += src/dev/perf.c
cfiles-$(CONFIG_VGA_CONTROLLER) += src/dev/screen.c
cfiles-$(CONFIG_VGA) += src/dev/vga.c
cfiles-$(CONFIG_UARTLITE) += src/dev/uartlite.c
cfiles-$(CONFIG_ETHERLITE) += src/dev/etherlite.c
cfiles-$(CONFIG_XILINX_SPI) += src/dev/xilinx-spi.c

OBJS := $(cfiles-y:src/%.c=$(OBJ_DIR)/%.o)

# Compilation patterns
$(OBJ_DIR)/%.o: src/%.c Makefile prepare
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $<

# Depencies
-include $(OBJS:.o=.d)

# Some convinient rules

.PHONY: app clean
app: $(BINARY) $(SHARED)

$(BINARY): $(OBJS)
	@echo + LD $@
	@$(LD) -O2 -o $@ $^ -lSDL -lreadline -ldl

$(SHARED): $(OBJS)
	@echo + AR $@
	@$(AR) -r -o $@ $^

clean: 
	rm -rf $(BUILD_DIR) perf.*
	rm -rf include/generated include/config
