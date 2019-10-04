NAME = nemu
INC_DIR += ./include
BUILD_DIR ?= ./build
OBJ_DIR ?= $(BUILD_DIR)/obj
BINARY ?= $(BUILD_DIR)/$(NAME)
SHARED ?= $(BUILD_DIR)/$(NAME).a

.DEFAULT_GOAL = app

# Compilation flags
CC = gcc
LD = gcc
AR = ar
INCLUDES  = $(addprefix -I, $(INC_DIR))
AUTOCONF_H := include/generated/autoconf.h
CFLAGS   += -O2 -MMD -Wall -Werror -ggdb -fno-strict-aliasing $(INCLUDES)
CFLAGS   += -include $(AUTOCONF_H)
# CFLAGS   += -fsanitize=undefined

# Files to be compiled
SRCS = $(shell find src/ -name "*.c")
OBJS = $(SRCS:src/%.c=$(OBJ_DIR)/%.o)

menuconfig/mconf:
	cd $(@D) && make

$(AUTOCONF_H): Kconfig menuconfig/mconf
	menuconfig/mconf ./Kconfig

# Compilation patterns
$(OBJ_DIR)/%.o: src/%.c Makefile $(AUTOCONF_H)
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $<

# Depencies
-include $(OBJS:.o=.d)

# Some convinient rules

.PHONY: app run debug submit clean menuconfig
app: $(BINARY) $(SHARED)

# IMG ?= $(BUILD_DIR)/nanos-mips32-npc
# IMG ?= $(AM_HOME)/tests/cputest/build/bubble-sort-mips32-npc
# IMG = ~/linux-4.11.4/vmlinux-mips
# Command to execute NEMU

$(BINARY): $(OBJS)
	@echo + LD $@
	@$(LD) -O2 -o $@ $^ -lSDL -lreadline

$(SHARED): $(OBJS)
	@echo + AR $@
	@$(AR) -r -o $@ $^

gdb: $(BINARY)
	gdb -s $(BINARY) --args $(BINARY) $(ARGS)

clean: 
	rm -rf $(BUILD_DIR)

menuconfig:
	menuconfig/mconf ./Kconfig
