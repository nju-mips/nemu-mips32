NAME = nemu
INC_DIR += ./include
BUILD_DIR ?= ./build
OBJ_DIR ?= $(BUILD_DIR)/obj
BINARY ?= $(BUILD_DIR)/$(NAME)

.DEFAULT_GOAL = app

# Compilation flags
CC = gcc
LD = gcc
INCLUDES  = $(addprefix -I, $(INC_DIR))
CFLAGS   += -O2 -MMD -Wall -Werror -ggdb $(INCLUDES)

# ddr start at 0x10000000 or 0x1000000
# CFLAGS += -D__ARCH_MIPS32_NPC__ 

CFLAGS += -DENABLE_DELAYSLOT
# CFLAGS += -DENABLE_SEGMENT

# enable interrupt will lose about 400 marks
CFLAGS += -DENABLE_INTR

# no action indeed
# CFLAGS += -DDEBUG


# Files to be compiled
SRCS = $(shell find src/ -name "*.c")
OBJS = $(SRCS:src/%.c=$(OBJ_DIR)/%.o)

# Compilation patterns
$(OBJ_DIR)/%.o: src/%.c
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $<

# Depencies
-include $(OBJS:.o=.d)

# Some convinient rules

.PHONY: app run submit clean
app: $(BINARY)

# ARGS ?= -l $(BUILD_DIR)/nemu-log.txt -i $(BUILD_DIR)/nanos-mips32-npc.bin
# ARGS ?= -b -e $(AM_HOME)/tests/cputest/build/bubble-sort-mips32-npc
ARGS ?= -e ~/linux-4.11.4/vmlinux-mips

# Command to execute NEMU
NEMU_EXEC := $(BINARY) $(ARGS)

$(BINARY): $(OBJS)
	@echo + LD $@
	@$(LD) -O2 -o $@ $^ -lSDL -lreadline

run: $(BINARY)
	$(NEMU_EXEC)

gdb: $(BINARY)
	gdb -s $(BINARY) --args $(NEMU_EXEC)

clean: 
	rm -rf $(BUILD_DIR)
