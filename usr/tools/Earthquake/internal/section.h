/*
 * section.h - code section management for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_INTERNAL_SECTION_H
#define EARTHQUAKE_INTERNAL_SECTION_H

struct __Eccsection {
    struct __Ecctoken *firsttok;
    struct __Ecctoken *lasttok;
};

struct __Eccsections {
    /* standard segments. */
    struct __Eccsection *text;
    struct __Eccsection *data;
    struct __Eccsection *bss;
    struct __Eccsection *symtab;
    struct __Eccsection *strtab;
    /* debug segments. */
    struct __Eccsection *stab;
    struct __Eccsection *stabstr;
};

#endif /* EARTHQUAKE_INTERNAL_SECTION_H */

