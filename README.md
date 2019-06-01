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
0xbff00000 - 0xbff0ffff: Mac (not completed)
0xbfe95000 - 0xbfe95fff: PerfCounter (meaningless when simulate)
0xb0002000 - 0xb0002fff: RTC
0xb0003000 - 0xb0003fff: screen config
0xbfe50000 - 0xbfe50fff: uartlite serial
0xbfe80000 - 0xbfe80fff: spi flash (not completed)
0xb0400000 - 0xb04fffff: video memory
```

## 编译
```
make
```

## 运行
```
./build/nemu -e *.elf
```
