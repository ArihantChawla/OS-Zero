#define ZCC_C99_TYPES  1

#if (ZCC_C99_TYPES)
#include <stdint.h>
#endif

/* compiler warning flags */
#define ZCC_WARN_UNUSED 0x00000001U
#define ZCC_WARN_UNDEF  0x00000002U
#define ZCC_WARN_ERROR  0x00000004U

/* compiler optimisation flags */
#define ZCC_OPT_NONE    0x00000000U
#define ZCC_OPT_ALIGN   0x00000001U
#define ZCC_OPT_UNROLL  0x00000002U
#define ZCC_OPT_INLINE  0x00000004U

/* compiler attribute flags */
#define ZCC_ATR_PACK    0x00000001U
#define ZCC_ATR_ALIGN   0x00000002U
#define ZCC_ATR_NORET   0x00000004U

/* integral types */
/* type ID in low 16 bits */
#define ZCC_CHAR        0x0001
#define ZCC_UCHAR       0x0002
#define ZCC_SHORT       0x0003
#define ZCC_USHORT      0x0004
#define ZCC_INT         0x0005
#define ZCC_UINT        0x0006
#define ZCC_LONG        0x0007
#define ZCC_ULONG       0x0008
#define ZCC_LONGLONG    0x0009
#define ZCC_ULONGLONG   0x000a
/* floating-point types */
#define ZCC_FLOAT       0x000b
#define ZCC_DOUBLE      0x000c
#define ZCC_LDOUBLE     0x000d
/* pointer flags */
/* high 16 bytes */
#define ZCC_PTR         0x80000000U
#define ZCC_UPTR        0x40000000U
struct zccval {
    unsigned long          type;
    /* integral value */
    union {
        signed char        c;
        unsigned char      uc;
        signed short       s;
        unsigned short     us;
        unsigned int       ui;
        signed int         i;
        signed long        l;
        unsigned long      ul;
        signed long long   ll;
        unsigned long long ull;
#if (ZCC_C99_TYPES)
        int8_t             i8;
        uint8_t            u8;
        int16_t            i16;
        uint16_t           u16;
        int32_t            i32;
        uint32_t           u32;
        int64_t            i64;
        uint64_t           u64;
#endif
    } ival;
    /* address value */
    union {
        intptr_t           p;
        uintptr_t          up;
    } pval;
    /* floating-point value */
    union {
        float              f;
        double             d;
        long double        ld;
    } fval;
};

struct zccsym {
    size_t           namelen;
    char            *name;
    size_t           fnamelen;
    char            *fname;
    uintptr_t        adr;
    struct zccsym   *prev;
    struct zccsym   *next;
};

/* type values */
/* low 16 bits */
/* for these, data is value */
#define ZCC_CHAR_TOKEN      0x0001
#define ZCC_SHORT_TOKEN     0x0002
#define ZCC_INT_TOKEN       0x0003
#define ZCC_LONG_TOKEN      0x0004
#define ZCC_LONG_LONG_TOKEN 0x0005
#if (ZCC_C99_TYPES)
#define ZCC_INT8_TOKEN      0x0006
#define ZCC_INT16_TOKEN     0x0007
#define ZCC_INT32_TOKEN     0x0008
#define ZCC_INT64_TOKEN     0x0009
#endif
/* for these, data is pointer */
#define ZCC_ADR_TOKEN       0x000a
#define ZCC_LATIN1_TOKEN    0x000b
#define ZCC_UTF8_TOKEN      0x000c
#define ZCC_UCS16_TOKEN     0x000d
#define ZCC_UCS32_TOKEN     0x000e
#define ZCC_MACRO_TOKEN     0x000f
#define ZCC_FUNC_TOKEN      0x0010
/* flag bits */
/* high 16 bits */
#define ZCC_UNSIGNED        0x80000000U
#define ZCC_STATIC          0x40000000U
#define ZCC_CONST           0x20000000U
#define ZCC_VOLATILE        0x10000000U
#define ZCC_EXTERN          0x08000000U
#define ZCC_INLINE          0x04000000U // datasz is threshold size
#define ZCC_ALIGN           0x02000000U // datasz is alignment
struct zcctoken {
    unsigned long    type;
    char            *str;
    size_t           datasz;
    uintptr_t        data;
    struct zccsym   *sym;
    struct zcctoken *prev;
    struct zcctoken *next;
};

