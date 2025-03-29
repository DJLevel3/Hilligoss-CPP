#!/bin/sh
set -e
N=$(printf %s "$1"|sed -n 's/.*\([1-9][0-9]*$\)/\1/p' )

ROWS=$(tput lines)
COLS=$(tput cols)

COLS=$((COLS / 2))
COLS=$((COLS - 2))

ROWS=$((ROWS -9 ))

./hilligoss-nnsts-512 -f "$1.pgm" -c 2000 -b30 -w255 -x$COLS -y$ROWS -j1600 -s

# 48000 Hz / 60 FPS = 800 vectors per frame
# 48000 Hz / 24 FPS = 2000 vectors per frame
# 192000 Hz / 60 FPS (4x Oversampling) = 3200 vectors per frame
# 192000 Hz / 60 FPS (4x Oversampling) = 8000 vectors per frame 
