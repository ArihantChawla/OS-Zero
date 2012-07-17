#
# x86.m4 - GNU Autoconf x86 architecture configuration for Lightning.
# Copyright (C) 2004 Tuomo Venäläinen
#
# See the file COPYING for information about using this software.

AC_DEFUN([LIGHTNING_SET_X86_CPU_FLAGS],
[
    case $CC in
        gcc | tcc)
            CPU_CFLAGS="-mcpu=$host_cpu -falign-functions=32 -falign-loops=32"
            ;;
        *)
            ;;
    esac
    case $host_cpu in
        i586)
            LIGHTNING_SET_OPTIMIZE_FLAGS_586
            ;;
        i686)
            LIGHTNING_SET_OPTIMIZE_FLAGS_586
            LIGHTNING_SET_OPTIMIZE_FLAGS_686
            ;;
    esac
])

AC_DEFUN([LIGHTNING_SET_X86_CPU_FUNCS],
[
lightning_x86_cpuid_function='static void
cpuid(int op, int *eax, int *ebx, int *ecx, int *edx)
{
    __asm__("cpuid"
            : "=a" (*eax),
              "=b" (*ebx),
              "=c" (*ecx),
              "=d" (*edx)
            : "0" (op));

    return;
}'
lightning_x86_vendorid_function='static void
vendorid(char *deststr)
{
    int eax, ebx, ecx, edx;

    cpuid(X86_CPUID_COMMAND, &eax, &ebx, &ecx, &edx);
    *((int *)&deststr[[0]]) = ebx;
    *((int *)&deststr[[8]]) = ecx;
    *((int *)&deststr[[4]]) = edx;

    return;
}'
])

AC_DEFUN([LIGHTNING_CHECK_X86_CPU_VENDOR],
[
    if test "$lightning_build_local" = "yes" ; then
        AC_REQUIRE([LIGHTNING_SET_X86_CPU_FUNCS])
        LIGHTNING_CHECK_X86_CPU_VENDOR_STRING([GenuineIntel])
        if test "$lightning_cpu_vendor" = "unknown" ; then
            LIGHTNING_CHECK_X86_CPU_VENDOR_STRING([AuthenticAMD])
        fi
        if test "$lightning_cpu_vendor" = "unknown" ; then
            LIGHTNING_CHECK_X86_CPU_VENDOR_STRING([CyrixInstead])
        fi
        if test "$lightning_cpu_vendor" = "unknown" ; then
            LIGHTNING_CHECK_X86_CPU_VENDOR_STRING([Geode by NSC])
        fi
        if test "$lightning_cpu_vendor" = "unknown" ; then
            LIGHTNING_CHECK_X86_CPU_VENDOR_STRING([UMC UMC UMC ])
        fi
        if test "$lightning_cpu_vendor" = "unknown" ; then
            LIGHTNING_CHECK_X86_CPU_VENDOR_STRING([CentaurHauls])
        fi
        if test "$lightning_cpu_vendor" = "unknown" ; then
            LIGHTNING_CHECK_X86_CPU_VENDOR_STRING([NexGenDriven])
        fi
        if test "$lightning_cpu_vendor" = "unknown" ; then
            LIGHTNING_CHECK_X86_CPU_VENDOR_STRING([RiseRiseRise])
        fi
        if test "$lightning_cpu_vendor" = "unknown" ; then
            LIGHTNING_CHECK_X86_CPU_VENDOR_STRING([TransmetaCPU])
        fi
        if test "$lightning_cpu_vendor" = "unknown" ; then
            LIGHTNING_CHECK_X86_CPU_VENDOR_STRING([SiS SiS SiS ])
        fi
        if test "$lightning_cpu_vendor" = "unknown" ; then
            LIGHTNING_MSG_LINE([x86 CPU vendor not detected])
        else
            LIGHTNING_MSG_LINE([x86 CPU vendor string: $lightning_cpu_vendor])
        fi
    fi
])

AC_DEFUN([LIGHTNING_CHECK_X86_CPU_VENDOR_STRING],
[
    lightning_cpu_vendor=$1
    AC_RUN_IFELSE(
    [
       #include <string.h>

       #define X86_CPUID_COMMAND 0x00000000
       #define X86_INTEL_STRING  "$lightning_cpu_vendor"

       $lightning_x86_cpuid_function

       $lightning_x86_vendorid_function

       int
       main()
       {
           char idstr[[32]];

           memset(idstr, 0, sizeof(idstr));
           vendorid(idstr);

           exit(strcmp(idstr, X86_INTEL_STRING));
        }
    ],
    ,
    lightning_cpu_vendor="unknown")
])

# Set CPU-specific optimization flags for i586 CPUs.
AC_DEFUN([LIGHTNING_SET_OPTIMIZE_FLAGS_586],
[
    if test "$lightning_build_local" = "yes" ; then
        AC_REQUIRE([LIGHTNING_SET_X86_CPU_FUNCS])
        LIGHTNING_MSG(["checking for i586 extensions: "])
        AC_CACHE_VAL(lightning_cv_cpu_586_mmx,
        [
            AC_RUN_IFELSE(
            [
                #define X86_CPUID_COMMAND 0x00000001
                #define X86_MMX_FLAG      0x00800000

                $lightning_x86_cpuid_function

                int
                main()
                {
                    int eax, ebx, ecx, edx;

                    cpuid(X86_CPUID_COMMAND, &eax, &ebx, &ecx, &edx);

                    exit(!(edx & X86_MMX_FLAG));
                }
            ],
            lightning_cv_cpu_586_mmx="yes",
            lightning_cv_cpu_586_mmx="no")
        ])
        if test "$lightning_cv_cpu_586_mmx" = "yes" ; then
            LIGHTNING_MSG_LINE([mmx])
            AC_DEFINE([HAVE_MMX], 1)
            LIGHTNING_SET_X86_MMX_FLAGS
        else
            LIGHTNING_MSG_LINE
        fi
    elif test "$lightning_cpu_mmx" ; then
        LIGHTNING_SET_X86_MMX_FLAGS
    fi
])

# Set CPU-specific optimization flags for i686 CPUs.
AC_DEFUN([LIGHTNING_SET_OPTIMIZE_FLAGS_686],
[
    if test "$lightning_build_local" = "yes" ; then
        AC_REQUIRE([LIGHTNING_SET_X86_CPU_FUNCS])
        LIGHTNING_MSG(["checking for i686 extensions: "])
        AC_CACHE_VAL(lightning_cv_cpu_686_sse,
        [
            AC_RUN_IFELSE(
            [
                #define X86_CPUID_COMMAND 0x00000001
                #define X86_SSE_FLAG      0x02000000

                $lightning_x86_cpuid_function

                int
                main()
                {
                    int eax, ebx, ecx, edx;

                    cpuid(X86_CPUID_COMMAND, &eax, &ebx, &ecx, &edx);

                    exit(!(edx & X86_SSE_FLAG));
                }
            ],
            lightning_cv_cpu_686_sse="yes",
            lightning_cv_cpu_686_sse="no")
        ])
        if test "$lightning_cv_cpu_686_sse" = "yes" ; then
            LIGHTNING_MSG([sse])
            AC_DEFINE([HAVE_SSE], 1)
            LIGHTNING_SET_X86_SSE_OPTIMIZE_FLAGS
        fi

        AC_CACHE_VAL(lightning_cv_cpu_686_sse2,
        [
            AC_RUN_IFELSE(
            [
                #define X86_CPUID_COMMAND 0x00000001
                #define X86_SSE2_FLAG     0x04000000

                $lightning_x86_cpuid_function

                int
                main()
                {
                    int eax, ebx, ecx, edx;

                    cpuid(X86_CPUID_COMMAND, &eax, &ebx, &ecx, &edx);

                    exit(!(edx & X86_SSE2_FLAG));
                }
            ],
            lightning_cv_cpu_686_sse2="yes",
            lightning_cv_cpu_686_sse2="no")
        ])

        if test "$lightning_cv_cpu_686_sse2" = "yes" ; then
            LIGHTNING_MSG_LINE([" sse2"])
            AC_DEFINE([HAVE_SSE2], 1)
            LIGHTNING_SET_X86_SSE2_FLAGS         
        elif test "$lightning_cv_cpu_686_sse" = "yes" ; then
            LIGHTNING_MSG_LINE
            LIGHTNING_SET_X86_SSE_FLAGS
        else
            LIGHTNING_MSG_LINE
        fi
    else
        if test "$lightning_cpu_sse2" = "yes" ; then
            LIGHTNING_SET_X86_SSE_OPTIMIZE_FLAGS
            LIGHTNING_SET_X86_SSE2_FLAGS
        elif test "$lightning_cpu_sse" = "yes" ; then
            LIGHTNING_SET_X86_SSE_OPTIMIZE_FLAGS
            LIGHTNING_SET_X86_SSE_FLAGS
        fi
    fi
])

AC_DEFUN([LIGHTNING_SET_X86_MMX_FLAGS],
[
    case $CC in
        gcc | tcc)
            LIGHTNING_SET_GCC_X86_MMX_FLAGS
            ;;
        *)
            ;;
    esac
])

AC_DEFUN([LIGHTNING_SET_X86_SSE_OPTIMIZE_FLAGS],
[
    case $CC in
        gcc | tcc)
            LIGHTNING_SET_GCC_X86_SSE_OPTIMIZE_FLAGS
            ;;
        *)
            ;;
    esac
])

AC_DEFUN([LIGHTNING_SET_X86_SSE_FLAGS],
[
    case $CC in
        gcc | tcc)
            LIGHTNING_SET_GCC_X86_SSE_FLAGS
            ;;
        *)
            ;;
    esac
])

AC_DEFUN([LIGHTNING_SET_X86_SSE2_FLAGS],
[
    case $CC in
        gcc | tcc)
            LIGHTNING_SET_GCC_X86_SSE_FLAGS
            ;;
        *)
            ;;
    esac
])

