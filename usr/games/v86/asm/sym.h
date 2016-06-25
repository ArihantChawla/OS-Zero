#ifndef __V86_SYM_H__
#define __V86_SYM_H__

#include <stddef.h>

#define V86_SYM_MAX_CHARS  15   // not counting the trailing '\0' (NUL)
#define V86_SYM_CHAR_BITS  6    // A-Z, a-z, _, 0-9
#define V86_SYM_HASH_ITEMS (1U << V86_SYM_HASH_BITS)
#define V86_SYM_HASH_BITS  10

/* special adr-member value */
#define V86_SYM_UNRESOLVED (̃0L)

struct v86inst {
    unsigned char *mnemo;
    size_t         mnemolen;
};

struct v86asm {
    long          *(*gencode)(struct v86vm *vm, struct v86op *op);
    struct v86sym *(*findsym)(struct v86vm *vm, char *name);
    struct v86sym *(*hashsym)(struct v86vm *vm, char *name, long adr);
    void          *(*delsym)(struct v86vm *vm, char *name);
};

struct v86sym {
    char          *name;
    size_t         namelen;
    long           adr;
    long           val;
    struct v86sym *prev;
    struct v86sym *next;
    struct v86sym *tabprev;
    struct v86sym *tabnext;
};

struct v86hashsym {
    char              *name;
    struct v86sym     *sym;
    struct v86hashsym *next;
};

#endif /* __V86_SYM_H__ */

