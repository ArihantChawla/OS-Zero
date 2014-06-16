topdir=`pwd`
echo Building kernel
./build.sh
echo Creating bootable CD image...
cp zero cdimg2/kern
./build/bin/grub-mkimage --prefix="$topdir/build" --format=i386-pc --output=core.img \
    --config="cdimg2/boot/grub/grub.cfg" loadenv biosdisk part_msdos part_gpt fat ntfs \
    ext2 ntfscomp iso9660 loopback search linux boot minicmd cat cpuid chain \
    halt help ls reboot echo test configfile normal sleep memdisk tar font \
    gfxterm gettext true vbe vga video_bochs video_cirrus multiboot vbe
cat ./build/lib/grub/i386-pc/cdboot.img core.img > cdimg2/grub.img
genisoimage -A "ZERO" -input-charset "iso8859-1" -R -b grub.img -no-emul-boot \
    -boot-load-size 4 -boot-info-table -o cd.iso cdimg2

