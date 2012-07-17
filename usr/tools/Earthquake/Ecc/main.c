/*
 * main.c - main routine for Lightning Experimental C Compiler.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#include <stdio.h>
#include <stdint.h>

#include <main.h>
#include <Ecc/main.h>

int
main(int argc, char *argv[])
{
    unsigned long ndx;
    uint8_t *optstr;
    int retval;

    for (ndx = 1 ; ndx < argc ; ndx++) {
	optstr = argv[ndx];
	if (!strcmp(optstr, "-v")) {
	    fprintf(stdout, "Earthquake Experimental C Compiler %s.%s.%s\n",
		    __Eccitos(__ECC_MAJOR__),
		    __Eccitos(__ECC_MINOR__),
		    __Eccitos(__ECC_PATCHLEVEL__));

	    return 0;
	}
    }
    retval = Einit(argc, argv);

    return retval;
}

