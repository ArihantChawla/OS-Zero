#! /bin/sh

find . -type d -exec ./dirtags.sh {} \;
ctags --file-scope=no -R

#find . -type f -iname "*.[chS]" | xargs etags -a

