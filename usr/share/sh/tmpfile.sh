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
    file="$1"

    if [ -n "$file" ]; then
        if [ -z "$zero_tmpfiles" ]; then
            zero_tmpfiles="$file"
        else
            zero_tmpfiles="$zero_tmpfiles $file"
        fi
    fi

    return 0
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
    list="$@"
    file=""

    if [ -n "$tmpfiles" ]; then
        for file in $list
        do
            zero_add_tmpfile "$file"
        done
    fi

    return 0
}

# usage
# -----
# zero_clean_tmpfile $file
#
# function
# --------
# scans zero_tmpfiles for $file and cleans the file from both list and device
# if found
zero_clean_tmpfile() {
    file="$1"
    list=

    if [ -n "$zero_tmpfiles" ]; then
        for file in $zero_tmpfiles
        do
            if [ "$file" = "$arg" ]; then
                if [ -d "$tmpfile" ]; then
                    rm -r "$file"
                else
                    rm "$file"
                fi
            else
                if [ -z "$list" ]; then
                    list="$file"
                else
                    list="$list $file"
                fi
            fi
        done
        zero_tmpfiles="$list"
    fi

    return 0
}

# usage
# -----
# zero_clean_tmpfiles
#
# function
# --------
# clean all files in zero_tmpfiles from device and nullify the list
zero_clean_tmpfiles() {
    list="$@"
    file=""

    if [ -n "$zero_tmpfiles" ]; then
        for tmpfile in $zero_tmpfiles
        do
            if [ -d "$file" ]; then
		rm -r "$file"
	    else
		rm "$file"
	    fi
        done
	zero_tmpfiles=""
    fi

    return 0
}

# usage
# -----
# zero_wipe_tmpfiles
#
# function
# --------
# forcefully remove all files in zero_tmpfiles from device and nullify the list
zero_wipe_tmpfiles() {
    if [ -n "$zero_tmpfiles" ]; then
        for tmpfile in $zero_tmpfiles
        do
            if [ -d "$file" ]; then
                rm -rf "$file"
            else
                rm -f "$file"
            fi
        done
        $zero_tmpfiles=
    fi

    return 0
}

