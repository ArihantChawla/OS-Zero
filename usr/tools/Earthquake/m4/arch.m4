#
# arch.m4 - GNU Autoconf architecture configuration for Lightning.
# Copyright (C) 2004 Tuomo Venäläinen
#
# See the file COPYING for information about using this software.

# Shell variables
# ---------------
#
# lightning_cpu_arch			configuration architecture.
#

# TODO: this is VERY early. I'm basically just recognizing some CPU types here.
# There's currently no support for CPUs other than x86 in Lightning.
AC_DEFUN([LIGHTNING_SET_CPU_ARCH],
[
    if test "$lightning_cpu_arch" = "check" ; then
        AC_MSG_CHECKING([for CPU type])
        AC_MSG_RESULT([$host_cpu])
        AC_MSG_CHECKING([for architecture type])
        case $host_cpu in
            alpha)
                LIGHTNING_SET_ARCH([alpha])
                ;;
            i386)
                LIGHTNING_SET_ARCH([x86])
                ;;
            i486)
                LIGHTNING_SET_ARCH([x86])
                ;;
            i586)
                LIGHTNING_SET_ARCH([x86])
                ;;
            i686)
                LIGHTNING_SET_ARCH([x86])
                ;;
            m68k)
                LIGHTNING_SET_ARCH([m68k])
                ;;
            m88k)
                LIGHTNING_SET_ARCH([m88k])
                ;;
            mips)
                LIGHTNING_SET_ARCH([mips])
                ;;
            i370)
                LIGHTNING_SET_ARCH([i370])
                ;;
            vax)
                LIGHTNING_SET_ARCH([vax])
                ;;
            rs6000)
                LIGHTNING_SET_ARCH([rs6000])
                ;;
            *)
                LIGHTNING_SET_ARCH([unknown])
                ;;
        esac
        if test "$lightning_cpu_arch" = "unknown" ; then
            AC_MSG_WARN([failed to detect architecture type])
        fi
    else
        LIGHTNING_MSG([setting architecture type to ])
        LIGHTNING_SET_ARCH([$lightning_cpu_arch])
    fi
    LIGHTNING_SET_ARCH_FLAGS
    LIGHTNING_SET_ARCH_DEFINES
    LIGHTNING_CHECK_CPU_VENDOR
])

AC_DEFUN([LIGHTNING_SET_ARCH],
[
    lightning_cpu_arch="$1"
    case $lightning_cpu_arch in
        alpha)
            AC_DEFINE([CPU_ALPHA])
            ;;
        m68k)
            AC_DEFINE([CPU_M68K])
            ;;
        x86)
            AC_DEFINE([CPU_X86])
            ;;
        *)
            ;;
    esac
    AC_MSG_RESULT([$lightning_cpu_arch])
    CPU_ARCHITECTURE="$lightning_cpu_arch"
    AC_SUBST([CPU_ARCHITECTURE])
    LIGHTNING_PARSE_CPU_OPTS
])

AC_DEFUN([LIGHTNING_SET_ARCH_FLAGS],
[
    if test "$lightning_cpu_arch" = "x86" ; then
        LIGHTNING_SET_X86_CPU_FLAGS
    fi
])

AC_DEFUN([LIGHTNING_CHECK_CPU_VENDOR],
[
    if test "$lightning_cpu_arch" = "x86" ; then
        LIGHTNING_CHECK_X86_CPU_VENDOR
    fi
])

