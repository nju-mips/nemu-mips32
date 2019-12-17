.PHONY: busybox linux initramfs uImage run-linux

UBOOT_HOME := $(abspath ../uboot)
BUSYBOX_HOME := $(shell echo ~)/busybox-1.20.1
export KERNEL_HOME := $(shell echo ~)/linux-noop-4.11.4

busybox:
	make -s -C $(BUSYBOX_HOME) ARCH=mips CROSS_COMPILE=mipsel-linux-gnu- -j32
	cp $(BUSYBOX_HOME)/busybox ../initramfs/root/bin/busybox

initramfs:
	make -s -C ../initramfs

linux:
	make -C $(KERNEL_HOME) ARCH=mips CROSS_COMPILE=mips-linux-gnu- -j32 uImage

# uImage: busybox initramfs linux

run-linux:
	build/nemu -b -e $(UBOOT_HOME)/u-boot

