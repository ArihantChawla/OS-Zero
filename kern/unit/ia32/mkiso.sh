#! /bin/sh

# Thanks for this script go to pecorade; I just did cosmetic changes // vendu

. ./kern.sh

build()
{
    echo "erasing old build tree"
    rm -rf $GRUB_BUILD_DIR
    mkdir -p $GRUB_BOOT_DIR
    echo "Building kernel"
    ./build-gcc.sh
    echo "copying kernel image to $GRUB_BOOT_DIR/$KERN_IMAGE"
    cp -pf $KERN_IMAGE $GRUB_BOOT_DIR

    return 0
}

mkgrub()
{
    echo "building GRUB"

    ./mkgrub.sh
}

mkimg()
{
    echo "building boot disc image"
    $GRUB_BUILD_DIR/bin/grub-mkimage --prefix=/boot/grub --format=i386-pc --output=zero.img $GRUB_MODS
    cat $GRUB_BUILD_DIR/lib/grub/$GRUB_ARCH/cdboot.img zero.img > $KERN_ISO_DIR/core.img
    genisoimage -v -A "ZERO" --input-charset "iso8859-1" -R -b core.img -no-emul-boot -boot-load-size 4 -boot-info-table -o $KERN_ISO_IMAGE $KERN_ISO_DIR
}

mkiso()
{
    build
    mkgrub $@
    mkimg
}

mkiso $@

