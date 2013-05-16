#define ZCCPROF     1
#define ZCCDEBUG    0
#define ZCCPRINT    0
#define ZPPTOKENCNT 1

#define ZCC_C99_TYPES  1
#define ZCCLINELEN     65536

#include <stddef.h>
#if (ZCC_C99_TYPES)
#include <stdint.h>
#endif

#define ZCC_NONE        0x00

/* compiler warning flags */
#define ZCC_WARN_UNUSED  0x00000001U
#define ZCC_WARN_UNDEF   0x00000002U
#define ZCC_WARN_ERROR   0x00000004U

/* compiler optimisation flags */
#define ZCC_OPT_NONE     0x00000000U
#define ZCC_OPT_ALIGN    0x00000001U
#define ZCC_OPT_UNROLL   0x00000002U
#define ZCC_OPT_INLINE   0x00000004U

/* compiler attribute flags */
#define ZCC_ATR_PACKED   0x01
#define ZCC_ATR_ALIGNED  0x02
#define ZCC_ATR_NORETURN 0x03
#define ZCC_ATR_FORMAT   0x04

/* adr == ZCC_NO_SYM for unresolved symbols */
#define ZCC_NO_SYM    0x00      // uninitialised/invalid
#define ZCC_VALUE_SYM 0x01
#define ZCC_TYPE_SYM  0x02      // type definition; adr is struct zcctype *
#define ZCC_VAR_SYM   0x03      // variable; adr is struct zccval *
#define ZCC_FUNC_SYM  0x04      // function construct; adr is struct zccfunc *
#define ZCC_LABEL_SYM 0x05      // label address (jump target)
/* special value for unresolved symbols */
#define ZCC_RESOLVE   (~((uintptr_t)0))
struct zccsym {
    long             type;
//    size_t           namelen;
    char            *name;
#if (ZCCDB)
    size_t           fnamelen;
    char            *fname;
    long             row;
    long             col;
#endif
    uintptr_t        adr;
    struct zccsym   *prev;
    struct zccsym   *next;
};

/* type values */
/* low 16 bits */
/* for these, data is value except for ZPP_TYPEDEF_TOKEN it's struct zcctype */
#define ZPP_TYPE_TOKEN         0x0001
#define ZPP_TYPEDEF_TOKEN      0x0002
#define ZPP_VAR_TOKEN          0x0003
#define ZPP_CHAR_TOKEN         0x0004
#define ZPP_SHORT_TOKEN        0x0005
#define ZPP_INT_TOKEN          0x0006
#define ZPP_LONG_TOKEN         0x0007
#define ZPP_LONG_LONG_TOKEN    0x0008
#if (ZCC_C99_TYPES)
#define ZPP_INT8_TOKEN         0x0009
#define ZPP_INT16_TOKEN        0x000a
#define ZPP_INT32_TOKEN        0x000b
#define ZPP_INT64_TOKEN        0x000c
#endif
/* aggregate types */
#define ZPP_STRUCT_TOKEN       0x000d
#define ZPP_UNION_TOKEN        0x000e
/* separators */
#define ZPP_OPER_TOKEN         0x000f
#define ZPP_DOT_TOKEN          0x0010
#define ZPP_INDIR_TOKEN        0x0011
#define ZPP_ASTERISK_TOKEN     0x0012
#define ZPP_COMMA_TOKEN        0x0013
#define ZPP_SEMICOLON_TOKEN    0x0014
#define ZPP_COLON_TOKEN        0x0015
#define ZPP_EXCLAMATION_TOKEN  0x0016
#define ZPP_LEFT_PAREN_TOKEN   0x0017
#define ZPP_RIGHT_PAREN_TOKEN  0x0018
#define ZPP_INDEX_TOKEN        0x0019
#define ZPP_END_INDEX_TOKEN    0x001a
#define ZPP_BLOCK_TOKEN        0x001b
#define ZPP_END_BLOCK_TOKEN    0x001c
#define ZPP_QUOTE_TOKEN        0x001d
#define ZPP_DOUBLE_QUOTE_TOKEN 0x001e
#define ZPP_BACKSLASH_TOKEN    0x001f
/* [constant] value */
#define ZPP_VALUE_TOKEN        0x0020
/* compiler attributes */
#define ZPP_QUAL_TOKEN         0x0021
#define ZPP_ATR_TOKEN          0x0022
#define ZPP_FUNC_TOKEN         0x0023
#define ZPP_LABEL_TOKEN        0x0024
#define ZPP_ADR_TOKEN          0x0025
#define ZPP_MACRO_TOKEN        0x0026
#define ZPP_PREPROC_TOKEN      0x0027
#define ZPP_CONCAT_TOKEN       0x0028
#define ZPP_STRINGIFY_TOKEN    0x0029
#define ZPP_LATIN1_TOKEN       0x002a
#define ZPP_UTF8_TOKEN         0x002b
#define ZPP_UCS16_TOKEN        0x002c
#define ZPP_UCS32_TOKEN        0x002d
/* flag bits */
/* high 16 bits */
#define ZCC_UNSIGNED           0x80000000U
#define ZCC_STATIC             0x40000000U
#define ZCC_CONST              0x20000000U
#define ZCC_VOLATILE           0x10000000U
#define ZCC_EXTERN             0x08000000U
#define ZCC_INLINE             0x04000000U // datasz is threshold size
#define ZCC_ALIGN              0x02000000U // datasz is alignment
#define ZCC_PACK               0x01000000U
/* parm values */
#define ZCC_EXTERN_QUAL        0x0001
#define ZCC_STATIC_QUAL        0x0002
#define ZCC_CONST_QUAL         0x0003
#define ZCC_VOLATILE_QUAL      0x0004
#define ZPP_IF_DIR             0x0005
#define ZPP_ELIF_DIR           0x0006
#define ZPP_ELSE_DIR           0x0007
#define ZPP_ENDIF_DIR          0x0008
#define ZPP_IFDEF_DIR          0x0009
#define ZPP_IFNDEF_DIR         0x000a
#define ZPP_DEFINE_DIR         0x000b
/* adr values */
#define ZCC_NO_ADR            ((void *)(~0L))
struct zpptoken {
    long             type;
    long             parm;
    char            *str;
    void            *adr;
    struct zpptoken *prev;
    struct zpptoken *next;
};

struct zpptokenq {
    struct zpptoken *head;
    struct zpptoken *tail;
};

struct zppinput {
    long              nq;
    struct zpptokenq **qptr;
};

/*
 * id is compile-time type ID.
 * for flg field, see struct zpptoken.
 * sz is width in [8-bit] bytes
 */
struct zcctype {
    char   *name;
    long    id;
    long    flg;
    size_t  sz;
};

/* integral types */
//#define zccgettype(t)   ((t) & 0xff)
#define zccgetreg(t)    (((t) >> 8) & 0xff)
#define zccvaltype(vp)  ((vp)->type & 0xff)
#define zccvalreg(vp)   zccgetreg((vp)->type)
/* type ID in low 8 bits */
#define ZCC_CHAR         0x01
#define ZCC_UCHAR        0x02
#define ZCC_SHORT        0x03
#define ZCC_USHORT       0x04
#define ZCC_INT          0x05
#define ZCC_UINT         0x06
#define ZCC_LONG         0x07
#define ZCC_ULONG        0x08
#define ZCC_LONGLONG     0x09
#define ZCC_ULONGLONG    0x0a
/* floating-point types */
#define ZCC_FLOAT        0x0b
#define ZCC_DOUBLE       0x0c
#define ZCC_LDOUBLE      0x0d
/* register ID in bits 8..15 */
#define ZCC_NO_REG       0xff
/* pointer flags */
/* high 16 bytes */
#define ZCC_VAL_FLAGS    0xffff0000U
#define ZCC_PTR          0x80000000U
#define ZCC_UPTR         0x40000000U
struct zccval {
    long                   type;
    long                   sz;
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

struct zccmacro {
    char             *name;
    long              narg;
    struct zpptoken  *tokq;
};

struct zccfunc {
    char            *name;
    long             narg;
    long            *argt;
    struct zpptoken *tokq;
};

#define ZCC_TYPE_TOKEN  0x00
#define ZCC_VALUE_TOKEN 0x01
#define ZCC_VAR_TOKEN   0x02
#define ZCC_MACRO_TOKEN 0x03
#define ZCC_FUNC_TOKEN  0x04
struct zcctoken {
    long             type;
    char            *str;
    void            *data;
    struct zcctoken *prev;
    struct zcctoken *next;
};

struct zppinput * zpplex(int argc, char *argv[]);
struct zcctoken * zpppreproc(struct zpptoken *token);

