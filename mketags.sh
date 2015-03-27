#! /bin/sh

#for f in `find . -type f -name *.[hcS]` ; do ctags -e -a $f ; done
#find . -type d -exec ./edirtags.sh {} \;

ctags -e -a `find . -type f -name \*.[hcS]`
for d in `find . -type d`
do
    ctags -R -e "$d" > "$d/TAGS"
done

