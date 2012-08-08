#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <zero/trix.h>
#include <zpc/zpc.h>
#include <zpc/op.h>

void stkinit(void);
#if (ZPCX11)
void x11init(void);
void x11nextevent(void);
#endif
#define TEST 0
#if (TEST)
void zpcprintqueue(struct zpctoken *queue);
#endif

#if 0
struct zpcbutton {
    char              *str;
    union {
        zpccfunc_t    *cop;
        zpczerofunc_t *zop;
    } func;
};
#endif

#define NHASHITEM 1024
static struct zpctoken   *zpcvarhash[NHASHITEM] ALIGNED(PAGESIZE);
static char               zpclinebuf[128];
//struct zpcstkitem        *zpcstktab[NSTKREG];
#define zpcpush(ip)                                                     \
    ((ip)->next = zpcstk, zpcstk = (ip))
#define zpcpop(tmp)                                                     \
    (((tmp) = zpcstk, zpcstk = ((zpcstk->next) ? zpcstk->next : NULL)), (tmp))
static struct zpcstkitem *zpcstk;
extern struct zpctoken   *zpctokenqueue;
extern struct zpctoken   *zpcparsequeue;
extern struct zpctoken   *zpcoperstk;
extern struct zpctoken   *zpcoperstktop;

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
zpcconvbin(uint64_t val)
{
    long     l;
    uint64_t mask = UINT64_C(1) << 63;

    for (l = 0 ; l < 64 ; l++) {
        sprintf(&zpclinebuf[l], "%c", (val & mask) ? '1' : '0');
        mask >>= 1;
    }

    return;
}

void
zpcprintitem(struct zpcstkitem *item)
{
    if (item->type == ZPCUINT) {
        if (item->size == 1) {
            switch(item->radix) {
                case 2:
                    zpcconvbin(zpcgetvalu8(&item->data.u64));
                    printf("%s\n", zpclinebuf);

                    break;
                case 8:
                    printf("%#o\n", zpcgetvalu8(&item->data.u64));

                    break;
                case 10:
                    printf("%d\n", zpcgetvalu8(&item->data.u64));

                    break;
                case 16:
                    printf("%x\n", zpcgetvalu8(&item->data.u64));

                    break;
            }
        } else if (item->size == 2) {
            switch(item->radix) {
                case 2:
                    zpcconvbin(zpcgetvalu16(&item->data.u64));
                    printf("%s\n", zpclinebuf);

                    break;
                case 8:
                    printf("%#o\n", zpcgetvalu16(&item->data.u64));

                    break;
                case 10:
                    printf("%d\n", zpcgetvalu16(&item->data.u64));

                    break;
                case 16:
                    printf("%x\n", zpcgetvalu16(&item->data.u64));

                    break;
            }
        } else if (item->size == 4) {
            switch(item->radix) {
                case 2:
                    zpcconvbin(zpcgetvalu32(&item->data.u64));
                    printf("%s\n", zpclinebuf);

                    break;
                case 8:
                    printf("%#o\n", zpcgetvalu32(&item->data.u64));

                    break;
                case 10:
                    printf("%u\n", zpcgetvalu32(&item->data.u64));

                    break;
                case 16:
                    printf("%x\n", zpcgetvalu32(&item->data.u64));

                    break;
            }
        } else {
            switch(item->radix) {
                case 2:
                    zpcconvbin(zpcgetvalu64(&item->data.u64));
                    printf("%s\n", zpclinebuf);

                    break;
                case 8:
                    printf("%#llo\n", zpcgetvalu64(&item->data.u64));

                    break;
                case 10:
                    printf("%llu\n", zpcgetvalu64(&item->data.u64));

                    break;
                case 16:
                    printf("%llx\n", zpcgetvalu64(&item->data.u64));

                    break;
            }
        }
    } else if (item->type == ZPCINT) {
        if (item->size == 1) {
            switch(item->radix) {
                case 2:
                    zpcconvbin(zpcgetvalu8(&item->data.i64));
                    printf("%s\n", zpclinebuf);

                    break;
                case 8:
                    printf("%#o\n", zpcgetvalu8(&item->data.i64));

                    break;
                case 10:
                    printf("%d\n", zpcgetvalu8(&item->data.i64));

                    break;
                case 16:
                    printf("%x\n", zpcgetvalu8(&item->data.i64));

                    break;
            }
        } else if (item->size == 2) {
            switch(item->radix) {
                case 2:
                    zpcconvbin(zpcgetvalu16(&item->data.i64));
                    printf("%s\n", zpclinebuf);

                    break;
                case 8:
                    printf("%#o\n", zpcgetvalu16(&item->data.i64));

                    break;
                case 10:
                    printf("%d\n", zpcgetvalu16(&item->data.i64));

                    break;
                case 16:
                    printf("%x\n", zpcgetvalu16(&item->data.i64));

                    break;
            }
        } else if (item->size == 4) {
            switch(item->radix) {
                case 2:
                    zpcconvbin(zpcgetvalu32(&item->data.i64));
                    printf("%s\n", zpclinebuf);

                    break;
                case 8:
                    printf("%#o\n", zpcgetvalu32(&item->data.i64));

                    break;
                case 10:
                    printf("%d\n", zpcgetvalu32(&item->data.i64));

                    break;
                case 16:
                    printf("%x\n", zpcgetvalu32(&item->data.i64));

                    break;
            }
        } else {
            switch(item->radix) {
                case 2:
                    zpcconvbin(zpcgetvalu64(&item->data.i64));
                    printf("%s\n", zpclinebuf);

                    break;
                case 8:
                    printf("%#llo\n", zpcgetvalu64(&item->data.i64));

                    break;
                case 10:
                    printf("%lld\n", zpcgetvalu64(&item->data.i64));

                    break;
                case 16:
                    printf("%llx\n", zpcgetvalu64(&item->data.i64));

                    break;
            }
        }
    } else if (item->size == 4) {
        printf("%f\n", item->data.f32);
    } else {
        printf("%e\n", item->data.f64);
    }
}

/* clear entire stack */
void
zpcclr(void)
{
    struct zpcstkitem *item = zpcstk;
    struct zpcstkitem *tmp;

    while (item) {
        tmp = item->next;
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
#if (TEST)
    struct zpctoken *queue;
    const char *exprstr = "3 + 4 * 2 / ( 1 - 5 ) ^ 2 ^ 3";
#endif
//    const char *exprstr = "1^2+3";

    stkinit();
    exprinit();
#if (ZPCX11)
    x11init();
#endif
#if (TEST)
    queue = zpctokenize(exprstr);
    zpcprintqueue(queue);
    zpcparsequeue = zpcparse(queue);
    fprintf(stderr, "PARSE\n");
    fprintf(stderr, "-----\n");
    zpcprintqueue(zpcparsequeue);
#if 0
    fprintf(stderr, "STACK\n");
    fprintf(stderr, "-----\n");
    zpcprintqueue(zpcoperstk);
#endif
#endif
    while (1) {
        x11nextevent();
    }
#if 0
    for (i = 0 ; i < 64 ; i++) {
        tzero64(u64, tmp);
        zpcconvbin(u64);
        printf("%s\n", zpclinebuf);
        u64 <<= 1;
    }
    item = malloc(sizeof(struct zpcstkitem));
    item->type = ZPCUINT;
    item->radix = 8;
    item->size = 4;
    item->data.i64 = 0xffffffff;
    zpcprintitem(item);
    item->type = ZPCUINT;
    item->radix = 16;
    item->size = 4;
    item->data.i64 = 0xffffffff;
    zpcprintitem(item);
    item->type = ZPCINT;
    item->radix = 8;
    item->size = 1;
    item->data.i64 = 0xffffffff;
    zpcprintitem(item);
    item->type = ZPCFLOAT;
    item->radix = 8;
    item->size = 8;
    item->data.f64 = 3.1415926536;
    zpcprintitem(item);
#endif

    exit(0);
}

