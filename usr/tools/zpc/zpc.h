#ifndef __ZPC_ZPC_H__
#define __ZPC_ZPC_H__

#define ZPCASM      0

#include <stdint.h>
#include <stddef.h>
#include <zero/param.h>
#include <zero/cdecl.h>
//#include <zpc/asm.h>

struct zpcv64v8 {
    uint8_t vec[8];
} PACK();

struct zpcv64v16 {
    uint16_t vec[4];
} PACK();

struct zpcv64v32 {
    uint32_t vec[2];
} PACK();

union zpcv64 {
    struct zpcv64v8  v8;
    struct zpcv64v16 v16;
    struct zpcv64v32 v32;
} vec;

union zpcui64 {
    int64_t  i64;
    uint64_t u64;
};

struct zpcvector {
    long              type;
    long              ndim;
    struct zpctoken **toktab;
};

struct zpccomplex {
    long              type;
    long              rsign;
    long              isign;
    union {
        union zpcui64 ui64;
        float         f32;
        double        f64;
    } real;
    union {
        union zpcui64 ui64;
        float         f32;
        double        f64;
    } img;
};

#define zpcisoperchar(c)                                                \
    (zpcoperchartab[(int)(c)])
#define zpcwordsize(tp)                                                 \
    ((tp)->param & PARAMSIZEMASK)
struct zpctoken {
    long                   type;
    char                  *str;
    long                   slen;
    long                   len;
    long                   param;
    long                   radix;
    long                   sign;
    long                   flags;
    union {
        union zpcv64       v64;
        union zpcui64      ui64;
        float              f32;
        double             f64;
        struct zpcvector   vector;
        struct zpccomplex  complex;
//        struct zpcv128     v128;
    } data;
    struct zpctoken       *prev;
    struct zpctoken       *next;
};

#define shuntradix       zpcradix
#define SHUNT_TOKEN      struct zpctoken
#define SHUNT_INT64      ZPCINT64
#define SHUNT_UINT64     ZPCUINT64
#define SHUNT_RESULT     int64_t
#define SHUNT_OP         zpcop_t
#define SHUNT_NARGTAB    zpcopnargtab
#define SHUNT_EVALTAB    zpcevaltab
#define SHUNT_LEFTPAREN  ZPCLEFTPAREN
#define SHUNT_RIGHTPAREN ZPCRIGHTPAREN
#define shuntprintstr(tok, val, rdx)                                    \
    zpcprintstr64(tok, val, rdx)
#define shuntisrtol(tok) zpccopisrtol(tok)
#define shuntprec(tok)   zpccopprec(tok)
#define shuntisvalue(tok)                                                \
    ((tok) && (tok)->type >= ZPCINT64 && (tok)->type <= ZPCCOMPLEX)
#define shuntisfunc(tok)                                                 \
    ((tok) && (tok)->type == ZPCFUNC)
#define shuntissep(tok)                                                  \
    ((tok) && (tok)->type == ZPCSEP)
#define shuntisoper(tok)                                                 \
    ((tok) && ((tok)->type >= ZPCNOT && (tok)->type <= ZPCASSIGN))
#define shuntqueue(tok, queue, tail)                                    \
    zpcqueuetoken(tok, queue, tail)
#define shuntpush(tok, stk)                                             \
    do {                                                                \
        if (tok) {                                                      \
            (tok)->prev = NULL;                                         \
            (tok)->next = *(stk);                                       \
            *(stk) = (tok);                                             \
        }                                                               \
    } while (0)
static __inline__ SHUNT_TOKEN *
shuntpop(SHUNT_TOKEN **stack)
{
    SHUNT_TOKEN *_token;

    if (stack) {
        _token = *stack;
        if (_token) {
            *stack = _token->next;
        }
    }

    return _token;
}
#include <zero/shunt.h>

#define ZPCTEXTBASE 8192

#define SMALLBUTTONS 1

#define NSTKREG     16
#define NREGSTK     1024
#define TOKENSTRLEN 128
#define STKSTRLEN   128
#define DEFAULTDIM  16

#if defined(__alpha__)
#define BIGENDIAN 1
#else
#define BIGENDIAN 0
#endif
#if (ZPCIMLIB2)
#define ZPCREVERSE 1
#else
#define ZPCREVERSE 0
#endif

#if (SMALLBUTTONS)
#define ZPC_BUTTON_WIDTH        48
#define ZPC_BUTTON_HEIGHT       39
#else
#define ZPC_BUTTON_WIDTH        64
#define ZPC_BUTTON_HEIGHT       48
#endif
#define ZPC_SMALL_BUTTON_WIDTH  48
#define ZPC_SMALL_BUTTON_HEIGHT 36
#define ZPC_NROW                6
#define ZPC_NCOLUMN             7
#define ZPC_WINDOW_WIDTH        (ZPC_NCOLUMN * ZPC_BUTTON_WIDTH)
#define ZPC_WINDOW_HEIGHT       (ZPC_NROW * ZPC_BUTTON_HEIGHT)

#define ZPCNOT        0x01
#define ZPCAND        0x02
#define ZPCOR         0x03
#define ZPCXOR        0x04
#define ZPCSHR        0x05
#define ZPCSHRA       0x06
#define ZPCSHL        0x07
#define ZPCROR        0x08
#define ZPCROL        0x09
#define ZPCINC        0x08
#define ZPCDEC        0x09
#define ZPCADD        0x0a
#define ZPCSUB        0x0b
#define ZPCMUL        0x0c
#define ZPCDIV        0x0d
#define ZPCMOD        0x0e
#define ZPCBZ         0x0f
#define ZPCBNZ        0x10
#define ZPCBLT        0x11
#define ZPCBLE        0x12
#define ZPCBGT        0x13
#define ZPCBGE        0x14
#define ZPCMOV        0x15
#define ZPCMOVD       0x16
#define ZPCMOVB       0x17
#define ZPCMOVW       0x18
#define ZPCJMP        0x19
#define ZPCCALL       0x20
#define ZPCRET        0x22
#define ZPCTRAP       0x23
#define ZPCIRET       0x24
#define ZPCTHR        0x25
#define ZPCNASMOP     38
#define ZPCASSIGN     0x26
#define ZPCNOPER      39
#define ZPCLEFTPAREN  0x40
#define ZPCRIGHTPAREN 0x41
#define ZPCFUNC       0x42
#define ZPCSEP        0x44
#define ZPCVAR        0x80
#define ZPCINT64      0x81
#define ZPCUINT64     0x82
#define ZPCFLOAT      0x83
#define ZPCDOUBLE     0x84
#define ZPCSTRING     0x85
#define ZPCVECTOR     0x86
#define ZPCMATRIX     0x87
#define ZPCCOMPLEX    0x88
#define PARAMSIZEMASK 0xff
#define PARAMFLOATBIT 0x40000000
#define PARAMSIGNBIT  0x80000000

#define ZPCBUTTONDIGIT 0x01
#define ZPCBUTTONOPER  0x02
#define ZPCBUTTONASM   0x03
#define ZPCBUTTONUTIL  0x04
#define ZPCBUTTONDEBUG 0x05

#define TOKENSTRLEN 128

#define todec(c)    zpcdectab[(int)(c)]
#define tohex(c)    zpchextab[(int)(c)]
#define tooct(c)    zpcocttab[(int)(c)]
#define todecflt(c) zpcdecflttab[(int)(c)]
#define tohexflt(c) zpchexflttab[(int)(c)]
#define tooctflt(c) zpcoctflttab[(int)(c)]
#define todecdbl(c) zpcdecdbltab[(int)(c)]
#define tohexdbl(c) zpchexdbltab[(int)(c)]
#define tooctdbl(c) zpcoctdbltab[(int)(c)]
#define tobin(c)    ((c) == '0' ? 0 : 1)
#define tobinflt(c) ((c) == '0' ? 0.0f : 1.0f)
#define tobindbl(c) ((c) == '0' ? 0.0 : 1.0)
#define isbdigit(c) ((c) == '0' || (c) == '1')
#define isodigit(c) ((c) >= '0' && (c) <= '7')

#define OPERRTOL  0x80000000
#define zpccopprec(tok)                                                 \
    (zpcopprectab[(tok)->type] & ~OPERRTOL)
#define zpccopisrtol(tok)                                               \
    (zpcopprectab[(tok)->type] & OPERRTOL)

//typedef void zpcophandler_t(struct zpcopcode *);
//typedef void zpchookfunc_t(struct zpcopcode *);
typedef void zpcaction_t(void);

typedef union {
    int32_t i32;
    int64_t i64;
    float   fval;
    double  dval;
} zpcword_t;

/* sign values */
#define ZPCUNSIGNED     0x01
#define ZPCUSERUNSIGNED 0x02
#define ZPCSIGNED       0x03
#define ZPCUSERSIGNED   0x04
/* flags values */
#define ZPCZERO         0x01
#define ZPCOVERFLOW     0x02
#define ZPCUNDERFLOW    0x04
#define ZPCSIGN         0x08
#if 0
#define zpcisvalue(tp)                                                  \
    ((tp) && (tp)->type >= ZPCINT64 && (tp)->type <= ZPCCOMPLEX)
#define zpcisfunc(tp)                                                   \
    ((tp) && (tp)->type == ZPCFUNC)
#define zpcissep(tp)                                                    \
    ((tp) && (tp)->type == ZPCSEP)
#define zpcisoper(tp)                                                   \
    ((tp) && ((tp)->type >= ZPCNOT && (tp)->type <= ZPCASSIGN))
#endif
/* item data types */
#define ZPCUINT  0x01
#define ZPCINT   0x02
#define ZPCFLT   0x03
#define ZPCDBL   0x04
struct zpcstkitem {
    long               type;
    long               radix;
    long               size;
    char              *str;
    struct zpctoken   *tokq;
    struct zpctoken   *parseq;
    long               slen;
    char              *scur;
    union {
        union zpcui64  ui64;
        float          f32;
        double         f64;
    } data;
    struct zpcstkitem *next;
};

struct zpctokenqueue {
    volatile long    lk;
    struct zpctoken *head;
    struct zpctoken *tail;
};

void              exprinit(void);
void              zpcconvbinuint64(uint64_t val, char *str, size_t len);
void              zpcconvbinint64(int64_t val, char *str, size_t len);
void              zpcfreequeue(struct zpctoken *queue);
void              zpcprinttoken(struct zpctoken *token);
struct zpctoken * zpcgettoken(const char *str, char **retstr);
struct zpctoken * zpctokenize(const char *str);
struct zpctoken * zpcparse(struct zpctoken *queue);
struct zpctoken * zpceval(struct zpctoken *srcqueue);

#if 0
#if (BIGENDIAN)
#define zpcgetval8(ptr)                                                 \
    (*(int8_t *)&ptr[7])
#define zpcgetval16(ptr)                                                \
    (*(int16_t *)&ptr[6])
#define zpcgetval32(ptr)                                                \
    (*(int32_t *)&ptr[4])
#define zpcgetval64(ptr)                                                \
    (*(int64_t *)ptr)
#define zpcgetvalu8(ptr)                                                \
    (*(uint8_t *)&ptr[7])
#define zpcgetvalu16(ptr)                                               \
    (*(uint16_t *)&ptr[6])
#define zpcgetvalu32(ptr)                                               \
    (*(uint32_t *)&ptr[4])
#define zpcgetvalu64(ptr)                                               \
    (*(uint64_t *)ptr)
#else
#define zpcgetval8(ptr)                                                 \
    (*(int8_t *)ptr)
#define zpcgetval16(ptr)                                                \
    (*(int16_t *)ptr)
#define zpcgetval32(ptr)                                                \
    (*(int32_t *)ptr)
#define zpcgetval64(ptr)                                                \
    (*(int64_t *)ptr)
#define zpcgetvalu8(ptr)                                                \
    (*(uint8_t *)ptr)
#define zpcgetvalu16(ptr)                                               \
    (*(uint16_t *)ptr)
#define zpcgetvalu32(ptr)                                               \
    (*(uint32_t *)ptr)
#define zpcgetvalu64(ptr)                                               \
    (*(uint64_t *)ptr)
#endif
#define zpcgetvalf32(ptr)                                               \
    (*(float *)ptr)
#define zpcgetvalf64(ptr)                                               \
    (*(double *)ptr)

#define zpcsetval64(ptr, val)                                           \
    *(int64_t *)(ptr) = (val)
#define zpcsetvalu64(ptr, val)                                          \
    *(uint64_t *)(ptr) = (val)
#define zpcsetvalf32(ptr, val)                                          \
    *(float *)(ptr) = (val)
#define zpcsetvalf64(ptr, val)                                          \
    *(double *)(ptr) = (val)
#endif

#define zpcprintstr64(tok, val, rad)                                    \
    do {                                                                \
        switch (rad) {                                                  \
            case 2:                                                     \
                zpcconvbinuint64((val), (tok)->str, (tok)->slen);       \
                                                                        \
                break;                                                  \
            case 8:                                                     \
                snprintf((tok)->str, (tok)->slen, "0%llo", (long long)(val)); \
                                                                        \
                break;                                                  \
            case 10:                                                    \
            default:                                                    \
                if ((tok)->type == ZPCINT64) {                          \
                    snprintf((tok)->str, (tok)->slen, "%lld", (long long)(val)); \
                } else {                                                \
                    snprintf((tok)->str, (tok)->slen, "%llu", (long long)(val)); \
                }                                                       \
                                                                        \
                break;                                                  \
            case 16:                                                    \
                if (val <= 0xff) {                                      \
                    sprintf((tok)->str, "0x%02llx", (long long)(val));  \
                } else if (val <= 0xffff) {                             \
                    sprintf((tok)->str, "0x%04llx", (long long)(val));  \
                } else if (val <= 0xffffffff) {                         \
                    sprintf((tok)->str, "0x%08llx", (long long)(val));  \
                } else {                                                \
                    sprintf((tok)->str, "0x%016llx", (long long)(val)); \
                }                                                       \
                                                                        \
                break;                                                  \
        }                                                               \
        (tok)->len = strlen((tok)->str);                                \
    } while (0)

#include <zpc/op.h>

extern long     zpcradix;
extern long     zpcopprectab[ZPCNOPER];
extern long     zpcopnargtab[ZPCNOPER];
extern zpcop_t *zpcevaltab[ZPCNOPER];

#endif /* __ZPC_ZPC_H__ */

