#!/bin/bash

if ! [ -f "$1" ] || ! readelf -h "$1" &> /dev/null; then
  echo "ELF file required"
  exit 0
fi

if [ s"$TMUX" == s"" ]; then
  echo "please open a tmux session firstly"
  exit 0
fi

CURRENT_PANE=${TMUX_PANE}
TARGET_PANE=$(tmux split-window -P -F "#{pane_id}" qemu-system-mipsel -kernel "$1" -S -s -monitor telnet:localhost:1111,server,nowait --nographic)
tmux select-pane -t $CURRENT_PANE
gdb-multiarch -ex "set arch mips" -ex "target remote localhost:1234" -ex "symbol $1"
tmux kill-pane -t "$TARGET_PANE"
