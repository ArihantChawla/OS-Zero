#
# gcc.m4 - GNU Autoconf GCC configuration for Lightning.
# Copyright (C) 2004 Tuomo Venäläinen
#
# See the file COPYING for information about using this software.

AC_DEFUN([LIGHTNING_SET_GCC_X86_MMX_FLAGS],
[
    CPU_CFLAGS="$CPU_CFLAGS -mmmx"
])

AC_DEFUN([LIGHTNING_SET_GCC_X86_SSE_OPTIMIZE_FLAGS],
[
    CPU_CFLAGS="$CPU_CFLAGS -malign-double -m128bit-long-double -mfpmath=sse"
])

AC_DEFUN([LIGHTNING_SET_GCC_X86_SSE_FLAGS],
[
    CPU_CFLAGS="$CPU_CFLAGS -msse"
])

AC_DEFUN([LIGHTNING_SET_GCC_X86_SSE2_FLAGS],
[
    CPU_CFLAGS="$CPU_CFLAGS -msse2"
])

# Set compiler flags for building kernel.
AC_DEFUN([LIGHTNING_SET_GCC_KERNEL_FLAGS],
[
    KERNEL_CFLAGS="-fno-builtin -ffreestanding -finline-functions -finline-limit=1048576"
    if test "$lightning_kernel_user" != "yes" ; then
        KERNEL_CFLAGS="-nostdinc -nostdlib $KERNEL_CFLAGS"
    fi
    AC_SUBST([KERNEL_CFLAGS])
])

# Set compiler flags for building libraries.
AC_DEFUN([LIGHTNING_SET_GCC_LIB_FLAGS],
[
    LIB_CFLAGS="-fno-builtin -static"
    AC_SUBST([LIB_CFLAGS])
])

# Set compiler flags for building applications.
AC_DEFUN([LIGHTNING_SET_GCC_LIGHTNING_FLAGS],
[
    LIGHTNING_CFLAGS="-fno-builtin"
    AC_SUBST([LIGHTNING_CFLAGS])
])
AC_DEFUN([LIGHTNING_SET_GCC_WARNING_FLAGS],[
    AC_ARG_ENABLE(compiler-warnings, 
      [  --enable-compiler-warnings=[no/minimum/yes]	Turn on compiler warnings.],,enable_compiler_warnings=minimum)

    AC_MSG_CHECKING(what warning flags to pass to the C compiler)
    WARNING_CFLAGS=
    if test "x$GCC" != xyes; then
        enable_compiler_warnings=no
    fi

    if test "x$enable_compiler_warnings" != "xno"; then
        if test "x$GCC" = "xyes"; then
            case " $CFLAGS " in
                *[\ \	]-Wall[\ \	]*) ;;
                *) WARNING_CFLAGS="-Wall -Wunused" ;;
            esac

            ## -W is not all that useful.  And it cannot be controlled
            ## with individual -Wno-xxx flags, unlike -Wall
            ## Added everything after -Wmissing-declarations. -vendu
            if test "x$enable_compiler_warnings" = "xyes"; then
	        WARNING_CFLAGS="$WARNING_CFLAGS -Wmissing-prototypes -Wmissing-declarations -Wbad-function-cast -Wcast-align -Waggregate-return -Wnested-externs -Wundef -Wshadow -Winline"
            fi
        fi
    fi
    AC_MSG_RESULT($WARNING_CFLAGS)

    AC_ARG_ENABLE(iso-c,
        [  --enable-iso-c          Try to warn if code is not ISO C ],,
        enable_iso_c=no)

    AC_MSG_CHECKING(what language compliance flags to pass to the C compiler)
    ISO_CFLAGS=
    if test "x$enable_iso_c" != "xno"; then
        if test "x$GCC" = "xyes"; then
            case " $CFLAGS " in
                *[\ \	]-ansi[\ \	]*) ;;
                *) ISO_CFLAGS="$ISO_CFLAGS -ansi" ;;
            esac

            case " $CFLAGS " in
                *[\ \	]-pedantic[\ \	]*) ;;
                *) ISO_CFLAGS="$ISO_CFLAGS -pedantic" ;;
            esac
        fi
    fi
    AC_MSG_RESULT($ISO_CFLAGS)
    if test "x$cflags_set" != "xyes"; then
        CFLAGS="$CFLAGS $WARNING_CFLAGS $ISO_CFLAGS"
    fi
])

