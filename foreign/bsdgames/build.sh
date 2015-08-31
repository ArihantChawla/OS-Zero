#! /bin/sh

pwd=`pwd`
sys=`uname`
if test "$sys" = "Linux" ; then
    if test -d bsd-games ; then
	cd bsd-games
	git pull
    else
	git clone https://github.com/rebroad/bsd-games
	cd bsd-games
    fi
    sudo mkdir -p /usr/local/share/dict/words
    sudo chmod u+w boggle/boggle/extern.h
    sudo patch -p0 < ../bsdgames.diff
    ./configure && make
else
    if test -d BSDGames ; then
	cd BSDGames
	git pull
    else
	git clone https://github.com/vattam/BSDGames.git
        cd BSDGames
    fi
    make -f Makefile.bsd
fi
cd "$pwd"

