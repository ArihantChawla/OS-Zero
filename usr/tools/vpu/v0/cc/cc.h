#define CPPDEBUG    0
#define CCPROF      0
#define CCDEBUG     0
#define CCPRINT     1
#define CCTOKENCNT 0

#define CC_C99_TYPES 1
#define CCLINELEN    65536

#include <stddef.h>
#if (CC_C99_TYPES)
#include <stdint.h>
#endif

#define ccvarsz(t)      (typesztab[(t)])

#define CC_NONE         0x00

/* compiler warning flags */
#define CC_WARN_UNUSED 0x00000001U
#define CC_WARN_UNDEF  0x00000002U
#define CC_WARN_ERROR  0x00000004U

/* compiler attribute flags */
#define CC_ATR_PACKED   0x01
#define CC_ATR_ALIGNED  0x02
#define CC_ATR_NORETURN 0x03
#define CC_ATR_FORMAT   0x04

/* adr == CC_RESOLVE for unresolved symbols */
#define CC_NO_SYM    0x00      // uninitialised/invalid
#define CC_VALUE_SYM 0x01
#define CC_TYPE_SYM  0x02      // type definition; adr is struct cctype *
#define CC_VAR_SYM   0x03      // variable; adr is struct ccval *
#define CC_FUNC_SYM  0x04      // function construct; adr is struct ccfunc *
#define CC_LABEL_SYM 0x05      // label address (jump target)
/* special value for unresolved symbols */
#define CC_RESOLVE   (~((uintptr_t)0))
struct ccsym {
    long            type;
//    size_t           namelen;
    char           *name;
#if (CCDB)
    size_t          fnamelen;
    char           *fname;
    long            row;
    long            col;
#endif
    uintptr_t       adr;
    struct ccsym   *prev;
    struct ccsym   *next;
};

/* type values */
/* low 16 bits */
#define CC_TYPE_TOKEN         0x0001
#define CC_TYPEDEF_TOKEN      0x0002
#define CC_VAR_TOKEN          0x0003
/* aggregate types */
#define CC_STRUCT_TOKEN       0x0004
#define CC_UNION_TOKEN        0x0005
/* separators */
#define CC_OPER_TOKEN         0x0006
#define CC_DOT_TOKEN          0x0007
#define CC_INDIR_TOKEN        0x0008
#define CC_ASTERISK_TOKEN     0x0009
#define CC_COMMA_TOKEN        0x000a
#define CC_SEMICOLON_TOKEN    0x000b
#define CC_COLON_TOKEN        0x000c
#define CC_EXCLAMATION_TOKEN  0x000d
#define CC_LEFT_PAREN_TOKEN   0x000e
#define CC_RIGHT_PAREN_TOKEN  0x000f
#define CC_INDEX_TOKEN        0x0010
#define CC_END_INDEX_TOKEN    0x0011
#define CC_BLOCK_TOKEN        0x0012
#define CC_END_BLOCK_TOKEN    0x0013
#define CC_QUOTE_TOKEN        0x0014
#define CC_DOUBLE_QUOTE_TOKEN 0x0015
#define CC_BACKSLASH_TOKEN    0x0016
#define CC_NEWLINE_TOKEN      0x0017
/* [constant] value */
#define CC_VALUE_TOKEN        0x0018
/* compiler attributes */
#define CC_STRING_TOKEN       0x0019
#define CC_LITERAL_TOKEN      0x001a
#define CC_QUAL_TOKEN         0x001b
#define CC_ATR_TOKEN          0x001c
#define CC_FUNC_TOKEN         0x001d
#define CC_LABEL_TOKEN        0x001e
#define CC_ADR_TOKEN          0x001f
#define CC_MACRO_TOKEN        0x0020
#define CC_PREPROC_TOKEN      0x0021
#define CC_CONCAT_TOKEN       0x0022
#define CC_STRINGIFY_TOKEN    0x0023
#define CC_INCLUDE_TOKEN      0x0024
#define CC_NTOKTYPE           0x0025
/* parm bits */
/* high 16 bits */
#define CC_UNSIGNED           0x80000000U
#define CC_STATIC             0x40000000U
#define CC_CONST              0x20000000U
#define CC_VOLATILE           0x10000000U
#define CC_EXTERN             0x08000000U
#define CC_WCHAR              0x04000000U
#define CC_INLINE             0x02000000U
#define CC_ALIGNED            0x01000000U // datasz is alignment
#define CC_PACKED             0x00800000U
/* parm values */
#define CC_EXTERN_QUAL        0x0001
#define CC_STATIC_QUAL        0x0002
#define CC_CONST_QUAL         0x0003
#define CC_VOLATILE_QUAL      0x0004
#define CC_IF_DIR             0x0005
#define CC_ELIF_DIR           0x0006
#define CC_ELSE_DIR           0x0007
#define CC_ENDIF_DIR          0x0008
#define CC_IFDEF_DIR          0x0009
#define CC_IFNDEF_DIR         0x000a
#define CC_DEFINE_DIR         0x000b
#define CC_UNDEF_DIR          0x000c
/* adr values */
#define CC_NO_ADR             0x00000000U
#if 0
struct cctoken {
    long            type;
    long            parm;
    char           *str;
    uintptr_t       data;
    struct cctoken *prev;
    struct cctoken *next;
};
#endif

struct cctokenq {
    struct cctoken *head;
    struct cctoken *tail;
};

struct ccinput {
    long              nq;
    struct cctokenq **qptr;
};

/*
 * id is compile-time type ID.
 * for flg field, see struct cctoken.
 * sz is width in [8-bit] bytes
 */
/* integral types */
extern long ccfindtype(char *name);
#define cctoktype(tp)  ((tp)->parm < CC_NTYPE                           \
                        ? (tp)->parm                                    \
                        : ccfindtype((tp)->str))
#if 0
#define ccgetreg(t)    (((t) >> 8) & 0xff)
#define ccvaltype(vp)  ((vp)->type & 0xff)
#define ccvalreg(vp)   ccgetreg((vp)->type)
#endif
/* type ID in low 8 bits */
#define CC_CHAR         0x01
#define CC_UCHAR        0x02
#define CC_SHORT        0x03
#define CC_USHORT       0x04
#define CC_INT          0x05
#define CC_UINT         0x06
#define CC_LONG         0x07
#define CC_ULONG        0x08
#define CC_LONGLONG     0x09
#define CC_ULONGLONG    0x0a
/* floating-point types */
#define CC_FLOAT        0x0b
#define CC_DOUBLE       0x0c
#define CC_LONG_DOUBLE  0x0d
/* void */
#define CC_VOID         0x0e
#define CC_NTYPE        0x0f
/* register ID in bits 8..15 */
#define CC_NO_REG       0xff
/* pointer flags */
/* high 16 bytes */
#define CC_VAL_FLAGS    0xffff0000U
#define CC_PTR          0x80000000U
#define CC_UPTR         0x40000000U
struct ccval {
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
#if (CC_C99_TYPES)
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

struct ccstruct {
    struct cctoken **mtab;
    size_t          *ofstab;
    size_t           nmemb;
};

struct ccunion {
    struct cctoken **mtab;
    size_t          *sztab;
    size_t           nmemb;
};

struct ccfunc {
    struct cctoken *argtab;
    size_t          narg;
    uintptr_t       adr;
};

#if 0
/* type values */
#define CC_TYPE_TOKEN   0x01   // qualifiers in flg, type in parm, datasz
#define CC_VALUE_TOKEN  0x02
#define CC_VAR_TOKEN    0x03   // data is struct ccval
#define CC_MACRO_TOKEN  0x04
#define CC_FUNC_TOKEN   0x05
#define CC_STRUCT_TOKEN 0x06
#define CC_UNION_TOKEN  0x07
#endif
/* parm values */
#define CC_VAR_DECL     0x01
#define CC_VAR_PTR      0x02
#define CC_MACRO_DECL   0x03
#define CC_MACRO_EXP    0x04
#define CC_FUNC_PROTO   0x05
#define CC_FUNC_DECL    0x06
#define CC_FUNC_CALL    0x07
struct cctoken {
    long            type;
    long            flg;
    long            parm;
    char           *str;
    uintptr_t       data;
    size_t          datasz;
    struct cctoken *prev;
    struct cctoken *next;
};

/* machine description */
struct ccmach {
    uint_fast8_t ialn;          // alignment for integral values
    uint_fast8_t faln;          // alignment for floating-point values
    uint_fast8_t jmpaln;        // alignment for labels (if set)
    uint_fast8_t funcaln;       // alignment for functions (if set)
};

struct ccinput * cclex(int argc, char *argv[]);
struct cctoken * ccpreproc(struct cctoken *token, struct cctoken **tailret);

#if (CCPRINT)
void             ccprintqueue(struct cctokenq *queue);
#endif

struct hashstr {
    long            val;
    void           *ptr;
#if (NEWHASH)
    char           *str;
    struct hashstr *next;
#endif
};

