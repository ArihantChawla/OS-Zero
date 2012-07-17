/*
 * ihash.c - internal hashing for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#include <main.h>

unsigned long
__Ehashstr(uint8_t *str, unsigned long nhashitems)
{
    unsigned long key;

    key = 0;
    while (*str) {
	key += *str++;
    }

    return (key & (nhashitems - 1));
}

