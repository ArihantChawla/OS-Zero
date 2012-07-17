/*
 * type.h - internal type management for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_INTERNAL_TYPE_H
#define EARTHQUAKE_INTERNAL_TYPE_H

#define __ECC_TYPE_NAME_MAX   128  /* default type name array size */
#define __ECC_TYPE_HASH_ITEMS 1024 /* must be power of two. */
#if (!powerof2(__ECC_TYPE_HASH_ITEMS))
#   error __ECC_TYPE_HASH_ITEMS must be power of two.
#endif
extern struct __Ecctype *__Ecctypehash[__ECC_TYPE_HASH_ITEMS];

/*
 * Compiler type.
 */
struct __Ecctype {
    unsigned long type;
    uint8_t *name;
    struct __Ecctype *chain;
    size_t size;
};

void __Ecchashtype(struct __Ecctype *type);
struct __Ecctype * __Eccfindtype(uint8_t *name);

#endif /* EARTHQUAKE_INTERNAL_TYPE_H */

