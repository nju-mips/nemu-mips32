# MIPS32-NEMU

MIPS32-NEMU是从ICS2017/nemu派生出来的MIPS32模拟器，经过了大量的精简，去除了diff testing，以及大量的macro。模拟器采用的是内存映射IO，在映射以及外设定义方面，应尽量与MIPS32-NPC保持一致。

## 内存映射

MIPS32-NPC中的内存映射定义如下（来自nexus-am/am/arch/mips32-npc/README.md）：
```
0x00000000 - 0x00001fff: BRAM
0x10000000 - 0x1fffffff: DDR
0x40000000 - 0x40000fff: GPIO-trap
0x40001000 - 0x40001fff: Uartlite
0x40010000 - 0x4001ffff: VGA
```

先阶段仅实现了DDR内存，其他外设暂时还未实现。

## Monitor
现阶段monitor只实现了info r打印寄存器，还有si单步执行，这些代码都是直接从ics2015中搬过去的。在调试过程中，请根据需要给monitor增加功能。

## 编译
```
make
```

## 运行
将nexus-am/tests/cputest/build下的\*.bin文件拷贝到build目录下，运行
```
./nemu -i *.bin
```
注意，由于我们在模拟器中并没有loader，请务必运行经过了objcopy处理过的\*.bin文件，而不是运行直接运行非.bin结尾的可执行文件。

## 阶段目标
通过所有cputest。
