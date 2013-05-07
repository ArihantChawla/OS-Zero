#define ZCC_C99_TYPES  1

#if (ZCC_C99_TYPES)
#include <stdint.h>
#endif

/* compiler warning flags */
#define ZCC_WARN_UNUSED 0x00000001

/* compiler optimisation flags */
#define ZCC_OPT_NONE    0x00000000
#define ZCC_OPT_ALIGN   0x00000001
#define ZCC_OPT_UNROLL  0x00000002
#define ZCC_OPT_INLINE  0x00000004

/* compiler attribute flags */
#define ZCC_ATR_PACK    0x00000001
#define ZCC_ATR_ALIGN   0x00000002
#define ZCC_ATR_NORET   0x00000004

/* integral types */
#define zcctype(t)      ((t) & ~(ZCC_PTR | ZCC_UPTR))
#define ZCC_CHAR        0x01
#define ZCC_UCHAR       0x02
#define ZCC_SHORT       0x03
#define ZCC_USHORT      0x04
#define ZCC_INT         0x05
#define ZCC_UINT        0x06
#define ZCC_LONG        0x07
#define ZCC_ULONG       0x08
#define ZCC_LONGLONG    0x09
#define ZCC_ULONGLONG   0x0a
/* floating-point types */
#define ZCC_FLOAT       0x0b
#define ZCC_DOUBLE      0x0c
#define ZCC_LDOUBLE     0x0d
/* pointer flags */
#define ZCC_PTR         0x80
#define ZCC_UPTR        0x40

struct zccval {
    int type;
    int flg;
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
} PACK();

struct zccsym {
    uintptr_t          adr;
    int                type;
    int                flg;
    union {
        struct zccval  val;
    } data;
} PACK();

/* type values */
#define ZCC_CHAR_TOKEN      0x01
#define ZCC_SHORT_TOKEN     0x02
#define ZCC_INT_TOKEN       0x03
#define ZCC_LONG_TOKEN      0x04
#define ZCC_LONG_LONG_TOKEN 0x05
#if (ZCC_C99_TYPES)
#define ZCC_INT8_TOKEN      0x06
#define ZCC_INT16_TOKEN     0x07
#define ZCC_INT32_TOKEN     0x08
#define ZCC_INT64_TOKEN     0x09
#endif
#define ZCC_ADR_TOKEN       0x0a
#define ZCC_LATIN1_TOKEN    0x0b
#define ZCC_UTF8_TOKEN      0x0c
#define ZCC_UCS16_TOKEN     0x0d
#define ZCC_UCS32_TOKEN     0x0e
#define ZCC_MACRO_TOKEN     0x0f
#define ZCC_FUNC_TOKEN      0x10
/* flag bits */
#define ZCC_UNSIGNED        0x01
#define ZCC_INLINE          0x02
struct zcctoken {
    int              type;
    int              flg;
    char            *str;
    size_t           datasz;
    uintptr_t        data;
    struct zcctoken *prev;
    struct zcctoken *next;
} PACK();

