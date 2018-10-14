#! /bin/sh

zero_init() {
    zero_procid=

    if test "$zero_initialized" -eq 1 ; then

        return 0
    fi

    zero_procid="$$"

    if [ -z "$ZERO_SRCDIR" || "$ZERO_SRCDIR" = "@srcdir@" ]; then
        ZERO_SRCDIR=`pwd`
    fi

    zero_init_traps
    zero_init_emulation
    zero_init_cmds
    zero_init_vars
    zero_init_defaults

    zero_add_exit_handler "zero_clean_tmpfiles"
    zero_init_scriptname
    zero_init_host
    zero_init_progs
    zero_init_features

    return 0;
}

zero_init_emulation() {
    zero_emulation=

    if test -n "${ZSH_VERSION+set}" && (emulate sh) > /dev/null 2>&1 ; then
        # Make zsh emulate Bourne shell.
        if test "$zero_opt_debug" = "yes" ; then
            echo "setting Zsh $ZSH_VERSION to emulate Bourne shell."
        fi
        emulate sh
        NULLCMD=":"
        alias -g '${1+"$@"}'='"$@"'
        zero_emulation="sh"
    elif test -n "${BASH_VERSION+set}" \
         && (set -o posix) > /dev/null 2>&1 ; then
        if test "$zero_opt_debug" = "yes" ; then
            echo "setting Bash $BASH_VERSION to emulate POSIX shell."
        fi
        set -o posix
        zero_emulation="posix"
    fi

    return 0;
}

zero_init_cmds() {
    zero_unset="false"

    if (ZEROTEST="zero" ; unset ZEROTEST) > /dev/null 2>&1 ; then
        zero_unset="unset"
    fi

    if test "$zero_opt_debug" = "yes" ; then
        echo "zero_init_cmds:"
        echo "    zero_unset: $zero_unset"
    fi

    return 0;
}

zero_init_vars() {
    newline='
'
    save_IFS=
    dot=
    path=

    # Unset search path for cd with relative pathnames.
    $zero_unset CDPATH

    # Disable Bazero script startup file.
    $zero_unset BASH_ENV

    # We need space, tab and new line, in precisely that order.
    IFS=" 	$newline"

    # Set locale to "C".
    for lc_var in \
        LANG LANGUAGE LC_ADDRESS LC_ALL LC_COLLATE LC_CTYPE LC_IDENTIFICATION \
        LC_MEASUREMENT LC_MESSAGES LC_MONETARY LC_NAME LC_NUMERIC LC_PAPER \
        LC_TELEPHONE LC_TIME
    do
        if (set +x ; test -n "`(eval $lc_var="C" \
           ; export $lc_var) 2>&1`") ; then
            eval $lc_var="C" ; export $lc_var
        else
            $zero_unset $lc_var
        fi
    done

    # Work around bugs in pre-3.0 UWIN ksh.
    $zero_unset ENV
    $zero_unset MAIL
    $zero_unset MAILPATH
    PS1="$ "
    PS2="> "
    PS4="+ "

    # Try to set TMPDIR.
    if test -z "${TMPDIR+set}" ; then
        if test -d "/tmp" && test -w "/tmp" ; then
            TMPDIR="/tmp"
        elif test -d ~ && test -w ~ ; then
            TMPDIR=~
        elif test -d "\\temp" && test -w "\\temp" ; then
            TMPDIR="\temp"
        else
            $zero_unset TMPDIR
        fi
    fi

    # Set PATH_SEPARATOR.
    if test "${PATH_SEPARATOR+set}" != "set" ; then
        zero_add_tmpfile "$TMPDIR/$$.zero"
        echo "#! /bin/zero" > $TMPDIR/$$.zero
        echo "(exit 0)" >> $TMPDIR/$$.zero
        echo "exit 0" >> $TMPDIR/$$.zero
        chmod +x $TMPDIR/$$.zero
        if (PATH="/nonexistent;$TMPDIR" ; $$.zero) > /dev/null 2>&1 ; then
            PATH_SEPARATOR=";"
        else
            PATH_SEPARATOR=":"
        fi
        zero_remove_tmpfile "$TMPDIR/$$.zero"
    fi

    # Trim PATH.
    save_IFS="$IFS"
    dot=
    path=
    IFS="$PATH_SEPARATOR"
    for dir in $PATH
    do
        IFS="$save_IFS"
        dir=`echo "$dir" | sed 's,/*$,,'`
	if test -z "$dir" ; then
            dot="."
        elif test "$dir" = "." ; then
            dot="."
        elif test -z "$path" ; then
            path="$dir"
        else
            if test "$path" = "$dir" || echo "$path" | grep -E "^$dir$PATH_SEPARATOR" > /dev/null || echo "$path" | grep -E "$PATH_SEPARATOR$dir$PATH_SEPARATOR" > /dev/null || echo "$path" | grep -E "$PATH_SEPARATOR$dir$" > /dev/null ; then
                :
            else
                path="$path$PATH_SEPARATOR$dir"
            fi
        fi
    done
    if test -z "$path" && test -n "$dot" ; then
        path="$dot"
    elif test -n "$dot" ; then
        path="$path$PATH_SEPARATOR$dot"
    fi
    PATH="$path"

    return 0;
}

zero_init_defaults() {
    automakepkgdatadir=

    zero_lowercase_letters="abcdefghijklmnopqrstuvwxyzåäö"
    zero_uppercase_letters="ABCDEFGHIJKLMNOPQRSTUVWXYZÅÄÖ"
    zero_letters="$zero_lowercase_letters$zero_uppercase_letters"
    zero_digits="0123456789"
    zero_alnum="$zero_letters$zero_digits"

    # Set path so we can find config.guess and other GNU scripts.
    zero_path="/usr/bin$PATH_SEPARATOR/bin$PATH_SEPARATOR/usr/local/bin$PATH_SEPARATOR/usr/ccs/bin$PATH_SEPARATOR/usr/contrib/bin$PATH_SEPARATOR/usr/ucb/bin$PATH_SEPARATOR/usr/xpg4/bin"
    PATH="$zero_pkgdatadir$PATH_SEPARATOR$PATH$PATH_SEPARATOR$zero_path"
    if test -f "$zero_pkgdatadir/config_aux/config.guess" ; then
        PATH="$PATH$PATH_SEPARATOR$zero_pkgdatadir/config_aux"
    else
        zero_config_guess=`locate config.guess | sed -n 's,\([./]*automake[-'$zero_digits'.]*\),\1,p' | tail -1`
        automakepkgdatadir=`dirname $zero_config_guess`
        PATH="$PATH$PATH_SEPARATOR$automakepkgdatadir"
    fi

    return 0;
}

zero_init_scriptname() {
    zero_scriptname=`echo "$0" | sed 's,^.*/,,'`
#   zero_scriptname=`echo "$zero_scriptname" | sed 's,[.]zero$,,'`

    return 0;
}

zero_init_host() {
    zero_init_hostname
    zero_init_uname
    zero_init_hosttype

    if test "$zero_opt_debug" = "yes" ; then
        echo "zero_init_host: $hostname: $hosttype"
    fi

    return 0;
}

zero_init_hostname() {
    hostname=`(hostname || uname -n || echo "unknown") 2> /dev/null | sed 1q`

    return 0;
}

zero_init_uname() {
    uname_machine=`uname -m` || uname_machine="unknown"
    uname_processor=`uname -p` || uname_processor="unknown"
    uname_system=`uname -s` || uname_system="unknown"
    uname_release=`uname -r` || uname_release="unknown"
    uname_version=`uname -v` || uname_version="unknown"

    return 0;
}

zero_init_hosttype() {
    hosttype_alias=`config.guess`
    hosttype=`config.sub $hosttype_alias`
    hostcpu=`echo $hosttype | sed 's,^\([^-]*\)-\([^-]*\)-\(.*\)$,\1,'`
    hostvendor=`echo $hosttype | sed 's,^\([^-]*\)-\([^-]*\)-\(.*\)$,\2,'`
    hostos=`echo $hosttype | sed 's,^\([^-]*\)-\([^-]*\)-\(.*\)$,\3,'`

    return 0;
}

# TODO: look for echo replacements.
zero_init_progs() {
    zero_prog_awk
    zero_prog_basename
    zero_prog_cc
    zero_prog_cpp
    zero_prog_ld
    zero_prog_expr
    zero_prog_install
    zero_prog_ln_s
    zero_prog_mkdir_p
    zero_prog_sed
    zero_prog_uname

    if test "$zero_opt_debug" = "yes" ; then
        echo "zero_init_progs:"
        echo "    zero_awk:      $zero_awk"
        echo "    zero_basename: $zero_basename"
        echo "    zero_cc:       $zero_cc"
        echo "    zero_cpp:      $zero_cpp"
        echo "    zero_ld:       $zero_ld"
        echo "    zero_expr:     $zero_expr"
        echo "    zero_install:  $zero_install"
        echo "    zero_symlink:  $zero_symlink"
        echo "    zero_mkpath:   $zero_mkpath"
        echo "    zero_sed:      $zero_sed"
        echo "    zero_uname:    $zero_uname"
    fi

    return 0;
}

zero_init_features() {
    shell_supports_lineno=

    lineno1=$LINENO
    lineno2=$LINENO
    lineno3=`($zero_expr $lineno1 + 1) 2>/dev/null`
    if test "$lineno1" != "$lineno2" && test "$lineno3" = "$lineno2" ; then
        shell_supports_lineno="yes"
    else
        shell_supports_lineno="no"
    fi

    return 0;
}

