#! /bin/sh

# management of temporary files

# usage
# -----
# zero_add_tmpfile $name
#
# function
# --------
# adds name of temporary file into an internal list zero_tmpfiles
zero_add_tmpfile()
{
    tmpfile="$1"

    if [ -n "$tmpfile" ]; then
        if [ -z "$zero_tmpfiles" ]; then
            zero_tmpfiles="$tmpfile"
        else
            zero_tmpfiles="$zero_tmpfiles $tmpfile"
        fi
    fi

    return 0;
}

# usage
# -----
# zero_add_tmpfile $list
#
# function
# --------
# adds a list of files into zero_tmpfiles
zero_add_tmpfiles()
{
    tmpfiles="$@"

    if [ -n "$tmpfiles" ]; then
        for tmpfile in $tmpfiles
        do
            zero_add_tmpfile "$tmpfile"
        done
    fi

    return 0;
}

# usage
# -----
# zero_remove_tmpfile $file
#
# function
# --------
# scans zero_tmpfiles for $file and removes the file from both list and device
# if found
zero_remove_tmpfile() {
    file="$1"
    tmpfiles=

    if [ -n "$zero_tmpfiles" ]; then
        for tmpfile in $zero_tmpfiles
        do
            if [ "$tmpfile" = "$file" ]; then
                if [ -d "$tmpfile" ]; then
                    rm -r "$tmpfile"
                else
                    rm "$tmpfile"
                fi
            else
                if [ -z "$tmpfiles" ]; then
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

# usage
# -----
# zero_remove_tmpfiles
#
# function
# --------
# remove all files in zero_tmpfiles from device and nullify the list
zero_remove_tmpfiles() {
    tmpfiles="$@"

    if [ -n "$zero_tmpfiles" ]; then
        for tmpfile in $zero_tmpfiles
        do
            if [ -d "$tmpfile" ]; then
		rm -r "$tmpfile"
	    else
		rm "$tmpfile"
	    fi
        done
	zero_tmpfiles=""
    fi

    return 0;
}

# usage
# -----
# zero_remove_tmpfiles
#
# function
# --------
# forcefully remove all files in zero_tmpfiles from device and nullify the list
zero_clean_tmpfiles() {
    if [ -n "$zero_tmpfiles" ]; then
        for tmpfile in $zero_tmpfiles
        do
            if [ -d "$tmpfile" ]; then
                rm -rf "$tmpfile"
            else
                rm -f "$tmpfile"
            fi
        done
        $zero_tmpfiles=
    fi

    return 0;
}

