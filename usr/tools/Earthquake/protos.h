/*
 * protos.h - prototypes for Earthquake C Environment.
 * Copyright (C) Tuomo Venäläinen 2004
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_PROTOS_H
#define EARTHQUAKE_PROTOS_H

/*
 * asm.c
 */
int asmmain(int argc, char *argv[]);

/*
 * cc.c
 */
int Eccmain(int argc, char *argv[]);
int Eccrun(void);

/*
 * cpp.c
 */
int Ecppmain(int argc, char *argv[]);
int Ecpprun(void);


#endif /* EARTHQUAKE_PROTOS_H */

