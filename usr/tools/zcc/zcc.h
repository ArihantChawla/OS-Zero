#define ZPPDEBUG      1
#define ZCCPROF       1
#define ZCCDEBUG      0
#define ZCCPRINT      1
#define ZPPTOKENCNT   1

#define ZCC_C99_TYPES 1
#define ZCCLINELEN    65536

#include <stddef.h>
#if (ZCC_C99_TYPES)
#include <stdint.h>
#endif

#define zccvarsz(t)      (typesztab[(t)])

#define ZCC_NONE         0x00

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

/* adr == ZCC_RESOLVE for unresolved symbols */
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
#define ZPP_TYPE_TOKEN         0x0001
#define ZPP_TYPEDEF_TOKEN      0x0002
#define ZPP_VAR_TOKEN          0x0003
/* aggregate types */
#define ZPP_STRUCT_TOKEN       0x0004
#define ZPP_UNION_TOKEN        0x0005
/* separators */
#define ZPP_OPER_TOKEN         0x0006
#define ZPP_DOT_TOKEN          0x0007
#define ZPP_INDIR_TOKEN        0x0008
#define ZPP_ASTERISK_TOKEN     0x0009
#define ZPP_COMMA_TOKEN        0x000a
#define ZPP_SEMICOLON_TOKEN    0x000b
#define ZPP_COLON_TOKEN        0x000c
#define ZPP_EXCLAMATION_TOKEN  0x000d
#define ZPP_LEFT_PAREN_TOKEN   0x000e
#define ZPP_RIGHT_PAREN_TOKEN  0x000f
#define ZPP_INDEX_TOKEN        0x0010
#define ZPP_END_INDEX_TOKEN    0x0011
#define ZPP_BLOCK_TOKEN        0x0012
#define ZPP_END_BLOCK_TOKEN    0x0013
#define ZPP_QUOTE_TOKEN        0x0014
#define ZPP_DOUBLE_QUOTE_TOKEN 0x0015
#define ZPP_BACKSLASH_TOKEN    0x0016
#define ZPP_NEWLINE_TOKEN      0x0017
/* [constant] value */
#define ZPP_VALUE_TOKEN        0x0018
/* compiler attributes */
#define ZPP_STRING_TOKEN       0x0019
#define ZPP_LITERAL_TOKEN      0x001a
#define ZPP_QUAL_TOKEN         0x001b
#define ZPP_ATR_TOKEN          0x001c
#define ZPP_FUNC_TOKEN         0x001d
#define ZPP_LABEL_TOKEN        0x001e
#define ZPP_ADR_TOKEN          0x001f
#define ZPP_MACRO_TOKEN        0x0020
#define ZPP_PREPROC_TOKEN      0x0021
#define ZPP_CONCAT_TOKEN       0x0022
#define ZPP_STRINGIFY_TOKEN    0x0023
#define ZPP_INCLUDE_TOKEN      0x0024
#define ZPP_NTOKTYPE           0x0025
/* parm bits */
/* high 16 bits */
#define ZCC_UNSIGNED           0x80000000U
#define ZCC_STATIC             0x40000000U
#define ZCC_CONST              0x20000000U
#define ZCC_VOLATILE           0x10000000U
#define ZCC_EXTERN             0x08000000U
#define ZCC_WCHAR              0x04000000U
#define ZCC_INLINE             0x02000000U
#define ZCC_ALIGNED            0x01000000U // datasz is alignment
#define ZCC_PACKED             0x00800000U
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
#define ZPP_UNDEF_DIR          0x000c
/* adr values */
#define ZCC_NO_ADR             0x00000000U
struct zpptoken {
    long             type;
    long             parm;
    char            *str;
    uintptr_t        data;
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
/* integral types */
#define zccgettype(tp)  ((tp)->parm < ZCC_NTYPE \
                         ? (tp)->parm           \
                         : zccfindtype((tp)->str))
#if 0
#define zccgetreg(t)    (((t) >> 8) & 0xff)
#define zccvaltype(vp)  ((vp)->type & 0xff)
#define zccvalreg(vp)   zccgetreg((vp)->type)
#endif
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
#define ZCC_VOID         0x0e
#define ZCC_NTYPE        0x0f
/* register ID in bits 8..15 */
#define ZCC_NO_REG       0xff
/* pointer flags */
/* high 16 bytes */
#define ZCC_VAL_FLAGS    0xffff0000U
#define ZCC_PTR          0x80000000U
#define ZCC_UPTR         0x40000000U
struct zccval {
    long                   type;
    size_t                 sz;
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

struct zccstruct {
    struct zcctoken **mtab;
    size_t           *ofstab;
    size_t            nmemb;
};

struct zccunion {
    struct zcctoken **mtab;
    size_t           *sztab;
    size_t            nmemb;
};

struct zccfunc {
    struct zcctoken *argtab;
    size_t           narg;
    uintptr_t        adr;
};

/* type values */
#define ZCC_TYPE_TOKEN   0x01   // qualifiers in flg, type in parm, datasz
#define ZCC_VALUE_TOKEN  0x02
#define ZCC_VAR_TOKEN    0x03   // data is struct zccval
#define ZCC_MACRO_TOKEN  0x04
#define ZCC_FUNC_TOKEN   0x05
#define ZCC_STRUCT_TOKEN 0x06
#define ZCC_UNION_TOKEN  0x07
/* parm values */
#define ZCC_VAR_DECL     0x01
#define ZCC_VAR_PTR      0x02
#define ZCC_MACRO_DECL   0x03
#define ZCC_MACRO_EXP    0x04
#define ZCC_FUNC_PROTO   0x05
#define ZCC_FUNC_DECL    0x06
#define ZCC_FUNC_CALL    0x07
struct zcctoken {
    long             type;
    long             flg;
    long             parm;
    char            *str;
    uintptr_t        data;
    size_t           datasz;
    struct zcctoken *prev;
    struct zcctoken *next;
};

/* machine description */
struct zccmach {
    uint_fast8_t ialn;          // alignment for integral values
    uint_fast8_t faln;          // alignment for floating-point values
    uint_fast8_t jmpaln;        // alignment for labels (if set)
    uint_fast8_t funcaln;       // alignment for functions (if set)
};

struct zppinput * zpplex(int argc, char *argv[]);
struct zcctoken * zpppreproc(struct zpptoken *token, struct zcctoken **tailret);

#if (ZCCPRINT)
void              zppprintqueue(struct zpptokenq *queue);
#endif

struct hashstr {
    long            val;
    void           *ptr;
#if (NEWHASH)
    char           *str;
    struct hashstr *next;
#endif
};

