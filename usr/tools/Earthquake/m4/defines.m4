#
# defines.h - C macros for Lightning.
# Copyright (C) 2004 Tuomo Venäläinen
#
# See the file COPYING for informationa about using this software.

AC_DEFUN([LIGHTNING_SET_DEFINES],
[
    LIGHTNING_SET_AUX_DEFINES
    LIGHTNING_SET_KERNEL_DEFINES
    LIGHTNING_SET_ARCH_DEFINES
    LIGHTNING_SET_SOURCE_DEFINES
])

# Set C macros for autoconf.h.

AC_DEFUN([LIGHTNING_SET_AUX_DEFINES],
[
    AC_DEFINE([HAVE_AUTOCONF_H], 1)
])

AC_DEFUN([LIGHTNING_SET_KERNEL_DEFINES],
[
    if test "$lightning_kernel_debug" = "yes" ; then
        AC_DEFINE([KERNEL_DEBUG], 1)
    fi
    if test "$lightning_kernel_profile" = "yes" ; then
        AC_DEFINE([KERNEL_PROFILE], 1)
    fi
    if test "$lightning_kernel_test" = "yes" ; then
        AC_DEFINE([KERNEL_TEST], 1)
    fi
    if test "$lightning_kernel_user" = "yes" ; then
        AC_DEFINE([KERNEL_USER_PROCESS], 1)
    fi
])

AC_DEFUN([LIGHTNING_SET_ARCH_DEFINES],
[
    AC_DEFINE_UNQUOTED([__LITTLE_ENDIAN], 1234)
    AC_DEFINE_UNQUOTED([__BIG_ENDIAN], 4321)
    AC_DEFINE_UNQUOTED([__PDP_ENDIAN], 3412)

    case $lightning_cpu_arch in
        x86)
            AC_DEFINE_UNQUOTED([__BYTE_ORDER], __LITTLE_ENDIAN)
            ;;
        *)
            ;;
    esac
])

AC_DEFUN([LIGHTNING_SET_SOURCE_DEFINES],
[
    if test "$lightning_build_posix" ; then
        AC_DEFINE([_POSIX_SOURCE], 1)
    fi
])

