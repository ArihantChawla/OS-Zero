#
# opts.m4 - GNU Autoconf option handling for Lightning.
# Copyright (C) 2004 Tuomo Venäläinen
#
# See the file COPYING for information about using this software.

AC_DEFUN([LIGHTNING_PARSE_OPTS],
[
    AC_ARG_WITH([arch],
        AC_HELP_STRING([--with-arch=ARCH], [set build architecture]),
        [
            lightning_cpu_arch="$withval"
        ],
        [
            lightning_cpu_arch="check"
        ]
    )

    AC_ARG_ENABLE([build-local],
        AC_HELP_STRING([--enable-build-local],
                       [build for local host default=yes]),
        [
            lightning_build_local="$enableval"
        ],
        [
            lightning_build_local="yes"
        ]
    )

    AC_ARG_ENABLE([build-posix],
        AC_HELP_STRING([--enable-build-posix],
                       [POSIX-compliancy default=yes]),
        [
            lightning_build_posix="$enableval"
        ],
        [
            lightning_build_posix="yes"
        ]
    )

    AC_ARG_ENABLE([configure-info],
        AC_HELP_STRING([--enable-configure-info],
                       [print configuration information]),
        [
            lightning_configure_info="$enableval"
        ],
        [
            lightning_configure_info="no"
        ],
    )

    AC_ARG_ENABLE([configure-trace],
        AC_HELP_STRING([--enable-configure-trace],
                       [enable configure tracing]),
        [
            lightning_configure_trace="$enableval"
        ],
        [
            lightning_configure_trace="no"
        ]
    )

    AC_ARG_ENABLE([kernel-debug],
        AC_HELP_STRING([--enable-kernel-debug],
                       [enable kernel debugging]),
        [
            lightning_kernel_debug="$enableval"
        ],
        [
            lightning_kernel_debug="no"
        ]
    )

    AC_ARG_ENABLE([kernel-profile],
        AC_HELP_STRING([--enable-kernel-profile],
                       [enable kernel profiling]),
        [
            lightning_kernel_profile="$enableval"
        ],
        [
            lightning_kernel_profile="no"
        ]
    )

    AC_ARG_ENABLE([kernel-test],
        AC_HELP_STRING([--enable-kernel-test],
                       [enable kernel testing]),
        [
            lightning_kernel_test="$enableval"
        ],
        [
            lightning_kernel_test="no"
        ]
    )

    AC_ARG_ENABLE([kernel-user],
        AC_HELP_STRING([--enable-kernel-user],
                       [test kernel as a user process]),
        [
            lightning_kernel_user="$enableval"
        ],
        [
            lightning_kernel_user="no"
        ]
    )

])

AC_DEFUN([LIGHTNING_PARSE_CPU_OPTS],
[
    case $lightning_cpu_arch in
        alpha)
            ;;
        m68k)
            ;;
        x86)
            AC_ARG_ENABLE([mmx],
                AC_HELP_STRING([--enable-mmx], [enable x86 MMX instructions]),
                [
                    lightning_cpu_mmx="yes"
                ]
            )
            AC_ARG_ENABLE([sse],
                AC_HELP_STRING([--enable-sse], [enable x86 SSE instructions]),
                [
                    lightning_cpu_sse="yes"
                ]
            )
            AC_ARG_ENABLE([sse2],
                AC_HELP_STRING([--enable-sse2], [enable x86 SSE2 instructions]),
                [
                    lightning_cpu_sse2="yes"
                ]
            )
            AC_ARG_ENABLE([3dnow],
                AC_HELP_STRING([--enable-3dnow], [enable x86 3DNOW instructions]),
                [
                    lightning_cpu_3dnow="yes"
                ]
            )
            if test "$lightning_cpu_mmx" = "yes" ; then
                LIGHTNING_MSG_LINE([enabling x86 MMX instructions])
                CPU_CFLAGS="$CPU_CFLAGS -mmmx"
                AC_DEFINE([HAVE_MMX], 1)
            fi
            if test "$lightning_cpu_sse" = "yes" ; then
                LIGHTNING_MSG_LINE([enabling x86 SSE instructions])
                CPU_CFLAGS="$CPU_CFLAGS -malign-double -m128bit-long-double -mfpmath=sse"
                AC_DEFINE([HAVE_SSE], 1)
            fi
            if test "$lightning_cpu_sse2" = "yes" ; then
                LIGHTNING_MSG_LINE([enabling x86 SSE2 instructions])
                CPU_CFLAGS="$CPU_CFLAGS -msse2"
                AC_DEFINE([HAVE_SSE2], 1)
            elif test "$lightning_cpu_sse" = "yes" ; then
                CPU_CFLAGS="$CPU_CFLAGS -msse"
            fi
            if test "$lightning_cpu_3dnow" = "yes" ; then
                LIGHTNING_MSG_LINE([enabling x86 3DNOW instructions])
                CPU_CFLAGS="$CPU_CFLAGS -m3dnow"
                AC_DEFINE([HAVE_3DNOW], 1)
            fi
            ;;
    esac
])

