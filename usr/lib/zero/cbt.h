#include <stdint.h>

struct cbtnode {
    unsigned long   weight;     // number of accesses
    uintptr_t       key;        // key value
    uintptr_t       data;       // data
    struct cbtnode *parent;     // parent node
    struct cbtnode *left;       // left child
    struct cbtnode *right;      // right childe
};

struct cbt {
    struct cbtnode  root;
    struct cbtnode *left;
    struct cbtnode *right;
};
