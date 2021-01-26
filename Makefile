NAME = nemu
INC_DIR += ./include
BUILD_DIR ?= ./build
OBJ_DIR ?= $(BUILD_DIR)/obj
BINARY ?= $(BUILD_DIR)/$(NAME)
SHARED ?= $(BUILD_DIR)/$(NAME).so

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

dirs-y += src/cpu/
dirs-y += src/monitor/
dirs-y += src/utils/
sdl-cfiles += src/utils/sdlkey.c

cfiles-y += $(filter-out $(sdl-cfiles),$(shell find $(dirs-y) -name "*.c"))
cfiles-y += src/main.c
cfiles-y += src/dev/events.c
cfiles-y += src/dev/register.c
cfiles-y += src/dev/black_hole.c
cfiles-$(CONFIG_BRAM) += src/dev/bram.c
cfiles-$(CONFIG_DDR) += src/dev/ddr.c
cfiles-$(CONFIG_NEMU_TRAP) += src/dev/nemu-trap.c
cfiles-$(CONFIG_NEMU_KEYBOARD) += src/dev/nemu-keyboard.c
cfiles-$(CONFIG_NEMU_CLOCK) += src/dev/nemu-clock.c
cfiles-$(CONFIG_NEMU_PMU) += src/dev/nemu-pmu.c
cfiles-$(CONFIG_NEMU_VGA_CTRL) += src/dev/nemu-vga-ctrl.c
cfiles-$(CONFIG_NEMU_VGA) += src/dev/nemu-vga.c
cfiles-$(CONFIG_NEMU_DISK) += src/dev/nemu-disk.c
cfiles-$(CONFIG_XLNX_ULITE) += src/dev/xlnx-ulite.c
cfiles-$(CONFIG_XLNX_SPI) += src/dev/xlnx-spi.c

cfiles-$(CONFIG_GRAPHICS) += $(sdl-cfiles)
libs-$(CONFIG_GRAPHICS) += -lSDL

OBJS := $(cfiles-y:src/%.c=$(OBJ_DIR)/%.o)

# Compilation patterns
$(OBJ_DIR)/%.o: src/%.c Makefile
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -fPIC -S -o $(@:.o=.s) $<
	@$(CC) $(CFLAGS) -fPIC -c -o $@ $<

# Depencies
-include $(OBJS:.o=.d)

# Some convinient rules

.PHONY: app clean
app: $(BINARY) $(SHARED)

$(BINARY): $(OBJS)
	@echo + LD $@
	@$(LD) -O2 -o $@ $^ $(libs-y)

$(SHARED): $(OBJS)
	@echo + AR $@
	@$(LD) -O2 -o $@ $^ -shared

clean: 
	rm -rf $(BUILD_DIR) perf.*
	rm -rf include/generated include/config
	make -s -C kconfig clean

# for testing
export AM_HOME        = $(PWD)/../nexus-am
export LINUX_HOME     = $(PWD)/../linux
export U_BOOT_HOME    = $(PWD)/../u-boot
export MIPS_TEST_HOME = $(PWD)/../mipstest
export ARCH = mips32-npc
include rules/testcases.mk
