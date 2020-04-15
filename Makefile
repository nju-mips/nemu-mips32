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
cfiles-$(CONFIG_NEMU_TRAP) += src/dev/nemu-trap.c
cfiles-$(CONFIG_NEMU_KEYBOARD) += src/dev/nemu-keyboard.c
cfiles-$(CONFIG_NEMU_CLOCK) += src/dev/nemu-clock.c
cfiles-$(CONFIG_NEMU_PMU) += src/dev/nemu-pmu.c
cfiles-$(CONFIG_NEMU_VGA_CTRL) += src/dev/nemu-vga-ctrl.c
cfiles-$(CONFIG_NEMU_VGA) += src/dev/nemu-vga.c
cfiles-$(CONFIG_XLNX_ULITE) += src/dev/xlnx-ulite.c

OBJS := $(cfiles-y:src/%.c=$(OBJ_DIR)/%.o)

# Compilation patterns
$(OBJ_DIR)/%.o: src/%.c Makefile
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -fPIC -c -o $@ $<

# Depencies
-include $(OBJS:.o=.d)

# Some convinient rules

.PHONY: app clean
app: $(BINARY) $(SHARED)

$(BINARY): $(OBJS)
	@echo + LD $@
	@$(LD) -O2 -o $@ $^ -lSDL -lreadline -ldl -lm

$(SHARED): $(OBJS)
	@echo + AR $@
	@$(LD) -O2 -o $@ $^ -shared

clean: 
	rm -rf $(BUILD_DIR) perf.*
	rm -rf include/generated include/config
