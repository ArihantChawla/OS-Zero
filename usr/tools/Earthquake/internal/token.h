/*
 * token.h - internal preprocessor header for Earthquake C Environment
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_INTERNAL_TOKEN_H
#define EARTHQUAKE_INTERNAL_TOKEN_H

#include <wchar.h>

/*
 * These are actually default values; more storage will be allocated
 * dynamically as necessary.
 */
#define __ECPP_TOKEN_NAME_MAX 64
#define __ECC_TOKEN_NAME_MAX  64

#define __ECC_KEYWORD_HASH_ITEMS 1024
#if (!powerof2(__ECC_KEYWORD_HASH_ITEMS))
#   error __ECC_KEYWORD_HASH_ITEMS must be power of two.
#endif

/*
 * Preprocessor token types.
 */
/*
 * Basic types.
 */
#define __ECPP_IDENTIFIER_TOKEN 1
#define __ECPP_KEYWORD_TOKEN    2
#define __ECPP_CONSTANT_TOKEN   3
#define __ECPP_OPERATOR_TOKEN   4
#define __ECPP_STRING_TOKEN     5
#define __ECPP_SEPARATOR_TOKEN  6
/*
 * Special types for preprocessing.
 */
#define __ECPP_DIRECTIVE_TOKEN  7
/*
 * Preprocessor operations.
 */
#define __ECPP_NULL_OPERATION            0
#define __ECPP_DEFINE_OPERATION          1
#define __ECPP_IF_OPERATION              2
#define __ECPP_IFDEF_OPERATION           3
#define __ECPP_IFNDEF_OPERATION          4
#define __ECPP_LINE_OPERATION            5
#define __ECPP_ERROR_OPERATION           6
#define __ECPP_PRAGMA_OPERATION          7
#define __ECPP_DEFINED_OPERATION         8
#define __ECPP_CONCATENATION_OPERATION   9
#define __ECPP_MACRO_OPERATION          10

/*
 * Compiler token types.
 */
#define __ECC_IDENTIFIER_TOKEN  0x00000001U /* identifier. */
#define __ECC_PROTOTYPE_TOKEN   0x00000002U /* function prototype. */
#define __ECC_CONSTANT_TOKEN    0x00000003U /* constant. */
#define __ECC_STRING_TOKEN      0x00000004U /* string. */
#define __ECC_LABEL_TOKEN       0x00000005U /* label. */
#define __ECC_INITIALIZER_TOKEN 0x00000006U /* initializer. */
#define __ECC_OPERATOR_TOKEN    0x00000007U /* operator. */
#define __ECC_VARIABLE_TOKEN    0x00000008U /* variable. */
#define __ECC_FUNCTION_TOKEN    0x00000009U /* function. */
/*
 * <NOTE>
 *     If you add to this list, update __ECC_TYPE_MASK and all of the flags
 * below.
 * </NOTE>
 */

/*
 * Type flags. Sorted in ascending size order.
 */
#define __ECC_VOID_TYPE             0x00000001U
#define __ECC_CHAR_TYPE             0x00000002U
#define __ECC_SHORT_TYPE            0x00000003U
#define __ECC_INT_TYPE              0x00000004U
#define __ECC_LONG_TYPE             0x00000005U
#define __ECC_LONG_LONG_TYPE        0x00000006U
#define __ECC_FLOAT_TYPE            0x00000007U
#define __ECC_DOUBLE_TYPE           0x00000008U
#define __ECC_LONG_DOUBLE_TYPE      0x00000009U
#define __ECC_STRUCT_TYPE           0x0000000aU
#define __ECC_UNION_TYPE            0x0000000bU
#define __ECC_STRING_TYPE           0x0000000cU
#define __ECC_ENUM_TYPE             0x0000000dU
#define __ECC_TYPE_MASK             0x0000000fU
/*
 * Storage flags.
 */
#define __ECC_EXTERN_FLAG           0x00000010U
#define __ECC_STATIC_FLAG           0x00000020U
#define __ECC_CONST_FLAG            0x00000040U
#define __ECC_VOLATILE_FLAG         0x00000080U
#define __ECC_REGISTER_FLAG         0x00000100U
#define __ECC_INLINE_FLAG           0x00000200U
#define __ECC_ASM_FLAG              0x00000400U
/*
 * More type flags.
 */
#define __ECC_UNSIGNED_FLAG         0x00000800U
#define __ECC_WIDE_FLAG             0x00001000U
#define __ECC_POINTER_FLAG          0x00002000U
#define __ECC_ARRAY_FLAG            0x00004000U
#define __ECC_BITFIELD_FLAG         0x00008000U
#define __ECC_TYPEDEF_FLAG          0x00010000U
/*
 * Misc flags.
 */
#define __ECC_TRIGRAPH_FLAG         0x00020000U
#define __ECC_GLOBAL_FLAG           0x00040000U
#define __ECC_UNRESOLVED_FLAG       0x00080000U
#define __ECC_ALIGNED_FLAG          0x00100000U

#define __ECC_TYPE(t)               ((t)->flags & __ECC_TYPE_MASK)
#define __ECC_SET_TYPE(t, f)        ((t)->flags |= ((f) & __ECC_TYPE_MASK))
#define __ECC_FLAG(t, f)            ((t)->flags & (f))
#define __ECC_SET_FLAG(t, f)        ((t)->flags |= (f))
#define __ECC_CLEAR_FLAG(t, f)      ((t)->flags &= ~(f))

#define __ECC_VOID(t)               (__ECC_TYPE(t) == __ECC_VOID_TYPE)
#define __ECC_CHAR(t)               (__ECC_TYPE(t) == __ECC_CHAR_TYPE)
#define __ECC_SHORT(t)              (__ECC_TYPE(t) == __ECC_SHORT_TYPE)
#define __ECC_INT(t)                (__ECC_TYPE(t) == __ECC_INT_TYPE)
#define __ECC_LONG(t)               (__ECC_TYPE(t) == __ECC_LONG_TYPE)
#define __ECC_LONG_LONG(t)          (__ECC_TYPE(t) == __ECC_LONG_LONG_TYPE)
#define __ECC_FLOAT(t)              (__ECC_TYPE(t) == __ECC_FLOAT_TYPE)
#define __ECC_DOUBLE(t)             (__ECC_TYPE(t) == __ECC_DOUBLE_TYPE)
#define __ECC_LONG_DOUBLE(t)        (__ECC_TYPE(t) == __ECC_LONG_DOUBLE_TYPE)
#define __ECC_STRUCT(t)             (__ECC_TYPE(t) == __ECC_STRUCT_TYPE)
#define __ECC_UNION(t)              (__ECC_TYPE(t) == __ECC_UNION_TYPE)
#define __ECC_STRING(t)             (__ECC_TYPE(t) == __ECC_STRING_TYPE)
#define __ECC_ENUM(t)               (__ECC_TYPE(t) == __ECC_ENUM_TYPE)

#define __ECC_EXTERN(t)             ((t)->flags & __ECC_EXTERN_FLAG)
#define __ECC_STATIC(t)             ((t)->flags & __ECC_STATIC_FLAG)
#define __ECC_CONST(t)              ((t)->flags & __ECC_CONST_FLAG)
#define __ECC_VOLATILE(t)           ((t)->flags & __ECC_VOLATILE_FLAG)
#define __ECC_REGISTER(t)           ((t)->flags & __ECC_REGISTER_FLAG)
#define __ECC_INLINE(t)             ((t)->flags & __ECC_INLINE_FLAG)
#define __ECC_ASM(t)                ((t)->flags & __ECC_ASM_FLAG)

#define __ECC_UNSIGNED(t)           ((t)->flags & __ECC_UNSIGNED_FLAG)
#define __ECC_POINTER(t)            ((t)->flags & __ECC_POINTER_FLAG)
#define __ECC_ARRAY(t)              ((t)->flags & __ECC_ARRAY_FLAG)
#define __ECC_BITFIELD(t)           ((t)->flags & __ECC_BITFIELD_FLAG)

#define __ECC_TRIGRAPH(t)           ((t)->flags & __ECC_TRIGRAPH_FLAG)
#define __ECC_GLOBAL(t)             ((t)->flags & __ECC_GLOBAL_FLAG)
#define __ECC_UNRESOLVED(t)         ((t)->flags & __ECC_UNRESOLVED_FLAG)
#define __ECC_ALIGNED(t)            ((t)->flags & __ECC_ALIGNED_FLAG)

#define __ECC_UNSIGNED_CHAR(t)      (__ECC_CHAR(t)      && __ECC_UNSIGNED(t))
#define __ECC_UNSIGNED_SHORT(t)     (__ECC_SHORT(t)     && __ECC_UNSIGNED(t))
#define __ECC_UNSIGNED_INT(t)       (__ECC_INT(t)       && __ECC_UNSIGNED(t))
#define __ECC_UNSIGNED_LONG(t)      (__ECC_LONG(t)      && __ECC_UNSIGNED(t))
#define __ECC_UNSIGNED_LONG_LONG(t) (__ECC_LONG_LONG(t) && __ECC_UNSIGNED(t))

/*
 * <NOTE>
 * - this structure is always padded for use in arrays.
 * </NOTE>
 */
struct __Ecckeyword {
    uint8_t *name;
    unsigned long type;
    struct __Ecckeyword *chain;
    unsigned long res;
};

/*
 * Preprocessor token.
 */
struct __Ecpptoken {
    unsigned long type;         /* token type. */
    unsigned long operation;
    uint8_t *name;           /* token name. */
    struct __Ecpptoken *next; /* next in input. */
    size_t namelen;
};

/*
 * Compiler token value.
 */
union __Eccvalue {
    void *vp;
    long double ld;
    double d;
    float f;
    unsigned long long ull;
    long long ll;
    unsigned long ul;
    long l;
    wchar_t wc;
    unsigned int ui;
    int i;
    unsigned short us;
    short s;
    unsigned char uc;
    char c;
};

/*
 * <FIXME>
 * - this structure needs padding on some architectures.
 * </FIXME>
 */
/*
 * Compiler token.
 */
struct __Ecctoken {
    unsigned long type;          /* token type. */
    unsigned long valtype;       /* type of value. */
    unsigned long initialized;   /* initialized flag. */
    unsigned long line;          /* line number. */
    struct __ctfile *file;       /* input file. */
    uint8_t *name;               /* token name. */
    struct __Ecctoken *block;     /* owner block. */
    struct __Ecctoken *arguments; /* function or macro arguments. */
    struct __Ecctoken *variables; /* function or block variables. */
    struct __Ecctoken *parentblk; /* parent block. */
    struct __Ecctoken *next;      /* next in input. */
    struct __Ecctoken *chain;     /* next in hash chain. */
    size_t objsize;               /* size of variable/array/struct/union. */
    size_t namelen;               /* name length. */
    union __Eccvalue value;       /* token value. */
};

#endif /* EARTHQUAKE_INTERNAL_TOKEN_H */

