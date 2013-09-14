#! /bin/sh

qemu-system-i386 -s -S -soundhw sb16 -cdrom cd.iso -m 512 -smp 2

