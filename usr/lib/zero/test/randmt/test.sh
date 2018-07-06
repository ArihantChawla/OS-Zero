#! /bin/sh

cwd=`pwd`
retval=0

mkdir _test
gcc -g -O -o _test/ref32 mt19937ar.c
gcc -g -O -o _test/ref64 mt19937-64.c
gcc -DRANDMT32TEST=1 -g -Wall -O -I../../.. -o _test/mt32 ../../randmt32.c
gcc -DRANDMT64TEST=1 -g -Wall -O -I../../.. -o _test/mt64 ../../randmt64.c

cd _test
./ref32 > ref32.log
./mt32 > mt32.log
./ref64 > ref64.log
./mt64 > mt64.log

echo -n "testing randmt32: "
diff -u ref32.log mt32.log
if test "$?" -eq 0; then
    echo "PASS"
else
    echo "FAIL"
    retval=`expr $retval + 1`
fi

echo -n "testing randmt64: "
diff -u ref64.log mt64.log
if test "$?" -eq 0; then
    echo "PASS"
else
    echo "FAIL"
    retval=`expr $retval + 1`
fi

cd "$cwd"
rm -r _test

exit $retval

