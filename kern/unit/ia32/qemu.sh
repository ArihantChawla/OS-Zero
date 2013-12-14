#! /bin/sh

#qemu-system-i386 -soundhw ac97 -cdrom cd.iso -m 256
qemu-system-i386 -vga std -cdrom cd.iso -m 512

