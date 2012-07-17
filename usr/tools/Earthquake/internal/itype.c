/*
 * itype.c - internal type management routines for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#include <zero/cdecl.h>
#include <zero/param.h>

#include <main.h>

struct __Ecctype *__Ecctypehash[__ECC_TYPE_HASH_ITEMS] ALIGNED(PAGESIZE);

void
__Ecchashtype(struct __Ecctype *type)
{
    unsigned long key;

    key = __Ehashstr(type->name, __ECC_TYPE_HASH_ITEMS);
    type->chain = __Ecctypehash[key];
    __Ecctypehash[key] = type;

    return;
}

struct __Ecctype *
__Eccfindtype(uint8_t *name)
{
    unsigned long key;
    struct __Ecctype *type;

    key = __Ehashstr(name, __ECC_TYPE_HASH_ITEMS);
    type = __Ecctypehash[key];
    while (type) {
	if (!strcmp(type->name, name)) {

	    return type;
	}
	type = type->chain;
    }

    return NULL;
}

