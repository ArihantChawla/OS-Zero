/*
 * cpp.c - preprocessor routines for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#include <main.h>

#define FALSE 0
#define TRUE  1

/*
 * Main compiler routine.
 */
int
Ecppmain(int argc, char *argv[])
{
    int retval;

    retval = __Einit(argc, argv);
    if (retval < 0) {

	return -1;
    }
    retval = Ecpprun();
 
    return retval;
}

int
Ecpprun(void)
{
    uint8_t *filename;
    struct __Efile *file;
    struct __Efile *infile;
    int retval;

    file = __Erun.files.infiles;
    retval = -1;
    while (file) {
	filename = file->name;
	infile = __Eopen(filename, FALSE, TRUE);
	if (infile == NULL) {

	    return -1;
	}
	if (__Ecppstage1(infile) == NULL) {
	    __Eclose(infile);

	    return -1;
	}
	file = file->next;
    }

    return 0;
}

