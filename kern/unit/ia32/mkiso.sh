# Thanks for this script go to pecorade; I just did cosmetic changes... // vendu

#! /bin/sh
ISO_DIRECTORY="zero_iso"
KERN_IMAGE="zero"

echo "Building kernel"
./build.sh

echo "Erasing old image tree"
rm -rf $ISO_DIRECTORY

echo "Creating image tree..."
mkdir -p $ISO_DIRECTORY/boot/grub

echo "Creating GRUB2 files..."
touch $ISO_DIRECTORY/boot/grub/grub.cfg
echo "set timeout=15" > $ISO_DIRECTORY/boot/grub/grub.cfg
echo "set default=0 # Set the default menu entry" >> $ISO_DIRECTORY/boot/grub/grub.cfg
echo "menuentry \"Zero\" {" >> $ISO_DIRECTORY/boot/grub/grub.cfg
echo "    multiboot /zero" >> $ISO_DIRECTORY/boot/grub/grub.cfg
echo "    boot" >> $ISO_DIRECTORY/boot/grub/grub.cfg
echo "}" >> $ISO_DIRECTORY/boot/grub/grub.cfg

echo "Copying kernel image..."
cp $KERN_IMAGE ./$ISO_DIRECTORY

echo "Building boot disk..."
./build/bin/grub-mkrescue -o cd.iso $ISO_DIRECTORY > /dev/null

