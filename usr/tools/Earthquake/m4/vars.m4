#
# vars.m4 - GNU Autoconf variables for Lightning.
# Copyright (C) 2004 Tuomo Venäläinen
#
# See the file COPYING for information about using this software.

# Shell variables
# ---------------
#
# lightning_lowercase_letters	list of lower case letters for sed.
# lightning_uppercase_letters	list of upper case letters for sed.
# lightning_cpu_vendor		CPU vendor ID string.
#
# Substituted variables
# ---------------------
#
# LIGHTNING_LIBS		list of libraries used to build Lightning.

# Set shell variables to be used in configure.
#
AC_DEFUN([LIGHTNING_SET_VARS],
[
# Character lists for sed and friends.
lightning_lowercase_letters="abcdefghijklmnopqrstuvwxyzåäö"
lightning_uppercase_letters="ABCDEFGHIJKLMNOPQRSTUVWXYZåäö"

# CPU vendor.
lightning_cpu_vendor="unknown"
])

AC_DEFUN([LIGHTNING_SET_LIBS],
[
    # Set libraries for Lightning.
    LIGHTNING_LIBS="$LIGHTNING_LIBRARY_PATH $LIBS"
    AC_SUBST([LIGHTNING_LIBS])
])

AC_DEFUN([LIGHTNING_SET_INSTALL_DIRS],
[
    # Standard GNU installation directories.

    # Installation prefix.
    if test -z "$prefix" || test "$prefix" = "NONE" ; then
        prefix="$ac_default_prefix"
    fi

    # Installation prefix.
    if test -z "$exec_prefix" || test "$exec_prefix" = "NONE" ; then
        exec_prefix="$prefix"
    fi

    # Binary installation directory.
    if test -z "$bindir" || test "bindir" = "NONE" ; then
        bindir="$exec_prefix/bin"
    fi

    # System binary installation directory.
    if test -z "$sbindir" || test "sbindir" = "NONE" ; then
        sbindir="$exec_prefix/sbin"
    fi

    # Executable installation directory.
    if test -z "$libexecdir" || test "libexecdir" = "NONE" ; then
        libexecdir="$exec_prefix/libexec"
    fi

    # Configuration file installation directory.
    if test -z "$sysconfdir" || test "$sysconfdir" = "NONE" ; then
        sysconfdir="$prefix/etc"
    fi

    # Header installation directory.
    if test -z "$includedir" || test "$includedir" = "NONE" ; then
        includedir="$prefix/include"
    fi

    # Non-GCC header installation directory.
    if test -z "$oldincludedir" || test "$oldincludedir" = "NONE" ; then
        oldincludedir="/usr/include"
    fi

    # Object and library installation directory.
    if test -z "$libdir" || test "$libdir" = "NONE" ; then
        libdir="$exec_prefix/lib"
    fi

    # State file installation directory.
    if test -z "$sharedstatedir" || test "$sharedstatedir" = "NONE" ; then
        sharedstatedir="$prefix/com"
    fi

    # Machine-specific state file installation directory.
    if test -z "$localstatedir" || test "$localstatedir" = "NONE" ; then
        localstatedir="$prefix/var"
    fi

    # Data installation directory.
    if test -z "$datadir" || test "$datadir" = "NONE" ; then
        datadir="$prefix/share"
    fi

    # Manual page installation directory.
    if test -z "$mandir" || test "$mandir" = "NONE" ; then
        mandir="$prefix/man"
    fi

    # Info file installation directory.
    if test -z "$infodir" || test "$infodir" = "NONE" ; then
        infodir="$prefix/info"
    fi

    # Emacs lisp file installation directory.
    if test -z "$lispdir" || test "$lispdir" = "NONE" ; then
        lispdir="$prefix/share/emacs/site-lisp"
    fi

    # Package object and library installation directory.
    if test -z "$pkglibdir" || test "$pkglibdir" = "NONE" ; then
        pkglibdir="$libdir/$PACKAGE"
    fi

    # Package header installation directory.
    if test -z "$pkgincludedir" || test "$pkgincludedir" = "NONE" ; then
        pkgincludedir="$includedir/$PACKAGE"
    fi

    # Package data installation directory.
    if test -z "$pkgdatadir" || test "$pkgdatadir" = "NONE" ; then
        pkgdatadir="$datadir/$PACKAGE"
    fi

    # Lightning-specific installation directories.

    # Kernel library installation directory.
    if test -z "$kernellibdir" || test "$kernellibdir" = "NONE" ; then
        kernellibdir="$pkglibdir/kernel"
        AC_SUBST([kernellibdir])
    fi

    # sys header directory.
    if test -z "$sysincludedir" || test "$libcsysincludedir" = "NONE" ; then
        sysincludedir="$includedir/sys"
        AC_SUBST([sysincludedir])
    fi

    # C library header directory.
    if test -z "$libcincludedir" || test "$libcincludedir" = "NONE" ; then
        libcincludedir="$pkgincludedir/libc"
	AH_TEMPLATE([libcincludedir])
        AC_SUBST([libcincludedir])
    fi

    # C library ISO header directory.
    if test -z "$libcisoincludedir" \
       || test "$libcisoincludedir" = "NONE" ; then
        libcisoincludedir="$libcincludedir/iso"
        AC_SUBST([libcisoincludedir])
    fi

    # C library POSIX header directory.
    if test -z "$libcposixincludedir" \
       || test "$libcposixincludedir" = "NONE" ; then
        libcposixincludedir="$libcincludedir/posix"
        AC_SUBST([libcposixincludedir])
    fi

    # C library POSIX header directory.
    if test -z "$libcposixincludedir" \
       || test "$libcposixincludedir" = "NONE" ; then
        libcposixincludedir="$libcincludedir/posix"
        AC_SUBST([libcposixincludedir])
    fi

    # C library POSIX sys header directory.
    if test -z "$libcposixsysincludedir" \
       || test "$libcposixsysincludedir" = "NONE" ; then
        libcposixsysincludedir="$libcincludedir/posix"
        AC_SUBST([libcposixsysincludedir])
    fi

    # C library Unix header directory.
    if test -z "$libcunixincludedir" \
       || test "$libcunixincludedir" = "NONE" ; then
        libcunixincludedir="$libcincludedir/unix"
        AC_SUBST([libcunixincludedir])
    fi

    # Earthquake C Environment header directory.
    if test -z "$earthquakeincludedir" \
       || test "$earthquakeincludedir" = "NONE" ; then
        earthquakeincludedir="$libcincludedir/unix"
        AC_SUBST([earthquakeincludedir])
    fi

    # Thunder Window System header directory.
    if test -z "$thunderincludedir" \
       || test "$thunderincludedir" = "NONE" ; then
        thunderincludedir="$libcincludedir/unix"
	AH_TEMPLATE([thunderincludedir])
        AC_SUBST([thunderincludedir])
    fi

    # Shell module directory.
    if test -z "$lshdir" || test "$lshdir" = "NONE" ; then
        lshdir="$pkgdatadir/lsh"
        AC_SUBST([lshdir])
    fi

    lightning_subst_dirs="$LIGHTNING_DISTRIBUTION_ROOT/aux/substdirs.make"
    AC_SUBST_FILE([lightning_subst_dirs])
])

