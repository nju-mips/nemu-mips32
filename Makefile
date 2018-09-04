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

# CFLAGS += -D__ARCH_MIPS32_R1__ 
CFLAGS += -D__ARCH_LOONGSON__ 

# CFLAGS += -DENABLE_PERF

CFLAGS += -DENABLE_DELAYSLOT
# CFLAGS += -DENABLE_SEGMENT # prior to PAGING
CFLAGS += -DENABLE_PAGING

# enable interrupt will lose about 400 marks
CFLAGS += -DENABLE_INTR
CFLAGS += -DENABLE_EXCEPTION
CFLAGS += -DENABLE_CAE_CHECK # consistence after exception

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

.PHONY: app run debug submit clean
app: $(BINARY)

# IMG ?= $(BUILD_DIR)/nanos-mips32-npc
# IMG ?= $(AM_HOME)/tests/cputest/build/bubble-sort-mips32-npc
# IMG = ~/linux-4.11.4/vmlinux-mips
IMG = ~/u-boot/u-boot

# Command to execute NEMU

$(BINARY): $(OBJS)
	@echo + LD $@
	@$(LD) -O2 -o $@ $^ -lSDL -lreadline

run: $(BINARY)
	$(BINARY) -b -e $(IMG)

debug: $(BINARY)
	$(BINARY) -e $(IMG)

gdb: $(BINARY)
	gdb -s $(BINARY) --args $(NEMU_EXEC)

clean: 
	rm -rf $(BUILD_DIR)
