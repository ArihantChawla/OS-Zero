/*
 * opt.h - command-line options for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_INTERNAL_OPT_H
#define EARTHQUAKE_INTERNAL_OPT_H

int __Eparseopts(int argc, char *argv[]);
int __Eparsefopt(uint8_t *optstr, char *progname);
int __Eparsemopt(uint8_t *optstr, char *progname);

struct __Eoptdesc {
    uint8_t *str;
    const uint8_t *desc;
};

/* 4 machine words. */
struct __Einputopts {
    unsigned long ansi;
    unsigned long nobuiltin;
    unsigned long nostdinc;
    unsigned long nostdlib;
};

/* 8 machine words. */
struct __Eoutputopts {
    unsigned long arch;
    unsigned long format;
    unsigned long preproc;
    unsigned long assemble;
    unsigned long debug;
    unsigned long nolink;
    unsigned long profile;
    unsigned long chkbounds;
};

/* 16 machine words. */
struct __Eoptimopts {
    unsigned long optimize;
    unsigned long alignfuncs;
    unsigned long alignjumps;
    unsigned long alignlabels;
    unsigned long alignloops;
    unsigned long alignstructs;
    unsigned long alignunions;
    unsigned long noinline;
    unsigned long forcememreg;
    unsigned long forceaddrreg;
    unsigned long omitfp;
    unsigned long fastmath;
    unsigned long regmove;
    unsigned long longdbl128;
};

struct __Edebugopts {
    unsigned long stats;
    unsigned long profile;
    unsigned long dump;
};

struct __Eopts {
    struct __Einputopts input;
    struct __Eoutputopts output;
    struct __Eoptimopts optim;
    struct __Edebugopts debug;
};

#endif /* EARTHQUAKE_INTERNAL_OPT_H */

