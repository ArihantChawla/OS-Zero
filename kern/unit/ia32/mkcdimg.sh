#! /bin/sh

#cp zero cdimg/kern
#cp zero cdimg/kern
#genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o cd.iso cdimg/
# use GRUB 2
cp zero cdimg4/zero.img
grub-mkrescue --modules="iso9660 biosdisk" -o cd.iso cdimg4/
#grub-mkimage -v --compression=none -d cdimg2 --format=i386-pc -o cd.iso
#mkisofs -l -iso-level 3 -R -b boot/grub/cdboot.img -no-emul-boot -boot-load-size 4 -boot-info-table -o cd.iso cdimg3
#mkisofs -iso-level 3 -l -D -b boot/grub/cdboot.img -boot-load-size 4 -hide cdboot.img -o cd.iso cdimg3
#grub-mkrescue -o cd.iso -d cdimg3
#echo "grub-mkrescue is broken"
#exit 1

#grub-mkimage --prefix=/boot --config=cdimg2/boot/grub/grub.cfg --directory=cdimg3 --output=cd.iso --format=i386-pc
#mkisofs -l -iso-level 3 -U -R -b boot/grub/cdboot.img -no-emul-boot -boot-load-size 4 -boot-info-table -o cd.iso cdimg3

