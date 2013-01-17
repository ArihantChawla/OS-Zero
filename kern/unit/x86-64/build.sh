#! /bin/sh

gcc -mcmodel=kernel -m64 -ffreestanding -nostdlib -mno-red-zone
ld -nostdlib -nodefaultlibs -T kern.lds -o zero

