/*
 * ct.c - generic routines for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#include <main.h>

int
Einit(int argc, char *argv[])
{
    int retval;

    retval = __Einit(argc, argv);

    return retval;
}

