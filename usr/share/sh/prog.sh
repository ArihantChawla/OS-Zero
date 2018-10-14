#! /bin/sh

#Shell commands and programs:
#    zero_awk		- AWK interpreter.
#    zero_basename	- get file base name.
#    zero_cc		- C compiler.
#    zero_cpp		- C preprocessor.
#    zero_ld		- linker.
#    zero_expr		- expr.
#    zero_install	- install program.
#    zero_symlink	- command to create symbolic links.
#    zero_mkpath	- command to create directories.
#    zero_sed		- sed.
#    zero_uname		- uname.
#    zero_unset		- unset variable

zero_prog_awk() {
    zero_awk="false"
    save_IFS=

    for prog in gawk mawk nawk awk
    do
        if test -n "$AWK" ; then
            zero_awk="$AWK"
        else
            save_IFS="$IFS"
            IFS="$PATH_SEPARATOR"
            for dir in $PATH
            do
                IFS="$save_IFS"
                test -z "$dir" && dir="."
                for ext in "" $zero_exec_exts
                do
                    if test -f "$dir/$prog$ext" ; then
                        zero_awk="$dir/$prog$ext"
                        break 2
                    fi
                done
            done
        fi
    done

    return 0;
}

zero_prog_basename() {
    zero_basename="false"

    if (basename /) > /dev/null 2>&1 && test "x`basename / 2>&1`" = "x/" ; then
        zero_basename="`which basename`"
    fi

    return 0;
}

#
# <FIXME>
# - add checks to make sure CC actually works.
# </FIXME>
#
zero_prog_cc() {
    zero_cc="false"
    cc_supports_ansi=
    cc_opts_ansi=

    progname="$TMPDIR/$$"
    progfiles="$progname $progname.c $progname.o $progname.rel"
    status=
    checkcmd=

    zero_add_tmpfiles "$progfiles"

    # Check for compiler.
    case $CC_FOR_BUILD,$HOST_CC,$CC in
        ,,)
            echo "int x;" > $progname.c
            for cc in gcc cc clang c89 c99 cl
            do
                if ($cc -c -o $progname.o $progname.c) > /dev/null 2>&1 ; then
                    zero_cc=`which $cc`
                    cc_opts_o="-c -o"
                    CC="$zero_cc"
                    CC_FOR_BUILD="$CC"
                    HOST_CC="$CC"
                    break
                elif ($cc -o $progname.o $progname.c) > /dev/null 2>&1 ; then
                    zero_cc=`which $cc`
                    cc_opts_o="-o"
                    CC="$zero_cc"
                    CC_FOR_BUILD="$zero_cc"
                    HOST_CC="$zero_cc"
                    break
                fi
            done
            rm -f "$progfiles"
            ;;
        ,,*)
            zero_cc="$CC"
            CC_FOR_BUILD="$zero_cc"
            HOST_CC="$zero_cc"
            ;;
        ,*,*)
            zero_cc="$HOSTCC"
            CC="$zero_cc"
            CC_FOR_BUILD="$zero_cc"
            ;;
    esac

    if test -z "$CC" ; then

        return 1
    fi

    # Check for compiler flag to accept ANSI C.
    cat > $progname.c <<EOF
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
/* Most of the following tests are stolen from RCS 5.7's src/conf.zero.  */
int argc;
char **argv;

struct buf { int x; };
struct s1 { int (*f) (int a); };
struct s2 { int (*f) (double a); };

FILE * (*rcsopen)(struct buf *, struct stat *, int);
int test(int i, double x);
int pairnames(int, char **, FILE *(*)(struct buf *, struct stat *, int), int, int);

static char *e(p, i)
    char **p;
    int i;
{
    return p[i];
}

static char *f(char * (*g) (char **, int), char **p, ...)
{
    char *s;
    va_list v;
    va_start(v,p);
    s = g(p, va_arg(v,int));
    va_end(v);

    return s;
}

int
main ()
{
    return (f(e, argv, 0) != argv[0] || f(e, argv, 1) != argv[1]);
    ;

    return 0;
}

EOF

    # Don't try gcc -ansi; that turns off useful extensions and
    # breaks some systems' header files.
    # AIX                   -qlanglvl=ansi
    # Ultrix and OSF/1      -std1
    # HP-UX 10.20 and later -Ae
    # HP-UX older versions  -Aa -D_HPUX_SOURCE
    # SVR4                  -Xc -D__EXTENSIONS__
    for opts in "" -qlanglvl=ansi -std1 -Ae \
        "-Aa -D_HPUX_SOURCE" "-Xc -D__EXTENSIONS__"
    do
        rm -f "$progname"
        if {
            ($CC $opts $cc_opts_o $progname $progname.c) 2> /dev/null
            status="$?"
            (exit $status);
        } && {
            checkcmd="test -s $progname"
            {
                (eval $checkcmd) 2> /dev/null
                status="$?"
                (exit $status);
            }
        } ; then
            cc_supports_ansi="yes"
            cc_opts_ansi="$opts"
            break
        fi
    done

    zero_remove_tmpfiles "$progfiles"

    return 0;
}

# TODO: suggest additional cpp paths to the GNU Autoconf team. :)
zero_prog_cpp() {
    zero_cpp="false"

    cppsuccess=
    cppfailure=
    srcfile="$TMPDIR/$$.c"
    status=

    zero_add_tmpfile "$srcfile"

    # CPP may name a directory on some systems (Suns).
    if test -n "$CPP" && test -d "$CPP" ; then
        CPP=
    fi

    if test -z "$CPP" ; then
        for cpp in "$CC -E" "$CC -E -traditional-cpp" "cpp" "/lib/cpp" \
            "/lib/pcpp" "/usr/bin/cpp" "/usr/lib/cpp" "/usr/libexec/cpp" \
            "/usr/ccs/lib/cpp" "/usr/X11R6/bin/cpp"
        do
            cppsuccess="no"
            cppfailure="no"

            # Use a header file that comes with gcc, so configuring glibc
            # with a frezero cross-compiler works.
            # Prefer <limits.h> to <assert.h> if __STDC__ is defined, since
            # <limits.h> exists even on freestanding compilers.
            cat > $srcfile <<EOF
#line 666 "zero"
#if defined(__STDC__)
#   include <limits.h>
#else
#   include <assert.h>
#endif
    Syntax error
EOF
            if {
                ($cpp $CPPFLAGS $srcfile) > /dev/null 2>&1
                status="$?"
                (exit $status)
            } && cppsuccess="yes" ; then
                :
            fi

            if {
                cat > $srcfile <<EOF
#include <zero_dummy.h>
EOF
                ($cpp $CPPFLAGS $srcfile) > /dev/null 2>&1
                status="$?"
                (exit $status)
            } || cppfailure="yes" ; then
                :
            fi

            rm -f $srcfile

            if test "$cppsuccess" = "yes" && test "$cppfailure" = "yes" ; then
                zero_cpp="$cpp"
                CPP="$zero_cpp"
                break
            fi
        done
    else
        zero_cpp="$CPP"
    fi

    zero_remove_tmpfile "$srcfile"

    return 0;
}

# The QNX 4.25 `expr', in addition of preferring `0' to the empty
# string, has a funny behavior in its exit status: it's always 1
# when parentheses are used!
zero_prog_expr() {
    zero_expr="false"

    if expr a : '\(a\)' > /dev/null 2>&1 ; then
        zero_expr="`which expr`"
    fi

    return 0;
}

# Find a good install program.  We prefer a C program (faster),
# so one script is as good as another.  But avoid the broken or
# incompatible versions:
# SysV /etc/install, /usr/sbin/install
# SunOS /usr/etc/install
# IRIX /sbin/install
# AIX /bin/install
# AmigaOS /C/install, which installs bootblocks on floppy discs
# AIX 4 /usr/bin/installbsd, which doesn't work without a -g flag
# AFS /usr/afsws/bin/install, which mizeroandles nonexistent args
# SVR4 /usr/ucb/install, which tries to use the nonexistent group "staff"
# ./install, which can be erroneously created by make from ./install.zero.
#
# <FIXME>
# - look for shell scripts in some sane place. :)
# </FIXME>
zero_prog_install() {
    zero_install="false"

    local save_IFS="$IFS"
    IFS="$PATH_SEPARATOR"
    for dir in $PATH
    do
        IFS="$save_IFS"
        test -z "$dir" && dir="."
        # Account for people who put trailing slazeroes in PATH elements.
        case $dir/ in
            ./ | .// | /cC/* \
            | /etc/* | /usr/sbin/* | /usr/etc/* | /sbin/* | /usr/sfsws/bin/* \
            | /usr/ucb/*)
                ;;
            *)
            # OSF1 and SCO ODT 3.0 have their own names for install.
            # Don't use installbsd from OSF since it installs stuff as root
            # by default.
                for prog in ginstall scoinst install
                do
                    for ext in "" $zero_exec_exts
                    do
                        if test -f "$dir/$prog$ext" > /dev/null 2>&1 ; then
                            if test "$prog" = "install" \
                               && grep dspmsg "$dir/$prog$ext" \
                                  >& /dev/null 2>&1 ; then
                                # AIX install. Incompatible calling convention.
                                :
                            elif test "$prog" = "install" \
                                 && grep pwplus "$dir/$prog$ext" \
                                    > /dev/null 2>&1 ; then
                                # program-specific install script used by
                                # HP pwplus - don't use.
                                :
                            else
                                zero_install="$dir/$prog$ext"
                                break 3
                            fi
                        fi
                    done
                done
                ;;
        esac
    done

    if test -z "${zero_install+set}" ; then
        if test -f "$zero_pkgdatadir/config_aux/install-zero" ; then
            zero_install="$zero_pkgdatadir/config_aux/install-zero -c"
        elif test -f "$zero_pkgdatadir/config_aux/install.zero" ; then
            zero_install="$zero_pkgdatadir/config_aux/install.zero -c"
        elif test -f "$zero_pkgdatadir/config_aux/zerotool" ; then
            zero_install="$zero_pkgdatadir/config_aux/zerotool install -c"
        fi
    fi

    return 0;
}

zero_prog_ld() {
    zero_ld="false"

    prog="ld"
    save_IFS=

    # First see if (g)cc -print-prog-name=ld works.
    case $hosttype in
        *-*-mingw*)
            # gcc leaves a trailing carriage return, which upsets mingw.
            prog=`($CC -print-prog-name=ld) 2> /dev/null | tr -d '\015'`
            ;;
        *)
            prog=`($CC -print-prog-name=ld)` 2> /dev/null
            ;;
    esac

    case $prog in
        # Accept absolute paths.
        "")
            prog="ld"
            ;;
        *)
            test -z "$LD" && LD="`which $prog`"
            ;;
    esac

    if test -z "$LD" ; then
        save_IFS="$IFS"
        IFS="$PATH_SEPARATOR"
        for dir in $PATH
        do
            IFS="$save_IFS"
            test -z "$dir" && dir="."
            for ext in "" $zero_exec_exts
            do
                if test -f "$dir/$prog$ext" ; then
                    zero_ld="$dir/$prog$ext"
                    echo "$zero_ld"
                    break 2
                fi
            done
        done
    else
        zero_ld="$LD"
    fi

    return 0;
}

zero_prog_ln_s() {
    zero_symlink="false"

    tmpfiles="$TMPDIR/$$.tmp $TMPDIR/$$"

    zero_add_tmpfiles "$tmpfiles"

    #
    # <FIXME>
    # - fix this for DJGPP; see Autoconf configure scripts.
    # </FIXME>
    #
    echo > $TMPDIR/$$.tmp
    if ln -s $TMPDIR/$$.tmp $TMPDIR/$$ 2> /dev/null ; then
        zero_symlink="`which ln` -s"
    else
        zero_symlink="`which cp` -p"
    fi

    zero_remove_tmpfiles "$tmpfiles"

    return 0;
}

zero_prog_mkdir_p() {
    zero_mkpath="false"

    if mkdir -p . 2> /dev/null ; then
        zero_mkpath="`which mkdir` -p"
    fi

    return 0;
}

zero_prog_sed() {
    zero_sed="false"

    local save_IFS="$IFS"
    list=
    tmpdir=
    max=
    count=

    IFS="$PATH_SEPARATOR"
    for dir in $PATH
    do
        IFS="$save_IFS"
        test -z $dir && dir="."
        for prog in sed gsed
        do
            for ext in "" $zero_exec_exts
            do
                if test -f "$dir/$prog$ext" ; then
                    list="$list $dir/$prog$ext"
                fi
            done
        done
    done

    {
        tmpdir=`(umask 077 && mktemp -d -q "$TMPDIR/sedXXXXXX") \
                 2> /dev/null` \
                 && test -n "$tmpdir" && test -d "$tmpdir"
    } || {
        tmpdir="$TMPDIR/sed$$-$RANDOM"
        (umask 077 && mkdir $tmpdir)
    } || {

        return 1
    }
    zero_add_tmpfile "$tmpdir"

    max=0
    count=0
    # /usr/xpg4/bin/sed is typically found on Solaris along with /bin/sed which
    # truncates output.
    for prog in $list
    do
        test ! -f "$prog" && break
        cat /dev/null > "$tmpdir/sed.in"
        count=0
        echo "$zero_digits" > "$tmpdir/sed.in"

        # Check for GNU sed and select it if it's found.
        if "$prog" --version 2>&1 < /dev/null | grep "GNU" > /dev/null ; then
            zero_sed=`which $prog`
            break
        fi

        while true
        do
            cat "$tmpdir/sed.in" "$tmpdir/sed.in" > "$tmpdir/sed.tmp"
            mv "$tmpdir/sed.tmp" "$tmpdir/sed.in"
            cp "$tmpdir/sed.in" "$tmpdir/sed.nl"
            echo >> "$tmpdir/sed.nl"
            $prog -e 's/a$//' < "$tmpdir/sed.nl" > "$tmpdir/sed.out" || break
            cmp -s "$tmpdir/sed.out" "$tmpdir/sed.nl" || break

            test "$count" -gt 8 && break
            count=`$zero_expr $count + 1`
            if test "$count" -gt "$max" ; then
                max="$count"
                zero_sed=`which $prog`
            fi
        done
    done

    zero_remove_tmpfile "$tmpdir"

    return 0;
}

zero_prog_uname() {
    # Hack for Pyramid OS/BSD.
    if (test -f /.attbin/uname) > /dev/null 2>&1 ; then
        zero_uname="/.attbin/uname"
    else
        zero_uname=`which uname`
    fi

    return 0;
}

