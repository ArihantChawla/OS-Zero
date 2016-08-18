#! /bin/sh

topdir=`pwd`
echo Building kernel
./build.sh
echo Creating bootable CD image...
cp zero cdimg2/boot/zero
rm -f `find cdimg2 -name TAGS`
./build/bin/grub-mkimage --prefix=/boot/grub \
 --directory=./build/lib/grub/i386-pc --format=i386-pc --output=core.img \
 loadenv biosdisk part_gpt part_msdos fat \
 iso9660 search ls chain boot configfile vbe vga gfxterm font multiboot
cat ./grub/grub-core/cdboot.img core.img > cdimg2/grub.img
#genisoimage -A "ZERO" -input-charset "iso8859-1" -R -b grub.img -no-emul-boot \
#    -boot-load-size 4 -boot-info-table -o cd.iso cdimg2
genisoimage -v -A "ZERO" -input-charset "iso8859-1" -R -b grub.img \
 -boot-load-size 4 -no-emul-boot -boot-info-table -o cd.iso cdimg2

