# MIPS32-NEMU

与noop同步的模拟器

## 内存映射

MIPS32-NPC中的内存映射定义如下（来自nexus-am/am/arch/mips32-npc/README.md）：
```
0xbfc00000 - 0xbfcfffff: BRAM
0x80000000 - 0x8fffffff: DDR (cached)
0xa0000000 - 0xafffffff: DDR (uncached)
0xb0000000 - 0xb0000fff: GPIO-trap
0xbfe94000 - 0xbfe94fff: Keyboard
0xbff00000 - 0xbff0ffff: Mac (almost completed)
0xbfe95000 - 0xbfe95fff: PerfCounter
0xb0002000 - 0xb0002fff: RTC
0xb0003000 - 0xb0003fff: screen config
0xbfe50000 - 0xbfe50fff: uartlite serial
0xbfe80000 - 0xbfe80fff: spi flash
0xb0400000 - 0xb04fffff: video memory
```

## 编译与运行

* 配置nemu-mips32
  * `make menuconfig`, 手动配置
  * `make xx_defconfig`, 使用缺省配置
* 编译
  * `make`
* 运行elf文件
  * `build/nemu -b -e xx.elf`
* 运行binary文件
  * `build/nemu -b -i xx.elf`
* 调试elf文件，请先安装gdb-multiarch
  * `build/nemu -e xx.elf`

## linux

* how to run linux on nemu-mips32 ?
  * `git clone https://github.com/nju-mips/minilab -b run-linux`
  * `cd minilab && git clone https://github.com/nju-mips/nemu-mips32 nemu -b M2`
  * `cd M2 && make run`

## configs

* `make microbench_defconfig`,专门为microbench优化的配置，暂时不可用
* `make mips32r1_defconfig`，运行linux/debian所用的配置，没有窗口，跑不了microbench
* `make noop_defconfig`,用于和noop/woop差分测试的配置，可以运行microbench

## linux configs
* `ARCH=mips make noop_debian_defconfig`，运行debian的，有完整的网络栈，镜像比较大
* `ARCH=mips make noop_defconfig`，用来在noop/woop上差分测试的镜像
* `ARCH=mips make noop_emu_defconfig`，同上，不过大小非常小，用来快速验证
* `ARCH=mips make noop_spi_defconfig`，启用了spi，可以从spi flash启动busybox/systemd
* `ARCH=mips make noop_legacy_boot_defconfig`，可以在命令行指定initrd的地址，进而启动1号进程
