#! /bin/sh

./colltbl > iso14651t1.tbl
#./utf8test > utf8.tbl
./ctypetbl ascii > ascii.tbl
./ctypetbl 8859-1 > iso1.tbl
./ctypetbl 8859-2 > iso2.tbl
./ctypetbl 8859-5 > iso5.tbl
./ctypetbl 8859-6 > iso6.tbl
./ctypetbl 8859-7 > iso7.tbl
./ctypetbl 8859-8 > iso8.tbl
./ctypetbl 8859-9 > iso9.tbl
./ctypetbl 8859-10 > iso10.tbl
./ctypetbl 8859-13 > iso13.tbl
./ctypetbl 8859-14 > iso14.tbl
./ctypetbl 8859-15 > iso15.tbl
./ctypetbl tis620 > tis620.tbl
./ctypetbl koi8-r > koi8r.tbl
./ctypetbl koi8-t > koi8t.tbl
./ctypetbl koi8-u > koi8u.tbl

