#
# types.m4 - GNU Autoconf type checks.
# Copyright (C) 2004 Tuomo Venäläinen
#
# See the file COPYING for information about using this software.

# Type checks.
AC_DEFUN([LIGHTNING_TYPES],
[
    LIGHTNING_SIZES
    LIGHTNING_TYPES
    LIGHTNING_TYPES_CPU
])

# LIGHTNING_CHECK_TYPE(TYPE [, DEFAULT, FOUND, NOT-FOUND, QUIET])
AC_DEFUN([LIGHTNING_CHECK_TYPE],
[
    type="$1"
    default="$2"
    quietsave="$quiet"
    quiet="$5"

    ac_cv_name=`echo "ac_cv_type_$type" | sed 'y, *,_p,'`
    ac_macro_name=`echo "HAVE_$type" | sed 'y,'$lightning_lowercase_letters' *,'$lightning_uppercase_letters'_P,'`
    ac_type_name=`echo "$type" | sed 'y,'$lightning_lowercase_letters' *,'$lightning_uppercase_letters'_P,'`

    AC_MSG_CHECKING([for $type])
    AC_CACHE_VAL($ac_cv_name,
    [
        AC_COMPILE_IFELSE(
        [
            AC_LANG_SOURCE(
            [
                int
                main()
                {
                    if (($type *)0) {

                        exit(0);
                    }
                    if (sizeof($type)) {

                        exit(0);
                    }

                    exit(1);
                }
            ])
        ],
        eval "$ac_cv_name=yes",
        eval "$ac_cv_name=no")
    ])
    eval "cvval=$`echo $ac_cv_name`"
    if test "$cvval" = "yes" ; then
        AC_MSG_RESULT([yes])
        AC_DEFINE_UNQUOTED([$ac_macro_name], 1)
        AC_DEFINE_UNQUOTED([$ac_type_name], [$type])
        [$3]
    else
        if test -n "$default" && test "$default" != "no" ; then
            AC_MSG_RESULT([$default])
            AC_DEFINE_UNQUOTED([$ac_type_name], [$default])
        else
            AC_MSG_RESULT([no])
        fi
        [$4]
    fi
    quiet="$quietsave"
])

# LIGHTNING_CHECK_SIZEOF(TYPE)
AC_DEFUN([LIGHTNING_CHECK_SIZEOF],
[
    type="$1"

    ac_cv_name=`echo "ac_cv_type_$type" | sed 'y, *,_p,'`
    ac_cv_sizeof=`echo "ac_cv_sizeof_$type" | sed 'y, *,_p,'`
    ac_macro_name=`echo "SIZEOF_$type" | sed 'y,'$lightning_lowercase_letters' *,'$lightning_uppercase_letters'_P,'`

    havetype="`echo $ac_cv_name`"
    if test "$havetype" != "yes" ; then
        LIGHTNING_CHECK_TYPE([$type], no, , , yes)
    fi

    AC_MSG_CHECKING([size of $type])
    AC_CACHE_VAL($ac_cv_sizeof,
    [
        AC_RUN_IFELSE(
        [
            AC_LANG_SOURCE(
            [
                int
                main()
                {
                    exit(sizeof($type));
                }
            ])
        ],
        eval "$ac_cv_sizeof=0",
        eval "$ac_cv_sizeof=$?")
    ])
    eval "cvval=$`echo $ac_cv_sizeof`"
    AC_MSG_RESULT([$cvval])
    if test "$cvval" != "0" ; then
        AC_DEFINE_UNQUOTED([$ac_macro_name], [$cvval])
    fi
])

# Check for type sizes

AC_DEFUN([LIGHTNING_SIZES],
[
    # standard integer types. */

    LIGHTNING_CHECK_SIZEOF([signed char])
    LIGHTNING_CHECK_SIZEOF([char])
    LIGHTNING_CHECK_SIZEOF([unsigned char])
    LIGHTNING_CHECK_SIZEOF([short])
    LIGHTNING_CHECK_SIZEOF([unsigned short])
    LIGHTNING_CHECK_SIZEOF([int])
    LIGHTNING_CHECK_SIZEOF([unsigned int])
    LIGHTNING_CHECK_SIZEOF([long])
    LIGHTNING_CHECK_SIZEOF([unsigned long])

    # standard pointer types. */

    LIGHTNING_CHECK_SIZEOF([char *])
    LIGHTNING_CHECK_SIZEOF([void *])

    # standard floating point types */

    LIGHTNING_CHECK_SIZEOF([float])
    LIGHTNING_CHECK_SIZEOF([double])
    LIGHTNING_CHECK_SIZEOF([long double])

    # non-standard integer types. */

    LIGHTNING_CHECK_SIZEOF([unsigned long long])
    LIGHTNING_CHECK_SIZEOF([long long])
])

AC_DEFUN([LIGHTNING_TYPES],
[
    LIGHTNING_TYPES_INTEGER
    LIGHTNING_TYPES_FLOAT
    LIGHTNING_TYPES_ALIGN
])

AC_DEFUN([LIGHTNING_TYPES_INTEGER],
[
    LIGHTNING_TYPES_INTEGER_8_BIT
    LIGHTNING_TYPES_INTEGER_16_BIT
    LIGHTNING_TYPES_INTEGER_32_BIT
    LIGHTNING_TYPES_INTEGER_64_BIT
    LIGHTNING_TYPES_INTEGER_128_BIT
    LIGHTNING_TYPES_INTEGER_256_BIT
])

AC_DEFUN([LIGHTNING_TYPES_FLOAT],
[
    LIGHTNING_TYPES_FLOAT_32_BIT
    LIGHTNING_TYPES_FLOAT_64_BIT
    LIGHTNING_TYPES_FLOAT_128_BIT
])

AC_DEFUN([LIGHTNING_TYPES_INTEGER_8_BIT],
[
    AC_REQUIRE([AC_C_CHAR_UNSIGNED])
    AC_REQUIRE([LIGHTNING_SIZES])

    # A signed 8-bit integer quantity (byte)
    AC_CACHE_VAL(ac_cv_type_LINT8_T,
    [
        if test "$ac_cv_sizeof_signed_char" = 1 ; then
            LIGHTNING_CHECK_TYPE(LINT8_T, signed char)

            ac_cv_type_LINT8_T="yes"
        elif test "$ac_cv_c_char_unsigned" = "no" && test "$ac_cv_sizeof_char" = 1 ; then
            LIGHTNING_CHECK_TYPE(LINT8_T, char)

            ac_cv_type_LINT8_T="yes"
        elif test "$ac_cv_sizeof_int8_t" = 1 ; then
            LIGHTNING_CHECK_TYPE(LINT8_T, int8_t)

            ac_cv_type_LINT8_T="yes"
        else
            ac_cv_type_LINT8_T="no"
        fi
    ])
    if test "$ac_cv_type_LINT8_T" != "yes" ; then
        AC_MSG_ERROR([signed 8-bit integer type not found (required)])
    fi

    # An unsigned 8-bit integer quantity (byte)
    AC_CACHE_VAL(ac_cv_type_LUINT8_T,
    [
        if test "$ac_cv_sizeof_unsigned_char" = 1 ; then
            LIGHTNING_CHECK_TYPE(LUINT8_T, unsigned char)

            ac_cv_type_LUINT8_T="yes"
        elif test "$ac_cv_c_char_unsigned" = "yes" && test "$ac_cv_sizeof_char" = 1 ; then
            LIGHTNING_CHECK_TYPE(LUINT8_T, char)

            ac_cv_type_LUINT8_T="yes"
        elif test "$ac_cv_sizeof_uint8_t" = 1 ; then
            LIGHTNING_CHECK_TYPE(LUINT8_T, uint8_t)

            ac_cv_type_LUINT8_T="yes"
        else
            ac_cv_type_LUINT8_T="no"
        fi
    ])
    if test "$ac_cv_type_LUINT8_T" != "yes" ; then
        AC_MSG_ERROR([unsigned 8-bit integer type not found (required)])
    fi
])

AC_DEFUN([LIGHTNING_TYPES_INTEGER_16_BIT],
[
    AC_REQUIRE([AC_C_CHAR_UNSIGNED])
    AC_REQUIRE([LIGHTNING_SIZES])

    # A signed 16-bit integer quantity
    AC_CACHE_VAL(ac_cv_type_LINT16_T,
    [
        if test "$ac_cv_sizeof_short" = 2 ; then
            LIGHTNING_CHECK_TYPE(LINT16_T, short)

            ac_cv_type_LINT16_T="yes"
        elif test "$ac_cv_sizeof_int16_t" = 2 ; then
            LIGHTNING_CHECK_TYPE(LINT16_T, int16_t)

            ac_cv_type_LINT16_T="yes"
        elif test "$ac_cv_sizeof_signed_char" = 2 ; then
            LIGHTNING_CHECK_TYPE(LINT16_T, signed char)

            ac_cv_type_LINT16_T="yes"
        elif test "$ac_cv_c_char_unsigned" = "no" && test "$ac_cv_sizeof_char" = 2 ; then
            LIGHTNING_CHECK_TYPE(LINT16_T, char)

            ac_cv_type_LINT16_T="yes"
        else
            ac_cv_type_LINT16_T="no"
        fi
    ])
    if test "$ac_cv_type_LINT16_T" != "yes" ; then
        AC_MSG_ERROR([signed 16-bit integer type not found (required)])
    fi

    # An unsigned 16-bit integer quantity
    AC_CACHE_VAL(ac_cv_type_LUINT16_T,
    [
        if test "$ac_cv_sizeof_unsigned_short" = 2 ; then
            LIGHTNING_CHECK_TYPE(LUINT16_T, unsigned short)

            ac_cv_type_LUINT16_T="yes"
        elif test "$ac_cv_sizeof_uint16_t" = 2 ; then
            LIGHTNING_CHECK_TYPE(LUINT16_T, uint16_t)

            ac_cv_type_LUINT16_T="yes"
        elif test "$ac_cv_sizeof_unsigned_char" = 2 ; then
            LIGHTNING_CHECK_TYPE(LUINT16_T, unsigned char)

            ac_cv_type_LUINT16_T="yes"
        elif test "$ac_cv_c_char_unsigned" = "yes" && test "$ac_cv_sizeof_char" = 2 ; then
            LIGHTNING_CHECK_TYPE(LUINT16_T, char)

            ac_cv_type_LUINT16_T="yes"
        else
            ac_cv_type_LUINT16_T="no"
        fi
    ])
    if test "$ac_cv_type_LUINT16_T" != "yes" ; then
        AC_MSG_ERROR([unsigned 16-bit integer type not found (required)])
    fi
])

AC_DEFUN([LIGHTNING_TYPES_INTEGER_32_BIT],
[
    AC_REQUIRE([LIGHTNING_SIZES])

    # A signed 32-bit integer quantity
    AC_CACHE_VAL(ac_cv_type_LINT32_T,
    [
        if test "$ac_cv_sizeof_int" = 4 ; then
            LIGHTNING_CHECK_TYPE(LINT32_T, int)

            ac_cv_type_LINT32_T="yes"
        elif test "$ac_cv_sizeof_long" = 4 ; then
            LIGHTNING_CHECK_TYPE(LINT32_T, long)

            ac_cv_type_LINT32_T="yes"
        elif test "$ac_cv_sizeof_short" = 4 ; then
            LIGHTNING_CHECK_TYPE(LINT32_T, short)

            ac_cv_type_LINT32_T="yes"
        elif test "$ac_cv_sizeof_int32_t" = 4 ; then
            LIGHTNING_CHECK_TYPE(LINT32_T, int32_t)

            ac_cv_type_LINT32_T="yes"
        else
            ac_cv_type_LINT32_T="no"
        fi
    ])
    if test "$ac_cv_type_LINT32_T" != "yes" ; then    
        AC_MSG_ERROR([signed 32-bit integer type not found (required)])
    fi

    # An unsigned 32-bit integer quantity
    AC_CACHE_VAL(ac_cv_type_LUINT32_T,
    [
        if test "$ac_cv_sizeof_unsigned_int" = 4 ; then
            LIGHTNING_CHECK_TYPE(LUINT32_T, unsigned int)

            ac_cv_type_LUINT32_T="yes"
        elif test "$ac_cv_sizeof_unsigned_long" = 4 ; then
            LIGHTNING_CHECK_TYPE(LUINT32_T, unsigned long)

            ac_cv_type_LUINT32_T="yes"
        elif test "$ac_cv_sizeof_unsigned_short" = 4 ; then
            LIGHTNING_CHECK_TYPE(LUINT32_T, unsigned short)

            ac_cv_type_LUINT32_T="yes"
        elif test "$ac_cv_sizeof_uint32_t" = 4 ; then
            LIGHTNING_CHECK_TYPE(LUINT32_T, uint32_t)

            ac_cv_type_LUINT32_T="yes"
        else
            ac_cv_type_LUINT32_T="no"
        fi
    ])
    if test "$ac_cv_type_LUINT32_T" != "yes" ; then
        AC_MSG_ERROR([unsigned 32-bit integer type not found (required)])
    fi
])

AC_DEFUN([LIGHTNING_TYPES_INTEGER_64_BIT],
[
    AC_REQUIRE([LIGHTNING_SIZES])

    # A signed 64-bit integer quantity
    AC_CACHE_VAL(ac_cv_type_LINT64_T,
    [
        if test "$ac_cv_sizeof_int" = 8 ; then
            LIGHTNING_CHECK_TYPE(LINT64_T, int)

            ac_cv_type_LINT64_T="yes"
        elif test "$ac_cv_sizeof_long" = 8 ; then
            LIGHTNING_CHECK_TYPE(LINT64_T, long)

            ac_cv_type_LINT64_T="yes"
        elif test "$ac_cv_sizeof_short" = 8 ; then
            LIGHTNING_CHECK_TYPE(LINT64_T, short)

            ac_cv_type_LINT64_T="yes"
        elif test "$ac_cv_sizeof_int64_t" = 8 ; then
            LIGHTNING_CHECK_TYPE(LINT64_T, int64_t)

            ac_cv_type_LINT64_T="yes"
        elif test "$ac_cv_sizeof_long_long" = 8 ; then
            LIGHTNING_CHECK_TYPE(LINT64_T, long long)

            ac_cv_type_LINT64_T="yes"
        else
            ac_cv_type_LINT64_T="no"
        fi
    ])
    if test "$ac_cv_type_LINT64_T" != "yes" ; then
        AC_MSG_WARN([signed 64-bit integer type not found (optional)])
    fi

    # An unsigned 64-bit integer quantity
    AC_CACHE_VAL(ac_cv_type_LUINT64_T,
    [
        if test "$ac_cv_sizeof_unsigned_int" = 8 ; then
            LIGHTNING_CHECK_TYPE(LUINT64_T, unsigned int)

            ac_cv_type_LUINT64_T="yes"
        elif test "$ac_cv_sizeof_unsigned_long" = 8 ; then
            LIGHTNING_CHECK_TYPE(LUINT64_T, unsigned long)

            ac_cv_type_LUINT64_T="yes"
        elif test "$ac_cv_sizeof_unsigned_short" = 8 ; then
            LIGHTNING_CHECK_TYPE(LUINT64_T, unsigned short)

            ac_cv_type_LUINT64_T="yes"
        elif test "$ac_cv_sizeof_uint64_t" = 8 ; then
            LIGHTNING_CHECK_TYPE(LUINT64_T, uint64_t)

            ac_cv_type_LUINT64_T="yes"
        elif test "$ac_cv_sizeof_char_p" = 8 ; then
            LIGHTNING_CHECK_TYPE(LUINT64_T, char *)

            ac_cv_type_LUINT64_T="yes"
        elif test "$ac_cv_sizeof_unsigned_long_long" = 8 ; then
            LIGHTNING_CHECK_TYPE(LUINT64_T, unsigned long long)

            ac_cv_type_LUINT64_T="yes"
        else
            ac_cv_type_LUINT64_T="no"
        fi
    ])
    if test "$ac_cv_type_LUINT64_T" != "yes" ; then
        AC_MSG_WARN([unsigned 64-bit integer type not found (optional)])
    fi
])

AC_DEFUN([LIGHTNING_TYPES_INTEGER_128_BIT],
[
    AC_REQUIRE([LIGHTNING_SIZES])

    # A signed 128-bit integer quantity
    AC_CACHE_VAL(ac_cv_type_LINT128_T,
    [
        if test "$ac_cv_sizeof_int" = 16 ; then
            LIGHTNING_CHECK_TYPE(LINT128_T, int)

            ac_cv_type_LINT128_T="yes"
        elif test "$ac_cv_sizeof_long" = 16 ; then
            LIGHTNING_CHECK_TYPE(LINT128_T, long)

            ac_cv_type_LINT128_T="yes"
        elif test "$ac_cv_sizeof_int128_t" = 16 ; then
            LIGHTNING_CHECK_TYPE(LINT128_T, int128_t)

            ac_cv_type_LINT128_T="yes"
        elif test "$ac_cv_sizeof_long_long" = 16 ; then
            LIGHTNING_CHECK_TYPE(LINT128_T, long long)

            ac_cv_type_LINT128_T="yes"
        else
            ac_cv_type_LINT128_T="no"
        fi
    ])
    if test "$ac_cv_type_LINT128_T" != "yes" ; then
        AC_MSG_WARN([signed 128-bit integer type not found (optional)])
    fi

    # An unsigned 128-bit integer quantity
    AC_CACHE_VAL(ac_cv_type_LUINT128_T,
    [
        if test "$ac_cv_sizeof_unsigned_int" = 16 ; then
            LIGHTNING_CHECK_TYPE(LUINT128_T, unsigned int)

            ac_cv_type_LUINT128_T="yes"
        elif test "$ac_cv_sizeof_unsigned_long" = 16 ; then
            LIGHTNING_CHECK_TYPE(LUINT128_T, unsigned long)

            ac_cv_type_LUINT128_T="yes"
        elif test "$ac_cv_sizeof_uint128_t" = 16 ; then
            LIGHTNING_CHECK_TYPE(LUINT128_T, uint128_t)

            ac_cv_type_LUINT128_T="yes"
        elif test "$ac_cv_sizeof_char_p" = 16 ; then
            LIGHTNING_CHECK_TYPE(LUINT128_T, char *)

            ac_cv_type_LUINT128_T="yes"
        elif test "$ac_cv_sizeof_unsigned_long_long" = 16 ; then
            LIGHTNING_CHECK_TYPE(LUINT128_T, unsigned long long)

            ac_cv_type_LUINT128_T="yes"
        else
            ac_cv_type_LUINT128_T="no"
        fi
    ])
    if test "$ac_cv_type_LUINT128_T" != "yes" ; then
        AC_MSG_WARN([unsigned 128-bit integer type not found (optional)])
    fi
])

AC_DEFUN([LIGHTNING_TYPES_INTEGER_256_BIT],
[
    AC_REQUIRE([LIGHTNING_SIZES])

    # A signed 256-bit integer quantity
    AC_CACHE_VAL(ac_cv_type_LINT256_T,
    [
        if test "$ac_cv_sizeof_int" = 32 ; then
            LIGHTNING_CHECK_TYPE(LINT256_T, int)

            ac_cv_type_LINT256_T="yes"
        elif test "$ac_cv_sizeof_long" = 32 ; then
            LIGHTNING_CHECK_TYPE(LINT256_T, long)

            ac_cv_type_LINT256_T="yes"
        elif test "$ac_cv_sizeof_int256_t" = 32 ; then
            LIGHTNING_CHECK_TYPE(LINT256_T, int256_t)

            ac_cv_type_LINT256_T="yes"
        elif test "$ac_cv_sizeof_long_long" = 32 ; then
            LIGHTNING_CHECK_TYPE(LINT256_T, long long)

            ac_cv_type_LINT256_T="yes"
        else
            ac_cv_type_LINT256_T="no"
        fi
    ])
    if test "$ac_cv_type_LINT256_T" != "yes" ; then
        AC_MSG_WARN([signed 256-bit integer type not found (optional)])
    fi

    # An unsigned 256-bit integer quantity
    AC_CACHE_VAL(ac_cv_type_LUINT256_T,
    [
        if test "$ac_cv_sizeof_unsigned_int" = 32 ; then
            LIGHTNING_CHECK_TYPE(LUINT256_T, unsigned int)

            ac_cv_type_LUINT256_T="yes"
        elif test "$ac_cv_sizeof_unsigned_long" = 32 ; then
            LIGHTNING_CHECK_TYPE(LUINT256_T, unsigned long)

            ac_cv_type_LUINT256_T="yes"
        elif test "$ac_cv_sizeof_uint256_t" = 32 ; then
            LIGHTNING_CHECK_TYPE(LUINT256_T, uint256_t)

            ac_cv_type_LUINT256_T="yes"
        elif test "$ac_cv_sizeof_char_p" = 32 ; then
            LIGHTNING_CHECK_TYPE(LUINT256_T, char *)

            ac_cv_type_LUINT256_T="yes"
        elif test "$ac_cv_sizeof_unsigned_long_long" = 32 ; then
            LIGHTNING_CHECK_TYPE(LUINT256_T, unsigned long long)

            ac_cv_type_LUINT256_T="yes"
        else
            ac_cv_type_LUINT256_T="no"
        fi
    ])
    if test "$ac_cv_type_LUINT256_T" != "yes" ; then
        AC_MSG_WARN([unsigned 256-bit integer type not found (optional)])
    fi
])

AC_DEFUN([LIGHTNING_TYPES_FLOAT_32_BIT],
[
    AC_REQUIRE([LIGHTNING_SIZES])

    # A 32-bit floating-point type
    AC_CACHE_VAL(ac_cv_type_LFLT32_T,
    [
        if test "$ac_cv_sizeof_float" = 4 ; then
            LIGHTNING_CHECK_TYPE(LFLT32_T, float)

            ac_cv_type_LFLT32_T="yes"
        elif test "$ac_cv_sizeof_double" = 4 ; then
            LIGHTNING_CHECK_TYPE(LFLT32_T, double)

            ac_cv_type_LFLT32_T="yes"
        elif test "$ac_cv_sizeof_flt32_t" = 4 ; then
            LIGHTNING_CHECK_TYPE(LFLT32_T, lflt32_t)

            ac_cv_type_LFLT32_T="yes"
        else
            ac_cv_type_LFLT32_T="no"
        fi
    ])
    if test "$ac_cv_type_LFLT32_T" != "yes" ; then
        AC_MSG_WARN([32-bit floating-point type not found (optional)])
    fi
])

AC_DEFUN([LIGHTNING_TYPES_FLOAT_64_BIT],
[
    AC_REQUIRE([LIGHTNING_SIZES])

    # A 64-bit floating-point type
    AC_CACHE_VAL(ac_cv_type_LFLT64_T,
    [
        if test "$ac_cv_sizeof_float" = 8 ; then
            LIGHTNING_CHECK_TYPE(LFLT64_T, float)

            ac_cv_type_LFLT64_T="yes"
        elif test "$ac_cv_sizeof_double" = 8 ; then
            LIGHTNING_CHECK_TYPE(LFLT64_T, double)

            ac_cv_type_LFLT64_T="yes"
        elif test "$ac_cv_sizeof_flt64_t" = 8 ; then
            LIGHTNING_CHECK_TYPE(LFLT64_T, lflt64_t)

            ac_cv_type_LFLT64_T="yes"
        else
            ac_cv_type_LFLT64_T="no"
        fi
    ])
    if test "$ac_cv_type_LFLT64_T" != "yes" ; then
        AC_MSG_WARN([64-bit floating-point type not found (optional)])
    fi
])

AC_DEFUN([LIGHTNING_TYPES_FLOAT_128_BIT],
[
    AC_REQUIRE([LIGHTNING_SIZES])

    # A 128-bit floating-point type
    AC_CACHE_VAL(ac_cv_type_LFLT128_T,
    [
        if test "$ac_cv_sizeof_float" = 16 ; then
            LIGHTNING_CHECK_TYPE(LFLT128_T, float)

            ac_cv_type_LFLT128_T="yes"
        elif test "$ac_cv_sizeof_double" = 16 ; then
            LIGHTNING_CHECK_TYPE(LFLT128_T, double)

            ac_cv_type_LFLT128_T="yes"
        elif test "$ac_cv_sizeof_long_double" = 16 ; then
            LIGHTNING_CHECK_TYPE(LFLT128_T, long double)

            ac_cv_type_LFLT128_T="yes"
        elif test "$ac_cv_sizeof_flt128_t" = 16 ; then
            LIGHTNING_CHECK_TYPE(LFLT128_T, lflt256_t)

            ac_cv_type_LFLT128_T="yes"
        else
            ac_cv_type_LFLT128_T="no"
        fi
    ])
    if test "$ac_cv_type_LFLT128_T" != "yes" ; then
        AC_MSG_WARN([128-bit floating-point type not found (optional)])
    fi
])

AC_DEFUN([LIGHTNING_TYPES_FLOAT_256_BIT],
[
    AC_REQUIRE([LIGHTNING_SIZES])

    # A 256-bit floating-point type
    AC_CACHE_VAL(ac_cv_type_LFLT256_T,
    [
        if test "$ac_cv_sizeof_float" = 32 ; then
            LIGHTNING_CHECK_TYPE(LFLT256_T, float)

            ac_cv_type_LFLT256_T="yes"
        elif test "$ac_cv_sizeof_double" = 32 ; then
            LIGHTNING_CHECK_TYPE(LFLT256_T, double)

            ac_cv_type_LFLT256_T="yes"
        elif test "$ac_cv_sizeof_long_double" = 32 ; then
            LIGHTNING_CHECK_TYPE(LFLT256_T, long double)

            ac_cv_type_LFLT256_T="yes"
        elif test "$ac_cv_sizeof_flt256_t" = 32 ; then
            LIGHTNING_CHECK_TYPE(LFLT256_T, lflt256_t)

            ac_cv_type_LFLT256_T="yes"
        else
            ac_cv_type_LFLT256_T="no"
        fi
    ])
    if test "$ac_cv_type_LFLT256_T" != "yes" ; then
        AC_MSG_WARN([256-bit floating-point type not found (optional)])
    fi
])

AC_DEFUN([LIGHTNING_TYPES_ALIGN],
[
    if test "$ac_cv_type_LUINT8_T" = "yes" ; then
        LIGHTNING_CHECK_TYPE(LALIGN8_T, LUINT8_T)
    elif test "$ac_cv_type_LINT8_T" = "yes" ; then
        LIGHTNING_CHECK_TYPE(LALIGN8_T, LINT8_T)
    fi

    if test "$ac_cv_type_LUINT16_T" = "yes" ; then
        LIGHTNING_CHECK_TYPE(LALIGN16_T, LUINT16_T)
    elif test "$ac_cv_type_LINT16_T" = "yes" ; then
        LIGHTNING_CHECK_TYPE(LALIGN16_T, LINT16_T)
    fi

    if test "$ac_cv_type_LUINT32_T" = "yes" ; then
        LIGHTNING_CHECK_TYPE(LALIGN32_T, LUINT32_T)
    elif test "$ac_cv_type_LINT32_T" = "yes" ; then
        LIGHTNING_CHECK_TYPE(LALIGN32_T, LINT32_T)
    fi

    if test "$ac_cv_type_LUINT64_T" = "yes" ; then
        LIGHTNING_CHECK_TYPE(LALIGN64_T, LUINT64_T)
    elif test "$ac_cv_type_LINT64_T" = "yes" ; then
        LIGHTNING_CHECK_TYPE(LALIGN64_T, LINT64_T)
    elif test "$ac_cv_sizeof_float" = 8 ; then
        LIGHTNING_CHECK_TYPE(LALIGN64_T, float)
    elif test "$ac_cv_sizeof_double" = 8 ; then
        LIGHTNING_CHECK_TYPE(LALIGN64_T, double)
    elif test "$ac_cv_sizeof_long_double" = 8 ; then
        LIGHTNING_CHECK_TYPE(LALIGN64_T, long double)
    else
        AC_MSG_WARN([64-bit type not found (optional)])
    fi

    if test "$ac_cv_type_LUINT128_T" = "yes" ; then
        LIGHTNING_CHECK_TYPE(LALIGN128_T, LUINT128_T)
    elif test "$ac_cv_type_LINT128_T" = "yes" ; then
        LIGHTNING_CHECK_TYPE(LALIGN128_T, LINT128_T)
    elif test "$ac_cv_sizeof_float" = 16 ; then
        LIGHTNING_CHECK_TYPE(LALIGN128_T, float)
    elif test "$ac_cv_sizeof_double" = 16 ; then
        LIGHTNING_CHECK_TYPE(LALIGN128_T, double)
    elif test "$ac_cv_sizeof_long_double" = 16 ; then
        LIGHTNING_CHECK_TYPE(LALIGN128_T, long double)
    else
        AC_MSG_WARN([128-bit type not found (optional)])
    fi

    if test "$ac_cv_type_LUINT256_T" = "yes" ; then
        LIGHTNING_CHECK_TYPE(LALIGN256_T, LUINT256_T)
    elif test "$ac_cv_type_LINT256_T" = "yes" ; then
        LIGHTNING_CHECK_TYPE(LALIGN256_T, LINT256_T)
    elif test "$ac_cv_sizeof_float" = 32 ; then
        LIGHTNING_CHECK_TYPE(LALIGN256_T, float)
    elif test "$ac_cv_sizeof_double" = 32 ; then
        LIGHTNING_CHECK_TYPE(LALIGN256_T, double)
    elif test "$ac_cv_sizeof_long_double" = 32 ; then
        LIGHTNING_CHECK_TYPE(LALIGN256_T, long double)
    else
        AC_MSG_WARN([256-bit type not found (optional)])
    fi
])

AC_DEFUN([LIGHTNING_TYPES_CPU],
[
    LIGHTNING_TYPES_WORD
    LIGHTNING_TYPES_PTR
    LIGHTNING_TYPE_PTRDIFF_T
])

# Check for machine word size types.
#
AC_DEFUN([LIGHTNING_TYPES_WORD],
[
    AC_REQUIRE([LIGHTNING_SIZES])

    AC_MSG_CHECKING([for machine word types])
    AC_CACHE_VAL(ac_cv_type_word_t,
    [
        if test "$ac_cv_sizeof_short" = "$ac_cv_sizeof_char_p" ; then
            LIGHTNING_CHECK_TYPE(word_t, short)

            ac_cv_type_word_t="yes"
        elif test "$ac_cv_sizeof_int" = "$ac_cv_sizeof_char_p" ; then
            LIGHTNING_CHECK_TYPE(word_t, int)

            ac_cv_type_word_t="yes"
        elif test "$ac_cv_sizeof_long" = "$ac_cv_sizeof_char_p" ; then
            LIGHTNING_CHECK_TYPE(word_t, long)

            ac_cv_type_word_t="yes"
        elif test "$ac_cv_sizeof_long_long" = "$ac_cv_sizeof_char_p"; then
            LIGHTNING_CHECK_TYPE(word_t, long long)

            ac_cv_type_word_t="yes"
        else
            LIGHTNING_CHECK_TYPE(word_t, no)
        fi
    ])
    if test "$ac_cv_type_word_t" != "yes" ; then
        AC_MSG_WARN(WORD_T not available)
    fi

    AC_CACHE_VAL(ac_cv_type_uword_t,
    [
        if test "$ac_cv_sizeof_unsigned_short" = "$ac_cv_sizeof_char_p" ; then
            LIGHTNING_CHECK_TYPE(uword_t, unsigned short)

            ac_cv_type_uword_t="yes"
        elif test "$ac_cv_sizeof_unsigned_int" = "$ac_cv_sizeof_char_p" ; then
            LIGHTNING_CHECK_TYPE(uword_t, unsigned int)

            ac_cv_type_uword_t="yes"
        elif test "$ac_cv_sizeof_unsigned_long" = "$ac_cv_sizeof_char_p" ; then
            LIGHTNING_CHECK_TYPE(uword_t, unsigned long)

            ac_cv_type_uword_t="yes"
        elif test "$ac_cv_sizeof_unsigned_long_long" = "$ac_cv_sizeof_char_p"; then
            LIGHTNING_CHECK_TYPE(uword_t, long long)

            ac_cv_type_uword_t="yes"
        else
            LIGHTNING_CHECK_TYPE(uword_t, no)
        fi
    ])
    if test "$ac_cv_type_uword_t" != "yes" ; then
        AC_MSG_WARN(UWORD_T not available)
    fi
])

# Check for pointer size integer types.
#
AC_DEFUN([LIGHTNING_TYPES_PTR],
[
    AC_REQUIRE([LIGHTNING_SIZES])

    AC_MSG_CHECKING([for pointer-size types])
    AC_CACHE_VAL(ac_cv_type_ptrint_t,
    [
        if test "$ac_cv_sizeof_short" = "$ac_cv_sizeof_char_p" ; then
            LIGHTNING_CHECK_TYPE(ptrint_t, short)

            ac_cv_type_ptrint_t="yes"
        elif test "$ac_cv_sizeof_int" = "$ac_cv_sizeof_char_p" ; then
            LIGHTNING_CHECK_TYPE(ptrint_t, int)

            ac_cv_type_ptrint_t="yes"
        elif test "$ac_cv_sizeof_long" = "$ac_cv_sizeof_char_p" ; then
            LIGHTNING_CHECK_TYPE(ptrint_t, long)

            ac_cv_type_ptrint_t="yes"
        elif test "$ac_cv_sizeof_long_long" = "$ac_cv_sizeof_char_p"; then
            LIGHTNING_CHECK_TYPE(ptrint_t, long long)

            ac_cv_type_ptrint_t="yes"
        else
            LIGHTNING_CHECK_TYPE(ptrint_t, no)
        fi
    ])
    if test "$ac_cv_type_ptrint_t" != "yes" ; then
        AC_MSG_WARN(PTRINT_T not available)
    fi

    AC_CACHE_VAL(ac_cv_type_ptruint_t,
    [
        if test "$ac_cv_sizeof_unsigned_short" = "$ac_cv_sizeof_char_p" ; then
            LIGHTNING_CHECK_TYPE(ptruint_t, unsigned short)

            ac_cv_type_ptruint_t="yes"
        elif test "$ac_cv_sizeof_unsigned_int" = "$ac_cv_sizeof_char_p" ; then
            LIGHTNING_CHECK_TYPE(ptruint_t, unsigned int)

            ac_cv_type_ptruint_t="yes"
        elif test "$ac_cv_sizeof_unsigned_long" = "$ac_cv_sizeof_char_p" ; then
            LIGHTNING_CHECK_TYPE(ptruint_t, unsigned long)

            ac_cv_type_ptruint_t="yes"
        elif test "$ac_cv_sizeof_unsigned_long_long" = "$ac_cv_sizeof_char_p"; then
            LIGHTNING_CHECK_TYPE(ptruint_t, long long)

            ac_cv_type_ptruint_t="yes"
        else
            LIGHTNING_CHECK_TYPE(ptruint_t, no)
        fi
    ])
    if test "$ac_cv_type_ptruint_t" != "yes" ; then
        AC_MSG_WARN(PTRUINT_T not available)
    fi
])

# Check for ptrdiff_t.

AC_DEFUN([LIGHTNING_TYPE_PTRDIFF_T],
[
    AC_REQUIRE([LIGHTNING_SIZES])

    AC_CACHE_VAL(ac_cv_type_ptrdiff_t,
    [
        if test "$ac_cv_sizeof_short" = "$ac_cv_sizeof_char_p" ; then
            LIGHTNING_CHECK_TYPE(ptrdiff_t, short)

            ac_cv_type_ptrdiff_t="yes"
        elif test "$ac_cv_sizeof_int" = "$ac_cv_sizeof_char_p" ; then
            LIGHTNING_CHECK_TYPE(ptrdiff_t, int)

            ac_cv_type_ptrdiff_t="yes"
        elif test "$ac_cv_sizeof_long" = "$ac_cv_sizeof_char_p" ; then
            LIGHTNING_CHECK_TYPE(ptrdiff_t, long)

            ac_cv_type_ptrdiff_t="yes"
        elif test "$ac_cv_sizeof_long_long" = "$ac_cv_sizeof_char_p"; then
            LIGHTNING_CHECK_TYPE(ptrdiff_t, long long)

            ac_cv_type_ptrdiff_t="yes"
        else
            LIGHTNING_CHECK_TYPE(ptrdiff_t, no)
        fi
    ])
    if test "$ac_cv_type_ptrdiff_t" != "yes" ; then
        AC_MSG_WARN(ptrdiff_t not available)
    fi
])

