#ifndef __VAS_VAS_H__
#define __VAS_VAS_H__

#include <vas/conf.h>

#if (VASZEROHASH)
#define NHASHBIT 16
#define VASNHASH (1UL << NHASHBIT)
#else
#define VASNHASH 1024
#endif

#include <stddef.h>
#include <stdint.h>

#if (VAS32BIT)
typedef uint32_t  vasmemadr_t;
typedef int32_t   vasword_t;
typedef uint32_t  vasuword_t;
typedef uint32_t  vassize_t;
#else /* 64-bit */
typedef uint64_t  vasmemadr_t;
typedef int64_t   vasword_t;
typedef uint64_t  vasuword_t;
typedef uint64_t  vassize_t;
#endif

#if (ZVM)
#define vasgetmemt(adr, t)      zvmgetmemt(adr, t)
#define vasputmemt(adr, t, val) zvmputmemt(adr, t, val)
#endif

#define VASRESOLVE     (~((vasword_t)0))

#define VASTOKENVALUE  0x01
#define VASTOKENLABEL  0x02
#define VASTOKENINST   0x03
#define VASTOKENREG    0x04
#if (VASVEC)
#define VASTOKENVAREG  0x05
#define VASTOKENVLREG  0x06
#endif
#define VASTOKENDEF    0x07
#define VASTOKENSYM    0x08
#define VASTOKENCHAR   0x09
#define VASTOKENIMMED  0x0a
#define VASTOKENINDIR  0x0b
#define VASTOKENADR    0x0c
#define VASTOKENINDEX  0x0d
#define VASTOKENDATA   0x0e
#define VASTOKENGLOBL  0x0f
#define VASTOKENSPACE  0x10
#define VASTOKENORG    0x11
#define VASTOKENALIGN  0x12
#define VASTOKENASCIZ  0x13
#define VASTOKENSTRING 0x14
#define VASTOKENPAREN  0x15
#if (VASPREPROC)
#define VASTOKENOP     0x16
#endif
#define VASNTOKEN      0x20

struct vasopinfo {
    const char *name;
    uint8_t     narg;
};

struct vasop {
    const char *name;
    uint8_t     code;
    uint8_t     narg;
    uint8_t     len;
    struct vasop *next;
    struct vasop *tab;
};

struct vaslabel {
    char            *name;
    uintptr_t        adr;
    struct vaslabel *next;
};

struct vasvalue {
    vasword_t val;
    uint8_t   size;
};

struct vasinst {
    const char *name;
#if (VASDB)
    uint8_t    *data;
#endif
    uint8_t     op;
    uint8_t     narg;
#if (WPMVEC)
    uint8_t     flg;
#endif
};

struct vassymrec {
    struct vassymrec *next;
    char             *name;
    uintptr_t         adr;
};

struct vasdef {
    struct vasdef *next;
    char          *name;
    vasword_t      val;
};

struct vassym {
    char       *name;
    vasuword_t  adr;
};

struct vasadr {
    char      *name;
    uintptr_t  val;
};

struct vasndx {
    vasword_t reg;
    vasword_t val;
};

struct vasval {
    char          *name;
    vasword_t      val;
    struct vasval *next;
};

struct vastoken {
    struct vastoken     *prev;
    struct vastoken     *next;
    unsigned long        type;
    vasword_t            val;
#if (VASNEWHACKS)
    vasword_t            size;
#endif
    vasword_t            unit;
#if (WPMVEC)
    uint32_t             opflg;
#endif
#if (VASDB)
    uint8_t             *file;
    unsigned long        line;
#endif
    union {
        struct vaslabel  label;
        struct vasvalue  value;
        struct vasinst   inst;
        struct vasdef    def;
        struct vassym    sym;
        struct vasadr    adr;
        struct vasndx    ndx;
        char            *str;
        uint8_t          ch;
        uint8_t          size;
        vasuword_t       reg;
    } data;
};

#if (VASDB) || (WPMDB)
struct vasline {
    struct vasline *next;
    uintptr_t       adr;
    uint8_t        *file;
    unsigned long   num;
    uint8_t        *data;
};

struct vasline * vasfindline(vasmemadr_t adr);
#endif

extern void        vasinit(void);
#if (VASALIGN)
extern void        vasinitalign(void);
#endif
extern vasmemadr_t vastranslate(vasmemadr_t base);
extern void        vasresolve(vasmemadr_t base);
extern void        vasremovesyms(void);
#if (VASBUF)
extern void        vasreadfile(char *name, vasmemadr_t adr, int bufid);
#else
extern void        vasreadfile(char *name, vasmemadr_t adr);
#endif

extern void        vasfreetoken(struct vastoken *token);
extern void        vasqueuesym(struct vassymrec *sym);

extern void        vasprinttoken(struct vastoken *token);

typedef struct vastoken * vastokfunc_t(struct vastoken *, vasmemadr_t,
                                       vasmemadr_t *);

#endif /* __VAS_VAS_H__ */

