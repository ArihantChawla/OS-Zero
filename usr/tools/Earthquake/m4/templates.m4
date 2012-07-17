#
# templates.m4 - GNU Autoconf templates for Lightning.
# Copyright (C) 2004 Tuomo Venäläinen
#
# See the file COPYING for information about using this software.

# Define templates for macros Lightning defines.
#
AC_DEFUN([LIGHTNING_SET_TEMPLATES],
[
    # Auxiliary macros.
    AH_TEMPLATE([HAVE_AUTOCONF_H])

    # Kernel macros.
    AH_TEMPLATE([KERNEL_DEBUG])
    AH_TEMPLATE([KERNEL_PROFILE])
    AH_TEMPLATE([KERNEL_TEST])
    AH_TEMPLATE([KERNEL_USER_PROCESS])

    # Standard macros.
    AH_TEMPLATE([_POSIX_SOURCE])

    # Header macros.
    AH_TEMPLATE([__LITTLE_ENDIAN])
    AH_TEMPLATE([__BIG_ENDIAN])
    AH_TEMPLATE([__PDP_ENDIAN])
    AH_TEMPLATE([__BYTE_ORDER])

    # Miscellaneous macros.
    AH_TEMPLATE([LIGHTNING_BUILD_DATE])
    AH_TEMPLATE([LIGHTNING_AUTHORS])

    # Compiler characteristics.
    AH_TEMPLATE([HAVE_VOLATILE],
        [Define if your compiler supports volatile keyword])

    # CPU types.
    AH_TEMPLATE([CPU_ALPHA],
        [Define to non-zero if you Alpha x86 CPU])
    AH_TEMPLATE([CPU_M68K],
        [Define to non-zero if you have m68k CPU])
    AH_TEMPLATE([CPU_X86],
        [Define to non-zero if you have x86 CPU])

    # CPU features.
    AH_TEMPLATE([HAVE_MMX],
        [Define if your CPU supports MMX extensions (x86)])
    AH_TEMPLATE([HAVE_SSE],
        [Define if your CPU supports SSE extensions (x86)])
    AH_TEMPLATE([HAVE_SSE2],
        [Define if your CPU supports SSE2 extensions (x86)])
    AH_TEMPLATE([HAVE_3DNOW],
        [Define if your CPU supports 3DNOW extensions (x86)])

    # Compiler/non-standard types.
    AH_TEMPLATE([HAVE_LONG_LONG],
        [Define if you have long long type])
    AH_TEMPLATE([HAVE_UNSIGNED_LONG_LONG],
        [Define if you have unsigned long long type])
    AH_TEMPLATE([LONG_LONG],
        [Define if you have long long type])
    AH_TEMPLATE([UNSIGNED_LONG_LONG],
        [Define if you have unsigned long long type])

    # Floating-point types.
    AH_TEMPLATE([HAVE_FLOAT],
        [Define if you have float type])
    AH_TEMPLATE([HAVE_DOUBLE],
        [Define if you have double type])
    AH_TEMPLATE([HAVE_LONG_DOUBLE],
        [Define if you have long double type])
    AH_TEMPLATE([FLOAT],
        [Define to single-precision floating-point type])
    AH_TEMPLATE([DOUBLE],
        [Define to double-precision floating-point type])
    AH_TEMPLATE([LONG_DOUBLE],
        [Define if you have long double type])

    # Types with known sizes.
    AH_TEMPLATE([HAVE_LFLT32_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LFLT64_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LFLT128_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LFLT256_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LF8_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LF16_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LF32_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LF64_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LF128_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LF256_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LUINT8_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LUINT16_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LUINT32_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LUINT64_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LUINT128_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LUINT256_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LALIGN8_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LALIGN16_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LALIGN32_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LALIGN64_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LALIGN128_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([HAVE_LALIGN256_T],
        [Do NOT define this yourself])
    AH_TEMPLATE([LFLT32_T],
        [Define to signed 32-bit floating-point type])
    AH_TEMPLATE([LFLT64_T],
        [Define to signed 64-bit floating-point type])
    AH_TEMPLATE([LFLT128_T],
        [Define to signed 128-bit floating-point type])
    AH_TEMPLATE([LFLT256_T],
        [Define to signed 256-bit integer type])
    AH_TEMPLATE([LINT8_T],
        [Define to signed 8-bit integer type])
    AH_TEMPLATE([LINT16_T],
        [Define to signed 16-bit integer type])
    AH_TEMPLATE([LINT32_T],
        [Define to signed 32-bit integer type])
    AH_TEMPLATE([LINT64_T],
        [Define to signed 64-bit integer type])
    AH_TEMPLATE([LINT128_T],
        [Define to signed 128-bit integer type])
    AH_TEMPLATE([LINT256_T],
        [Define to signed 256-bit integer type])
    AH_TEMPLATE([LUINT8_T],
        [Define to unsigned 8-bit integer type])
    AH_TEMPLATE([LUINT16_T],
        [Define to unsigned 16-bit integer type])
    AH_TEMPLATE([LUINT32_T],
        [Define to unsigned 32-bit integer type])
    AH_TEMPLATE([LUINT64_T],
        [Define to unsigned 64-bit integer type])
    AH_TEMPLATE([LUINT128_T],
        [Define to unsigned 128-bit integer type])
    AH_TEMPLATE([LUINT256_T],
        [Define to unsigned 256-bit integer type])
    AH_TEMPLATE([LALIGN8_T],
        [Define to signed 8-bit type])
    AH_TEMPLATE([LALIGN16_T],
        [Define to signed 16-bit type])
    AH_TEMPLATE([LALIGN32_T],
        [Define to signed 32-bit type])
    AH_TEMPLATE([LALIGN64_T],
        [Define to signed 64-bit type])
    AH_TEMPLATE([LALIGN128_T],
        [Define to signed 128-bit type])
    AH_TEMPLATE([LALIGN256_T],
        [Define to signed 256-bit type])

    AH_TEMPLATE([LFLOAT32_T],
        [Define to 32-bit floating point type])
    AH_TEMPLATE([LFLOAT64_T],
        [Define to 64-bit floating point type])
    AH_TEMPLATE([LFLOAT128_T],
        [Define to 128-bit floating point type])

    # Other types.
    AH_TEMPLATE([HAVE_PTRDIFF_T],
        [Define if you have ptrdiff_t type])
    AH_TEMPLATE([HAVE_SIG_ATOMIC_T],
        [Define if you have sig_atomic_t type])
    AH_TEMPLATE([HAVE_WORD_T],
        [Define if you have word_t type])
    AH_TEMPLATE([HAVE_UWORD_T],
        [Define if you have uword_t type])
    AH_TEMPLATE([HAVE_PTRINT_T],
        [Define if you have ptrint_t type])
    AH_TEMPLATE([HAVE_PTRUINT_T],
        [Define if you have ptruint_t type])
    AH_TEMPLATE([PTRDIFF_T],
        [Define to pointer difference type])
    AH_TEMPLATE([SIG_ATOMIC_T],
        [Define to type that is atomic to manipulate])
    AH_TEMPLATE([WORD_T],
        [Define to signed machine word type])
    AH_TEMPLATE([UWORD_T],
        [Define to unsigned machine word type])
    AH_TEMPLATE([PTRINT_T],
        [Define to signed pointer-size type])
    AH_TEMPLATE([PTRUINT_T],
        [Define to unsigned pointer-size type])
])

