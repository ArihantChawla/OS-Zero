#include <stdlib.h>
#include <string.h>
#include <v86/vm.h>
#include <v86/cc/cc.h>
#include <v86/asm/sym.h>

static struct v86hashsym *v86symhash[V86_SYM_HASH_ITEMS];

#define _v86hashsym(str, key)                                           \
    do {                                                                \
        int _ch = *(unsigned char *)str;                                \
                                                                        \
        while (_ch) {                                                   \
            key <<= V86_SYM_CHAR_BITS;                                  \
            str++;                                                      \
            key |= v86packsymchar(_ch);                                 \
            _ch = *str;                                                 \
        }                                                               \
        key &= (1L << V86_SYM_HASH_BITS) - 1;                           \
    } while (0)

struct v86sym *
v86addsym(const char *name, size_t len, v86adr adr, v86word val)
{
    struct v86sym     *sym = NULL;
    struct v86hashsym *item;
    long               key = 0;

    item = malloc(sizeof(struct v86hashsym));
    if (item) {
        sym = malloc(sizeof(struct v86sym));
        if (sym) {
            sym->name = strdup(name);
            sym->namelen = len;
            sym->adr = adr;
            sym->val = val;
            _v86hashsym(name, key);
            item->sym = sym;
            item->next = v86symhash[key];
            v86symhash[key] = item;
        } else {
            free(item);
        }
    }

    return sym;
}

struct v86sym *
v86findsym(const char *name)
{
    struct v86sym     *sym = NULL;
    struct v86hashsym *item;
    long               key = 0;
    size_t             len = V86_SYM_MAX_CHARS;

    _v86hashsym(name, key);
    item = v86symhash[key];
    while (item && (len--)) {
        if (strcmp(name, item->name)) {
            sym = item->sym;

            break;
        }
        item = item->next;
        len = V86_SYM_MAX_CHARS;
    }

    return sym;
}

