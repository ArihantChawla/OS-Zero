#! /bin/sh

zero_add_tmpfile() {
    tmpfile="$1"

    if test -n "$tmpfile" ; then
        if test -z "$zero_tmpfiles" ; then
            zero_tmpfiles="$tmpfile"
        else
            zero_tmpfiles="$zero_tmpfiles $tmpfile"
        fi
    fi

    return 0;
}

zero_add_tmpfiles() {
    tmpfiles="$@"

    if test -n "$tmpfiles" ; then
        for tmpfile in $tmpfiles
        do
            zero_add_tmpfile "$tmpfile"
        done
    fi

    return 0;
}

zero_remove_tmpfile() {
    file="$1"
    tmpfiles=

    if test -n "$zero_tmpfiles" ; then
        for tmpfile in $zero_tmpfiles
        do
            if test "$tmpfile" = "$file" ; then
                if test -d "$tmpfile" ; then
                    rm -r "$tmpfile"
                else
                    rm "$tmpfile"
                fi
            else
                if test -z "$tmpfiles" ; then
                    tmpfiles="$tmpfile"
                else
                    tmpfiles="$tmpfiles $tmpfile"
                fi
            fi
        done
        zero_tmpfiles="$tmpfiles"
    fi

    return 0;
}

zero_remove_tmpfiles() {
    tmpfiles="$@"

    if test -n "$zero_tmpfiles" ; then
        for tmpfile in $tmpfiles
        do
            zero_remove_tmpfile "$tmpfile"
        done
    fi

    return 0;
}

zero_clean_tmpfiles() {
    if test -n "$zero_tmpfiles" ; then
        for tmpfile in $zero_tmpfiles
        do
            if test -d "$tmpfile" ; then
                rm -rf "$tmpfile"
            else
                rm -f "$tmpfile"
            fi
        done
        $zero_tmpfiles=
    fi

    return 0;
}

