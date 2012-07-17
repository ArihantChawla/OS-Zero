/*
 * iutil.c - internal routines for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#define FALSE 0
#define TRUE  1

#include <zero/cdecl.h>
#include <zero/param.h>

#include <main.h>

struct __Erun __Erun ALIGNED(PAGESIZE);

unsigned long __Eesctbl[WORD8_MAX + 1] ALIGNED(PAGESIZE) =
{
    0,    /* NUL '\0' */
    0,    /* SOH */
    0,    /* STX */
    0,    /* ETX */
    0,    /* EOT */
    0,    /* ENQ */
    0,    /* ACK */
    0,    /* BEL '\a' */
    0,    /* BS  '\b' */
    0,    /* HT  '\t' */
    0,    /* LF  '\n' */
    0,    /* VT  '\v' */
    0,    /* FF  '\f' */
    0,    /* CR  '\r' */
    0,    /* SO */
    0,    /* SI */
    0,    /* DLE */
    0,    /* DC1 */
    0,    /* DC2 */
    0,    /* DC3 */
    0,    /* DC4 */
    0,    /* NAK */
    0,    /* SYN */
    0,    /* ETB */
    0,    /* CAN */
    0,    /* EM */
    0,    /* SUB */
    0,    /* ESC */
    0,    /* FS */
    0,    /* GS */
    0,    /* RS */
    0,    /* US */
    0,    /* ' ' */
    0,    /* '!' */
    TRUE, /* '\"' */
    0,    /* '#' */
    0,    /* '$' */
    0,    /* '%' */
    0,    /* '&' */
    TRUE, /* '\'' */
    0,    /* '(' */
    0,    /* ')' */
    0,    /* '*' */
    0,    /* '+' */
    0,    /* ',' */
    0,    /* '-' */
    0,    /* '.' */
    0,    /* '/' */
    TRUE, /* '0' */
    TRUE, /* '1' */
    TRUE, /* '2' */
    TRUE, /* '3' */
    TRUE, /* '4' */
    TRUE, /* '5' */
    TRUE, /* '6' */
    TRUE, /* '7' */
    0,    /* '8' */
    0,    /* '9' */
    0,    /* ':' */
    0,    /* ';' */
    0,    /* '<' */
    0,    /* '=' */
    0,    /* '>' */
    TRUE, /* '\?' */
    0,    /* '@' */
    0,    /* 'A' */
    0,    /* 'B' */
    0,    /* 'C' */
    0,    /* 'D' */
    0,    /* 'E' */
    0,    /* 'F' */
    0,    /* 'G' */
    0,    /* 'H' */
    0,    /* 'I' */
    0,    /* 'J' */
    0,    /* 'K' */
    0,    /* 'L' */
    0,    /* 'M' */
    0,    /* 'N' */
    0,    /* 'O' */
    0,    /* 'P' */
    0,    /* 'Q' */
    0,    /* 'R' */
    0,    /* 'S' */
    0,    /* 'T' */
    0,    /* 'U' */
    0,    /* 'V' */
    0,    /* 'W' */
    TRUE, /* 'X' */
    0,    /* 'Y' */
    0,    /* 'Z' */
    0,    /* '[' */
    TRUE, /* '\\' */
    0,    /* ']' */
    0,    /* '^' */
    0,    /* '_' */
    0,    /* '`' */
    TRUE, /* 'a' */
    TRUE, /* 'b' */
    0,    /* 'c' */
    0,    /* 'd' */
    0,    /* 'e' */
    TRUE, /* 'f' */
    0,    /* 'g' */
    0,    /* 'h' */
    0,    /* 'i' */
    0,    /* 'j' */
    0,    /* 'k' */
    0,    /* 'l' */
    0,    /* 'm' */
    TRUE, /* 'n' */
    0,    /* 'o' */
    0,    /* 'p' */
    0,    /* 'q' */
    TRUE, /* 'r' */
    0,    /* 's' */
    TRUE, /* 't' */
    0,    /* 'u' */
    TRUE, /* 'v' */
    0,    /* 'w' */
    TRUE, /* 'x' */
    0,    /* 'y' */
    0,    /* 'z' */
    0,    /* '{' */
    0,    /* '|' */
    0,    /* '}' */
    0,    /* '~' */
    0     /* DEL */
};

unsigned long __Eseptbl[128] ALIGNED(PAGESIZE) =
{
    0,    /* NUL '\0' */
    0,    /* SOH */
    0,    /* STX */
    0,    /* ETX */
    0,    /* EOT */
    0,    /* ENQ */
    0,    /* ACK */
    0,    /* BEL '\a' */
    0,    /* BS  '\b' */
    0,    /* HT  '\t' */
    0,    /* LF  '\n' */
    0,    /* VT  '\v' */
    0,    /* FF  '\f' */
    0,    /* CR  '\r' */
    0,    /* SO */
    0,    /* SI */
    0,    /* DLE */
    0,    /* DC1 */
    0,    /* DC2 */
    0,    /* DC3 */
    0,    /* DC4 */
    0,    /* NAK */
    0,    /* SYN */
    0,    /* ETB */
    0,    /* CAN */
    0,    /* EM */
    0,    /* SUB */
    0,    /* ESC */
    0,    /* FS */
    0,    /* GS */
    0,    /* RS */
    0,    /* US */
    0,    /* ' ' */
    0,    /* '!' */
    TRUE, /* '\"' */
    0,    /* '#' */
    0,    /* '$' */
    0,    /* '%' */
    0,    /* '&' */
    TRUE, /* '\'' */
    TRUE, /* '(' */
    TRUE, /* ')' */
    0,    /* '*' */
    0,    /* '+' */
    0,    /* ',' */
    0,    /* '-' */
    0,    /* '.' */
    0,    /* '/' */
    0,    /* '0' */
    0,    /* '1' */
    0,    /* '2' */
    0,    /* '3' */
    0,    /* '4' */
    0,    /* '5' */
    0,    /* '6' */
    0,    /* '7' */
    0,    /* '8' */
    0,    /* '9' */
    0,    /* ':' */
    TRUE, /* ';' */
    0,    /* '<' */
    0,    /* '=' */
    0,    /* '>' */
    0,    /* '\?' */
    0,    /* '@' */
    0,    /* 'A' */
    0,    /* 'B' */
    0,    /* 'C' */
    0,    /* 'D' */
    0,    /* 'E' */
    0,    /* 'F' */
    0,    /* 'G' */
    0,    /* 'H' */
    0,    /* 'I' */
    0,    /* 'J' */
    0,    /* 'K' */
    0,    /* 'L' */
    0,    /* 'M' */
    0,    /* 'N' */
    0,    /* 'O' */
    0,    /* 'P' */
    0,    /* 'Q' */
    0,    /* 'R' */
    0,    /* 'S' */
    0,    /* 'T' */
    0,    /* 'U' */
    0,    /* 'V' */
    0,    /* 'W' */
    0,    /* 'X' */
    0,    /* 'Y' */
    0,    /* 'Z' */
    0,    /* '[' */
    TRUE, /* '\\' */
    0,    /* ']' */
    0,    /* '^' */
    0,    /* '_' */
    0,    /* '`' */
    0,    /* 'a' */
    0,    /* 'b' */
    0,    /* 'c' */
    0,    /* 'd' */
    0,    /* 'e' */
    0,    /* 'f' */
    0,    /* 'g' */
    0,    /* 'h' */
    0,    /* 'i' */
    0,    /* 'j' */
    0,    /* 'k' */
    0,    /* 'l' */
    0,    /* 'm' */
    0,    /* 'n' */
    0,    /* 'o' */
    0,    /* 'p' */
    0,    /* 'q' */
    0,    /* 'r' */
    0,    /* 's' */
    0,    /* 't' */
    0,    /* 'u' */
    0,    /* 'v' */
    0,    /* 'w' */
    0,    /* 'x' */
    0,    /* 'y' */
    0,    /* 'z' */
    TRUE, /* '{' */
    0,    /* '|' */
    TRUE, /* '}' */
    0,    /* '~' */
    0     /* DEL */
};

int
__Einit(int argc, char *argv[])
{
    int retval;

    __Eccinitkeywords();
    retval = __Eparseopts(argc, argv);
    if (retval < 0) {

	return -1;
    }

    return 0;
}

int
__Eaddinfile(uint8_t *name)
{
    struct __Efile *newfile;
    struct __Efile *lastfile;

    newfile = __Eallocfile(name, TRUE);
    if (newfile == NULL) {

	return -1;
    }
    lastfile = __Erun.files.lastin;
    if (lastfile) {
	lastfile->next = newfile;
    } else {
	__Erun.files.infiles = newfile;
    }
    __Erun.files.lastin = newfile;

    return 0;
}

