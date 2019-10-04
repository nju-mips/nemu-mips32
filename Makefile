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
MCONF := menuconfig/mconf
CONF := menuconfig/conf
CONFIG := ./.config
AUTOCONF_H := include/generated/autoconf.h
INCLUDES  = $(addprefix -I, $(INC_DIR))
CFLAGS   += -O2 -MMD -Wall -Werror -ggdb -fno-strict-aliasing $(INCLUDES)
CFLAGS   += -include $(AUTOCONF_H)
# CFLAGS   += -fsanitize=undefined

# Files to be compiled
SRCS = $(shell find src/ -name "*.c")
OBJS = $(SRCS:src/%.c=$(OBJ_DIR)/%.o)

$(MCONF):
	@cd $(@D) && make -s mconf

$(CONF):
	@cd $(@D) && make -s conf

$(CONFIG): $(MCONF) Kconfig
	@echo + GEN $@
	@$< Kconfig

$(AUTOCONF_H): $(CONFIG)
	@echo + GEN $@
	@$(CONF) --syncconfig ./Kconfig

# Compilation patterns
$(OBJ_DIR)/%.o: src/%.c Makefile $(AUTOCONF_H)
	@echo + CC $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $<

# Depencies
-include $(OBJS:.o=.d)

# Some convinient rules

.PHONY: app clean menuconfig
app: $(BINARY) $(SHARED)

$(BINARY): $(OBJS)
	@echo + LD $@
	@$(LD) -O2 -o $@ $^ -lSDL -lreadline

$(SHARED): $(OBJS)
	@echo + AR $@
	@$(AR) -r -o $@ $^

clean: 
	rm -rf $(BUILD_DIR) perf.*

menuconfig: $(MCONF)
	$(MCONF) ./Kconfig
