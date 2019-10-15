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

UBOOT_HOME := $(abspath ../uboot)
BUSYBOX_HOME := $(shell echo ~)/busybox-1.20.1
export KERNEL_HOME := $(shell echo ~)/linux-noop-4.11.4
# CFLAGS   += -fsanitize=undefined

# Files to be compiled
SRCS = $(shell find src/ -name "*.c")
OBJS = $(SRCS:src/%.c=$(OBJ_DIR)/%.o)

$(MCONF):
	@cd $(@D) && make -s mconf

$(CONF):
	@cd $(@D) && make -s conf

$(CONFIG): $(CONF) Kconfig
	@echo + GEN $@
	@$(CONF) --syncconfig ./Kconfig

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
	@$(LD) -O2 -o $@ $^ -lSDL -lreadline -ldl

$(SHARED): $(OBJS)
	@echo + AR $@
	@$(AR) -r -o $@ $^

clean: 
	rm -rf $(BUILD_DIR) perf.*
	rm -rf include/generated include/config

menuconfig: $(MCONF)
	@$(MCONF) ./Kconfig

%_defconfig:
	@cp configs/$@ ./.config


.PHONY: busybox linux initramfs uImage run-linux

busybox:
	make -s -C $(BUSYBOX_HOME) ARCH=mips CROSS_COMPILE=mipsel-linux-gnu- -j32
	cp $(BUSYBOX_HOME)/busybox ../initramfs/root/bin/busybox

initramfs:
	make -s -C ../initramfs

linux:
	make -C $(KERNEL_HOME) ARCH=mips CROSS_COMPILE=mips-linux-gnu- -j32 uImage

uImage: busybox initramfs linux

run-linux:
	build/nemu -b -e $(UBOOT_HOME)/u-boot
