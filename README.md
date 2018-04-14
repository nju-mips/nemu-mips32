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

现阶段实现了DDR内存，UART,GPIO仅支持写，不支持读，BRAM还有VGA未实现。

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
更多运行选项：
-b batch_mode：在batch mode下会直接运行，不会调用monitor，同时所有Log宏都不会输出，主要用于crosscheck。
-c cross_check_mode：在crosscheck mode下，每次运行外一条指令后，就会输出当前的pc、gpr以及hi、lo的信息，主要用于crosscheck。

## 阶段目标

- [] 通过所有cputest。（现在还有unaligned未通过）。
