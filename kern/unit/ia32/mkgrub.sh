#! /bin/sh

topdir=`pwd`
if test -d grub ; then
    echo "Updating GRUB"
    cd grub
    git pull
else
    echo "Fetching GRUB"
    git clone git://git.savannah.gnu.org/grub.git
    cd grub
fi
echo "Configuring GRUB"
./autogen.sh
./configure --prefix="$topdir/build" --target=i686
echo "Building GRUB"
make -j 3
echo "Installing GRUB"
make install
cd "$topdir"

