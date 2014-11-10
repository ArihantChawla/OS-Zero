#ifndef __ZVM_IO_H__
#define __ZVM_IO_H__

#include <stdio.h>
#include <zero/param.h>

#define zvmreadkbd(buf) scanf("%c", buf)

#define ZVMKBDBUF (PAGESIZE)

#define ZVMSTDIN  0 // standard input (keyboard)
#define ZVMSTDOUT 1 // standard output (console)
#define ZVMSTDERR 2 // standard error stream (console or file)
#define ZVMSTDFB  3 // graphics framebuffer address reads

#endif /* __ZVM_IO_H__ */

