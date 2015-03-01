#ifndef __ZERO_ROPE_H__
#define __ZERO_ROPE_H__

struct ropeleaf {
    void *str;  // character string
};

struct ropeconcat {
    void *left;         // left subtree
    void *right;        // right subtree
};

struct ropefunc {
    void *ptr;
};

struct ropesubstr {
    void *base;
};

/* node types */
#define ROPE_UNUSED   0x00
#define ROPE_LEAF     0x01
#define ROPE_CONCAT   0x02
#define ROPE_FUNCTION 0x03
#define ROPE_SUBSTR   0x04
/* flag-bit for status-field */
#define ROPE_BALANCED 0x08
struct rope {
    volatile long lk;
    volatile long nref;
    size_t        len;
    uint_fast8_t  depth;
    uint_fast8_t  status;
    union {
        struct ropeleaf   leaf;
        struct ropeconcat concat;
        struct ropefunc   func;
        struct ropesubstr substr;
    } data;
};

#endif /* __ZERO_ROPE_H__ */

