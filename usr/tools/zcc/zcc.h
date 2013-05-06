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
    union {
        signed char         c;
        unsigned char       uc;
        signed short        s;
        unsigned short      us;
        unsigned int        ui;
        signed int          i;
        signed long         l;
        unsigned long       ul;
        signed long long    ll;
        unsigned long long  ull;
    } ival;
    union {
        intptr_t            p;
        uintptr_t           up;
    } pval;
    union {
        float               f;
        double              d;
        long double         ld;
    } fval;
};

struct zccsym {
    uintptr_t               adr;
    int                     type;
    int                     flg;
    union {
        struct zccval       val;
    } data;
    struct zccsym          *prev;
    struct zccsym          *next;
};

