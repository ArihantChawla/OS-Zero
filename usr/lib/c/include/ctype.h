#ifndef __CTYPE_H__
#define __CTYPE_H__

#include <features.h>
#include <stdint.h>
#if (_GNU_SOURCE) && 0
/* reentrant locale functions */
#include <xlocale.h>
#endif

extern volatile uint8_t *ctypeparamptr;

#include <bits/ctype.h>

#define CHARISALNUM      0x01
#define CHARISALPHA      0x02
#define CHARISGRAPH      0x04
#define CHARISLOWER      0x08
#define CHARISPRINT      0x10
#define CHARISPUNCT      0x20
#define CHARISSPACE      0x40
#define CHARISUPPER      0x80
#define isalnum(c)       (ctypeparamptr[(c)] & CHARISALNUM)
#define isalpha(c)       (ctypeparamptr[(c)] & CHARISALPHA)
#if (_XOPEN_SOURCE) || (USESVID)
#define isascii(c)       (!((c) & 0x80))
#endif
#if (_ISOC99_SOURCE)
#define isblank(c)       (((c) == ' ') || ((c) == '\t'))
#endif
#define iscntrl(c)       ((c) & 0x1f)
#define isdigit(c)       (((c) >= '0') && ((c) <= '9'))
#define isgraph(c)       (ctypeparamptr[(c)] & CHARISGRAPH)
#define islower(c)       (ctypeparamptr[(c)] & CHARISLOWER)
#define isprint(c)       (ctypeparamptr[(c)] & CHARISPRINT)
#define ispunct(c)       (ctypeparamptr[(c)] & CHARISPUNCT)
#define isspace(c)       (ctypeparamptr[(c)] & CHARISSPACE)
#define isupper(c)       (ctypeparamptr[(c)] & CHARISUPPER)
#define isxdigit(c)      (isdigit(c) || (tolower(c) >= 'a' && tolower(c) <= 'f'))
#if (_XOPEN_SOURCE) || (USESVID)
#define toascii(c)       ((c) & 0x7f)
#define tolower(c)       (ctypelowptr[(c)])
#define toupper(c)       (ctypeupptr[(c)])
#define _toupper(c)      ((c) - ('a' - 'A'))
#define _tolower(c)      ((c) + ('a' - 'A'))
#endif

#endif /* __CTYPE_H__ */

