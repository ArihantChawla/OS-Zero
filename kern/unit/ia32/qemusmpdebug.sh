#! /bin/sh

qemu-system-i386 -s -S --no-reboot --no-shutdown -vga std -soundhw ac97 -cdrom cd.iso -m 1024 -smp 2

