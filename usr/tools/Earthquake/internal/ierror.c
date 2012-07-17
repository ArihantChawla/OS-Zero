/*
 * ierror.c - error handling routines for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#include <stdio.h>

#include <main.h>

static const char *__Eerrtbl[] =
{
    NULL,
    "failed to allocate memory",
    "unexpected '}'",
    "unexpected end of file",
    "invalid token",
    "unexpected token",
    "invalid constant",
    "invalid include statement"
};

void
__Eprterror(struct __Eerror *error)
{
    unsigned long code;

    code = error->code;
    fprintf(stderr, "%s: %s; %s\n", error->prog, __Eerrtbl[code], error->str);

    return;
}

