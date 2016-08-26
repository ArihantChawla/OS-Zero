#! /bin/sh

pwd=`pwd`
if test -d openlibm ; then
    cd openlibm
    git pull
else
    git clone https://github.com/JuliaLang/openlibm
    cd openlibm
fi
make && make test && sudo make install

cd "$pwd"

