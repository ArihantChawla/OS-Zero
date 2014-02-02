#! /bin/sh

qemu-system-i386 -s -S -soundhw ac97 -cdrom cd.iso -m 512
#qemu-system-i386 -s -S -vga std -cdrom cd.iso -m 512

