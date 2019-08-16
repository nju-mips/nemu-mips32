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
CFLAGS   += -O2 -MMD -Wall -Werror -ggdb -fno-strict-aliasing $(INCLUDES)

# CFLAGS += -D__ARCH_MIPS32_R1__ 
CFLAGS += -D__ARCH_LOONGSON__ 

CFLAGS += -DENABLE_DELAYSLOT
# CFLAGS += -DENABLE_SEGMENT # prior to PAGING
CFLAGS += -DENABLE_PAGING

# CFLAGS += -DENABLE_INTR
CFLAGS += -DENABLE_EXCEPTION
# CFLAGS += -DENABLE_CAE_CHECK # consistence after exception

# enable interrupt will lose about 600 marks
# CFLAGS += -DDEBUG

# Files to be compiled
SRCS = $(shell find src/ -name "*.c")
OBJS = $(SRCS:src/%.c=$(OBJ_DIR)/%.o)

# Compilation patterns
$(OBJ_DIR)/%.o: src/%.c Makefile
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $<

# Depencies
-include $(OBJS:.o=.d)

# Some convinient rules

.PHONY: app run debug submit clean
app: $(BINARY) $(SHARED)

# IMG ?= $(BUILD_DIR)/nanos-mips32-npc
# IMG ?= $(AM_HOME)/tests/cputest/build/bubble-sort-mips32-npc
# IMG = ~/linux-4.11.4/vmlinux-mips
IMG = ~/u-boot/u-boot
ARGS ?= -b -e $(IMG)

# Command to execute NEMU

$(BINARY): $(OBJS)
	@echo + LD $@
	@$(LD) -O2 -o $@ $^ -lSDL -lreadline

$(SHARED): $(OBJS)
	@echo + AR $@
	@$(AR) -r -o $@ $^

run: $(BINARY)
	$(BINARY) $(ARGS)

debug: $(BINARY)
	$(BINARY) -e $(IMG)

gdb: $(BINARY)
	gdb -s $(BINARY) --args $(BINARY) $(ARGS)

clean: 
	rm -rf $(BUILD_DIR)
