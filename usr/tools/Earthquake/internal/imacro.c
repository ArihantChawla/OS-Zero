/*
 * imacro.c - internal macro management routines for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#include <string.h>

#include <zero/cdecl.h>
#include <zero/param.h>

#include <main.h>

struct __Ecppmacro *__Ecppmacrohash[__ECPP_MACRO_HASH_ITEMS] ALIGNED(PAGESIZE);

void
__Ecpphashmacro(struct __Ecppmacro *macro)
{
    unsigned long key;

    key = __Ehashstr(macro->name, __ECPP_MACRO_HASH_ITEMS);
    macro->chain = __Ecppmacrohash[key];
    __Ecppmacrohash[key] = macro;

    return;
}

struct __Ecppmacro *
__Ecppfindmacro(uint8_t *name)
{
    unsigned long key;
    struct __Ecppmacro *macro;

    key = __Ehashstr(name, __ECPP_MACRO_HASH_ITEMS);
    macro = __Ecppmacrohash[key];
    while (macro) {
	if (!strcmp(macro->name, name)) {

	    return macro;
	}
	macro = macro->chain;
    }

    return NULL;
}

