#!/bin/bash

nemu=build/nemu

if make &> /dev/null; then
  echo "NEMU compile OK"
else
  echo "NEMU compile error... exit..."
  exit
fi

echo "compiling testcases..."
if make -C $AM_HOME/tests/cputest ARCH=mips32-npc &> /dev/null; then
  echo "testcases compile OK"
else
  echo "testcases compile error... exit..."
  exit
fi

files=`ls $AM_HOME/tests/cputest/build/*-mips32-npc`
ori_log="build/nemu-log.txt"

for file in $files; do
  base=`basename $file | sed -e 's/-mips32-npc.bin//'`
  printf "[%14s] " $base
  logfile=$base-log.txt
  $nemu -b -e $file &> $logfile

  if (grep 'HIT GOOD TRAP' $logfile > /dev/null) then
    echo -e "\033[1;32mPASS!\033[0m"
    rm $logfile
  else
    echo -e "\033[1;31mFAIL!\033[0m see $logfile for more information"
    if (test -e $ori_log) then
      echo -e "\n\n===== the original log.txt =====\n" >> $logfile
      cat $ori_log >> $logfile
    fi
  fi
done
