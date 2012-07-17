/*
 * cc.c - compiler routines for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#include <main.h>

/*
 * Main compiler routine.
 */
int
Eccmain(int argc, char *argv[])
{
    int retval;

    retval = Einit(argc, argv);
    if (retval < 0) {

	return -1;
    }
    retval = Ecpprun();
    if (retval < 0) {

	return -1;
    }
    retval = Eccrun();
    if (retval < 0) {

	return -1;
    }

    return 0;
}

int
Eccrun(void)
{
    return -1;
}

