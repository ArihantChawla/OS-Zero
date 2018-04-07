#! /bin/sh

#qemu-system-i386 -soundhw ac97,sb16 -cdrom cd.iso -m 2048
qemu-system-i386 -cdrom cd.iso -m 2048 -d guest_errors

