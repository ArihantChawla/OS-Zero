#! /bin/sh

gcc -O -fno-omit-frame-pointer -Wall -Wextra -Wundef -mcmodel=kernel -m64 -ffreestanding -nostdlib
ld -nostdlib -nodefaultlibs -T kern.lds -o zero

