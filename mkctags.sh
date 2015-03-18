#! /bin/sh

find . -type f -iname "*.[chS]" | xargs etags -a

