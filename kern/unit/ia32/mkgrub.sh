#! /bin/sh

. ./kern.sh

mkcfg()
{
    echo "creating GRUB configuration file $GRUB_CFG_FILE"
    mkdir -p $GRUB_CFG_DIR
    touch $GRUB_CFG_FILE
    echo "setting parameters"
    cat > $GRUB_CFG_FILE <<EOF
set timeout=$GRUB_TIMEOUT # wait seconds before start
set default=$GRUB_DEFAULT # default menu entry

EOF
    echo "insmod:"
    for mod in $GRUB_MODS
    do
	echo " $mod"
	echo "insmod $mod" >> $GRUB_CFG_FILE
    done
    echo "configuring graphics mode"

    cat >> $GRUB_CFG_FILE <<EOF

set gfxmode=$GRUB_GFXMODE
set gfxpayload=$GRUB_GFXMODE
EOF
    echo "creating boot menu"
    cat >> $GRUB_CFG_FILE <<EOF

menuentry "zero" {
    multiboot /boot/zero
    boot
}
EOF

    return $?
}

conf()
{
    echo "configuring GRUB in $GRUB_SRC_DIR"
    cd $GRUB_SRC_DIR
    if [ ! -f config.h ]; then
	./autogen.sh
        autoreconf -i
    fi
    ./configure --prefix="$GRUB_BUILD_DIR" --target=i686
    cd $KERN_BUILD_DIR

    return $?
}

build()
{
    echo "building GRUB in $GRUB_SRC_DIR"
    cd $GRUB_SRC_DIR
    make -j 5
    cd $KERN_BUILD_DIR

    return $?
}

fetch()
{
    echo "fetching GRUB from $GRUB_GIT_URI"
    git clone $GRUB_GIT_URI && conf && build

    return $?
}

update()
{
    echo "updating GRUB in $GRUB_SRC_DIR"
    cd $GRUB_SRC_DIR
    git pull && build
    cd $KERN_BUILD_DIR

    return $?
}

install()
{
    curdir=`pwd`
    echo "installing GRUB"
    cd $GRUB_SRC_DIR
    cd grub
    make install
    cd $KERN_BUILD_DIR

    return $?
}

mkgrub()
{
    mkcfg
    if [ ! -d grub ] || [ -d $GRUB_BUILD_DIR ]; then
	fetch && install
    else
	update && install
    fi

    return $?
}

mkgrub $@

