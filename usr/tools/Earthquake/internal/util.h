/*
 * util.h - internal utilities for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_INTERNAL_UTIL_H
#define EARTHQUAKE_INTERNAL_UTIL_H

#include <zero/cdecl.h>
#include <zero/param.h>

extern unsigned long __Eesctbl[];
extern unsigned long __Eseptbl[];

struct __Efiles {
    struct __Efile *infiles;
    struct __Efile *outfile;
    struct __Efile *lastin;
    struct __Efile *logfile;
};

struct __Epaths {
    uint8_t *header;
    uint8_t *library;
};

struct __Estats {
    unsigned long funccnt;
    unsigned long macrocnt;
    unsigned long bytecnt;
    unsigned long tokcnt;
};

struct __Erun {
    struct __Efiles files ALIGNED(CLSIZE);
    struct __Epaths paths ALIGNED(CLSIZE);
    struct __Eopts opts ALIGNED(CLSIZE);
    struct __Eerror error ALIGNED(CLSIZE);
    struct __Estats stats ALIGNED(CLSIZE);
    struct __Ecpp cpp ALIGNED(CLSIZE);
    struct __Ecc cc ALIGNED(CLSIZE);
};

extern struct __Erun __Erun;

int __Einit(int argc, char *argv[]);
int __Eaddinfile(uint8_t *name);

#endif /* EARTHQUAKE_INTERNAL_UTIL_H */

