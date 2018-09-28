#! /bin/sh

build()
{
    echo "Configuring GRUB"
    if !test -f config.h
       ./autogen.sh
    fi
    ./configure --prefix="$topdir/build" --target=i686 || exit 1
    echo "Building GRUB"
    make -j 5 || exit 1
    echo "Installing GRUB"
    make install || exit 1

    exit 0
}

topdir=`pwd`
if test -d grub ; then
    echo "Updating GRUB"
    cd grub
    git pull && build
else
    echo "Fetching GRUB"
    git clone git://git.savannah.gnu.org/grub.git && cd grub && build
fi

