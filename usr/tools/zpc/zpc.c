#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <zero/trix.h>
#include <zpc/zpc.h>
#include <zpc/asm.h>
#include <zpc/op.h>
#include <wpm/wpm.h>

void stkinit(void);
#if (ZPCX11)
void x11init(void);
void x11nextevent(void);
#endif

struct zasopinfo zpcopinfotab[ZPCNASMOP]
= {
    { NULL, 0 },
    { "not", 1 },
    { "xor", 2 },
    { "or", 2 },
    { "and", 2 },
    { "shr", 2 }, 
    { "shra", 2 },
    { "shl", 2 },
    { "ror", 2 },
    { "rol", 2 },
    { "inc", 1 },
    { "dec", 1 },
    { "add", 2 },
    { "sub", 2 },
    { "mul", 2 },
    { "div", 2 },
    { "mod", 2 },
    { "bz", 1 },
    { "bnz", 1 },
    { "blt", 1 },
    { "ble", 1 },
    { "bgt", 1 },
    { "bge", 1 },
    { "mov", 2 },
    { "call", 1 },
    { "ret", 1 },
    { "trap", 1 },
    { "iret", 1 },
    { "thr", 1 },
    { NULL, 0 }
};

#if 0
zpcop_t *zpcopfunctab[ZPCNASMOP] ALIGNED(PAGESIZE)
= {
    NULL,       // illegal instruction
    opnot,
    opand,
    opor,
    opxor,
    opshr,
    opshra,
    opshl,
    opror,
    oprol,
    opinc,
    opdec,
    opadd,
    opsub,
    opmul,
    opdiv,
    opmod64
#if 0
    bz
    bnz
    blt
    ble
    bgt
    bge
    mov
    movd
    movb
    movw
    jmp
    call
    ret
    trap
    iret
    thr
#endif
};
#endif /* 0 */

#define NHASHITEM 1024
static struct zpctoken   *zpcvarhash[NHASHITEM] ALIGNED(PAGESIZE);
//struct zpcstkitem        *zpcstktab[NSTKREG];
#define zpcpush(ip)                                                     \
    ((ip)->next = zpcstk, zpcstk = (ip))
#define zpcpop(tmp)                                                     \
    (((tmp) = zpcstk, zpcstk = ((zpcstk->next) ? zpcstk->next : NULL)), (tmp))
static struct zpcstkitem *zpcstk;
long                      zpcradix;
extern struct zpctoken   *zpctokenqueue;
extern struct zpctoken   *zpcparsequeue;
extern struct zpctoken   *zpcoperstk;
extern struct zpctoken   *zpcoperstktop;
uint8_t                  *physmem;

void
zpcfreequeue(struct zpctoken *queue)
{
    struct zpctoken *token1 = queue;
    struct zpctoken *token2;

    while (token1) {
        token2 = token1->next;
        free(token1);
        token1 = token2;
    }

    return;
}

void
zpcaddvar(struct zpctoken *token)
{
    long  key = 0;
    char *cp;

    cp = token->str;
    while (*cp) {
        key += *cp++;
    }
    key &= NHASHITEM - 1;
    token->next = zpcvarhash[key];
    if (token->next) {
        token->next->prev = token;
    }
    zpcvarhash[key] = token;

    return;
}

struct zpctoken *
zpcfindvar(const char *name)
{
    long             key = 0;
    struct zpctoken *token = NULL;

    while (*name) {
        key += *name++;
    }
    key &= NHASHITEM - 1;
    token = zpcvarhash[key];
    while ((token) && strcmp(name, token->str)) {
        token = token->next;
    }

    return token;
}

void
zpcdelvar(const char *name)
{
    char            *cp;
    long             key = 0;
    struct zpctoken *token = NULL;

    cp = token->str;
    while (*cp) {
        key += *cp++;
    }
    key &= NHASHITEM - 1;
    token = zpcvarhash[key];
    while ((token) && strcmp(name, token->str)) {
        token = token->next;
    }
    if (token) {
        if (token->prev) {
            token->prev->next = token->next;
        } else {
            zpcvarhash[key] = token->next;
        }
        if (token->next) {
            token->next->prev = token->prev;
        }
    }

    return;
}

void
zpcconvbinuint64(uint64_t val, char *str, size_t len)
{
    long     l;
    uint64_t mask = UINT64_C(1) << 63;

    if (len < 67) {
        fprintf(stderr, "not enough size for 64 bits\n");

        return;
    }
    sprintf(str, "0b");
    for (l = 2 ; l < 66 ; l++) {
        snprintf(&str[l], len, "%c", (val & mask) ? '1' : '0');
        mask >>= 1;
    }
    str[l] = '\0';

    return;
}

void
zpcconvbinint64(int64_t val, char *str, size_t len)
{
    long    l;
    int64_t mask = INT64_C(1) << 63;

    if (len < 67) {
        fprintf(stderr, "not enough size for 64 bits\n");

        return;
    }
    sprintf(str, "0b");
    for (l = 2 ; l < 66 ; l++) {
        snprintf(&str[l], len, "%c", (val & mask) ? '1' : '0');
        mask >>= 1;
    }
    str[l] = '\0';

    return;
}

/* clear entire stack */
void
zpcclr(void)
{
    struct zpcstkitem *item = zpcstk;
    struct zpcstkitem *tmp;

    while (item) {
        tmp = item->next;
        free(item->str);
        zpcfreequeue(item->tokq);
        zpcfreequeue(item->parseq);
        free(item);
        item = tmp;
    }

    return;
}

/* drop topmost item from stack */
void
zpcdrop(void)
{
    struct zpcstkitem *tmp;
    struct zpcstkitem *item = zpcpop(tmp);

    if (item) {
        free(item);
    }

    return;
}

/* swap two topmost items on stack */
void
zpcswap(void)
{
    struct zpcstkitem *tmp;
    struct zpcstkitem *item1 = zpcpop(tmp);
    struct zpcstkitem *item2 = zpcpop(tmp);

    if (item1) {
        zpcpush(item1);
    }
    if (item2) {
        zpcpush(item2);
    }

    return;
}

int
main(int argc, char *argv[])
{
//    zasinit(zpcopinfotab);
    zpcradix = 10;
    stkinit();
    exprinit();
#if (ZPCX11)
    x11init();
#endif
    while (1) {
        x11nextevent();
    }

    /* NOTREACHED */
    exit(0);
}

