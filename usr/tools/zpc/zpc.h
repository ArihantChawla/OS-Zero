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

/* configuration for shunting yard algorithm */
#define shuntradix       zpcradix
#define SHUNT_TOKEN      struct zpctoken
#define SHUNT_INT64      ZPCINT64
#define SHUNT_UINT64     ZPCUINT64
#define SHUNT_RESULT     int64_t
#define SHUNT_OP         zpcop_t
#define SHUNT_NARGTAB    shuntcopnargtab
#define SHUNT_EVALTAB    zpcevaltab
#define SHUNT_LEFTPAREN  ZPCLEFTPAREN
#define SHUNT_RIGHTPAREN ZPCRIGHTPAREN
#define shuntprintstr(tok, val, rdx)                                    \
    zpcprintstr64(tok, val, rdx)
#define shuntcisrtol(tok) (shuntcopprec(tok) & SHUNTCRTOL)
#define shuntcprec(tok)   (shuntcopprectab[(tok)->type])
#define shuntcisvalue(tok)                                              \
    ((tok) && (tok)->type >= ZPCINT64 && (tok)->type <= ZPCCOMPLEX)
#define shuntcisfunc(tok)                                               \
    ((tok) && (tok)->type == SHUNTCFUNC)
#define shuntcissep(tok)                                                \
    ((tok) && (tok)->type == SHUNTCSEP)
#define shuntcisop(tok)                                                 \
    ((tok) && ((tok)->type >= SHUNTCNOT && (tok)->type <= SHUNTCASSIGN))
#include <zero/shunt.h>
#define zpcqueuetoken(tok, queue, tail)                                 \
    shuntqueue(tok, queue, tail)

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

#define ZPCSHRA       (SHUNTCNOP + 0x00)
#define ZPCROR        (SHUNTCNOP + 0x01)
#define ZPCROL        (SHUNTCNOP + 0x02)
#define ZPCBZ         (SHUNTCNOP + 0x03)
#define ZPCBNZ        (SHUNTCNOP + 0x04)
#define ZPCBLT        (SHUNTCNOP + 0x05)
#define ZPCBLE        (SHUNTCNOP + 0x06)
#define ZPCBGT        (SHUNTCNOP + 0x07)
#define ZPCBGE        (SHUNTCNOP + 0x08)
#define ZPCMOV        (SHUNTCNOP + 0x09)
#define ZPCMOVD       (SHUNTCNOP + 0x0a)
#define ZPCMOVB       (SHUNTCNOP + 0x0b)
#define ZPCMOVW       (SHUNTCNOP + 0x0c)
#define ZPCJMP        (SHUNTCNOP + 0x0d)
#define ZPCCALL       (SHUNTCNOP + 0x0e)
#define ZPCRET        (SHUNTCNOP + 0x0f)
#define ZPCTRAP       (SHUNTCNOP + 0x10)
#define ZPCIRET       (SHUNTCNOP + 0x11)
#define ZPCTHR        (SHUNTCNOP + 0x12)
#define ZPCNASMOP     (ZPCTHR + 1)
#define ZPCNOPER      (ZPCTHR + 2)
#define ZPCLEFTPAREN  (SHUNTCNOP + 0x13)
#define ZPCRIGHTPAREN (SHUNTCNOP + 0x14)
#define ZPCFUNC       (SHUNTCNOP + 0x15)
#define ZPCSEP        (SHUNTCNOP + 0x16)
#define ZPCVAR        (SHUNTCNOP + 0x17)
#define ZPCINT64      (SHUNTCNOP + 0x18)
#define ZPCUINT64     (SHUNTCNOP + 0x19)
#define ZPCFLOAT      (SHUNTCNOP + 0x1a)
#define ZPCDOUBLE     (SHUNTCNOP + 0x1b)
#define ZPCSTRING     (SHUNTCNOP + 0x1c)
#define ZPCVECTOR     (SHUNTCNOP + 0x1d)
#define ZPCMATRIX     (SHUNTCNOP + 0x1e)
#define ZPCCOMPLEX    (SHUNTCNOP + 0x1f)
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
#if 0
extern long     zpcopprectab[SHUNTCNOP];
extern long     zpcopnargtab[SHUNTCNOP];
#endif
extern zpcop_t *zpcevaltab[ZPCNOPER];

#endif /* __ZPC_ZPC_H__ */

