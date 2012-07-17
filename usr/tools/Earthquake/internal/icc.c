/*
 * icc.c - internal compiler routines for Lightning C Tool Library.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#include <stdlib.h>

#include <zero/cdecl.h>
#include <zero/param.h>

#define FALSE 0
#define TRUE  1

#include <main.h>

static void __Ecchashkeyword(struct __Ecckeyword *keyword);

unsigned long __Eccoptbl[WORD8_MAX + 1] ALIGNED(PAGESIZE) =
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
    TRUE, /* '!' */
    0,    /* '\"' */
    0,    /* '#' */
    0,    /* '$' */
    TRUE, /* '%' */
    TRUE, /* '&' */
    0,    /* '\'' */
    0,    /* '(' */
    0,    /* ')' */
    TRUE, /* '*' */
    TRUE, /* '+' */
    TRUE, /* ',' */
    TRUE, /* '-' */
    TRUE, /* '.' */
    TRUE, /* '/' */
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
    TRUE, /* ':' */
    0,    /* ';' */
    TRUE, /* '<' */
    TRUE, /* '=' */
    TRUE, /* '>' */
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
    0,    /* 'X' */
    0,    /* 'Y' */
    0,    /* 'Z' */
    TRUE, /* '[' */
    0,    /* '\\' */
    TRUE, /* ']' */
    TRUE, /* '^' */
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
    0,    /* '{' */
    TRUE, /* '|' */
    0,    /* '}' */
    TRUE, /* '~' */
    0     /* DEL */
};

static struct __Ecckeyword *__Ecckeywordhash[__ECC_KEYWORD_HASH_ITEMS] ALIGNED(PAGESIZE);

static struct __Ecckeyword __Eccansikeywords[] ALIGNED(PAGESIZE) =
{
    /* ANSI C89 keywords. */
    { "auto", 0, NULL, 0 },
    { "break", 0, NULL, 0 },
    { "case", 0, NULL, 0 },
    { "char", __ECC_CHAR_TYPE, NULL, 0 },
    { "const", __ECC_CONST_FLAG, NULL, 0 },
    { "continue", 0, NULL, 0 },
    { "default", 0, NULL, 0 },
    { "do", 0, NULL, 0 },
    { "double", __ECC_DOUBLE_TYPE, NULL, 0 },
    { "else", 0, NULL, 0 },
    { "enum", __ECC_ENUM_TYPE, NULL, 0 },
    { "extern", __ECC_EXTERN_FLAG, NULL, 0 },
    { "float", __ECC_FLOAT_TYPE, NULL, 0 },
    { "for", 0, NULL, 0 },
    { "goto", 0, NULL, 0 },
    { "if", 0, NULL, 0 },
    { "int", __ECC_INT_TYPE, NULL, 0 },
    { "long", __ECC_LONG_TYPE, NULL, 0 },
    { "register", __ECC_REGISTER_FLAG, NULL, 0 },
    { "return", 0, NULL, 0 },
    { "short", __ECC_SHORT_TYPE, NULL, 0 },
    { "signed", 0, NULL, 0 },
    { "sizeof", 0, NULL, 0 },
    { "static", __ECC_STATIC_FLAG, NULL, 0 },
    { "struct", __ECC_STRUCT_TYPE, NULL, 0 },
    { "switch", 0, NULL, 0 },
    { "typedef", __ECC_TYPEDEF_FLAG, NULL, 0 },
    { "union", __ECC_UNION_TYPE, NULL, 0 },
    { "unsigned", __ECC_UNSIGNED_FLAG, NULL, 0 },
    { "void", __ECC_VOID_TYPE, NULL, 0 },
    { "volatile", __ECC_VOLATILE_FLAG, NULL, 0 },
    { "while", 0, NULL, 0 },
    /* <FIXME> I wonder if __asm__ and __inline__ belong here? </FIXME> */
    { "__asm__", __ECC_ASM_FLAG, NULL, 0 },
    { "__inline__", __ECC_INLINE_FLAG, NULL, 0 },
    { NULL, 0, NULL, 0 }
};

static struct __Ecckeyword __Eccnonansikeywords[] ALIGNED(PAGESIZE) =
{
    /* Other keywords. */
    { "asm", __ECC_ASM_FLAG, NULL, 0 },
    { "inline", __ECC_INLINE_FLAG, NULL, 0 },
    { "__CC_ALIGN__", __ECC_ALIGNED_FLAG, NULL, 0 },
    { "__CC_CACHE_ALIGN__", __ECC_ALIGNED_FLAG, NULL, 0 },
    { "__CC_PAGE_ALIGN__", __ECC_ALIGNED_FLAG, NULL, 0 },
    { NULL, 0, NULL, 0 }
};

/*
 * Initialize keywords.
 */
void
__Eccinitkeywords(void)
{
    unsigned long ndx;
    uint8_t *name;
    struct __Ecckeyword *keyword;

    ndx = 0;
    keyword = &__Eccansikeywords[ndx];
    name = keyword->name;
    while (name) {
	__Ecchashkeyword(keyword);
	ndx++;
	keyword = &__Eccansikeywords[ndx];
	name = keyword->name;
    }
    if (!__Erun.opts.input.ansi) {
	ndx = 0;
	keyword = &__Eccnonansikeywords[ndx];
	name = keyword->name;
	while (name) {
	    __Ecchashkeyword(keyword);
	    ndx++;
	    keyword = &__Eccnonansikeywords[ndx];
	    name = keyword->name;
	}
    }

    return;
}

static void
__Ecchashkeyword(struct __Ecckeyword *keyword)
{
    unsigned long key;

    key = __Ehashstr(keyword->name, __ECC_KEYWORD_HASH_ITEMS);
    keyword->chain = __Ecckeywordhash[key];
    __Ecckeywordhash[key] = keyword;

    return;
}

struct __Ecckeyword *
__Eccfindkeyword(uint8_t *name)
{
    unsigned long key;
    struct __Ecckeyword *keyword;

    key = __Ehashstr(name, __ECC_KEYWORD_HASH_ITEMS);
    keyword = __Ecckeywordhash[key];
    while (keyword) {
	if (!strcmp(keyword->name, name)) {

	    return keyword;
	}
	keyword = keyword->chain;
    }

    return NULL;
}

struct __Ecctoken *
__Eccparse(void)
{
    unsigned long errcode;
    struct __Ecpptoken *cpptok;
    struct __Ecctoken *newtok;
    struct __Ecctoken *lasttok;

    cpptok = __Erun.cpp.parse.firsttok;
    if (cpptok == NULL) {

	return NULL;
    }
    newtok = __Eccparsetok(cpptok);
    cpptok = cpptok->next;
    lasttok = newtok;
    while ((cpptok) && (newtok)) {
	newtok = __Eccparsetok(cpptok);
	cpptok = cpptok->next;
	lasttok->next = newtok;
	lasttok = newtok;
    }
    errcode = __Erun.error.code;
    if (errcode) {
	__Eprterror(&__Erun.error);

	return NULL;
    }
    
    return newtok;
}

struct __Ecctoken *
__Eccparsetok(struct __Ecpptoken *tok)
{
    unsigned long type;
    uint8_t *name;
    struct __Ecpptoken *cpptok;
    struct __Ecctoken *newtok;
    struct __Ecckeyword *cckeyword;

    cpptok = tok;
    newtok = NULL;
    if (cpptok == NULL) {

	return NULL;
    }
    type = cpptok->type;
    if (type == __ECPP_IDENTIFIER_TOKEN) {
	newtok = __Eccparseid(cpptok);
    } else if (type == __ECPP_KEYWORD_TOKEN) {
	name = cpptok->name;
	cckeyword = __Eccfindkeyword(name);
    } else if (type == __ECPP_CONSTANT_TOKEN) {
	newtok = __Eccparseconst(cpptok);
    } else if (type == __ECPP_OPERATOR_TOKEN) {
	newtok = __Eccparseop(cpptok);
    } else if (type == __ECPP_STRING_TOKEN) {
	newtok = __Eccparsestr(cpptok);
    } else if (type == __ECPP_SEPARATOR_TOKEN) {
	;
    }
    __Ecppfreetok(cpptok);

    return newtok;
}

struct __Ecctoken *
__Eccparseid(struct __Ecpptoken *cpptok)
{
    uint8_t *name;
    uint8_t *u8ptr;
    struct __Ecctoken *newtok;
    size_t namelen;
    size_t namesize;

    name = cpptok->name;
    namelen = cpptok->namelen;
    namesize = __ECC_TOKEN_NAME_MAX;
    newtok = __Eccalloctok(TRUE);
    if (newtok == NULL) {

	return NULL;
    }
    if (namelen > namesize - 1) {
	while (namelen > namesize - 1) {
	    namesize <<= 1;
	}
	name = __Eccreallocname(newtok, namesize);
    }
    u8ptr = newtok->name;
    memcpy(u8ptr, name, namelen + 1);
    newtok->type = __ECC_IDENTIFIER_TOKEN;
    newtok->namelen = namelen;

    return newtok;
}

struct __Ecctoken *
__Eccparseconst(struct __Ecpptoken *cpptok)
{
    unsigned long valtype;
    uint8_t *u8ptr;
    struct __Ecctoken *newtok;
    int c;

    valtype = 0;
    u8ptr = cpptok->name;
    newtok = NULL;
    c = *u8ptr++;
    if (c) {
	if (c == '0') {
	    c = *u8ptr;
	    if ((c == 'x') || (c == 'X')) {
		*u8ptr++;
		newtok = __Eccparsepow2(u8ptr, __EARTHQUAKE_LOG2_HEX);
	    } else {
		newtok = __Eccparsepow2(u8ptr, __EARTHQUAKE_LOG2_OCT);
	    }
	} else if (__Eisdigit(c)) {
	    newtok = __Eccparsedec(u8ptr);
	} else if (c == 'L') {
	    valtype |= __ECC_WIDE_FLAG;
	    c = *u8ptr++;
	}
	if (__Eischconstsep(c)) {
	    if (valtype == 0) {
		valtype |= __ECC_CHAR_TYPE;
	    }
	    newtok = __Eccalloctok(FALSE);
	    if (newtok == NULL) {

		return NULL;
	    }
	    c = *u8ptr++;
	    if (__Eisescpre(c)) {
		c = __Ecppparseesc(u8ptr);
		if (c < 0) {
		    __Erun.error.code = __EARTHQUAKE_UNEXPECTED_TOKEN;

		    return NULL;
		}
	    }
	    valtype |= __ECC_CONST_FLAG;
	    if ((valtype & ~__ECC_TYPE_MASK) == __ECC_WIDE_FLAG) {
		if (c > WCHAR_MAX) {
		    __Erun.error.code = __EARTHQUAKE_INVALID_TOKEN;

		    return NULL;
		}
		newtok->objsize = sizeof(wchar_t);
		newtok->value.wc = c;
	    } else {
		if (c > CHAR_MAX) {
		    if (c > UCHAR_MAX) {
			__Erun.error.code = __EARTHQUAKE_INVALID_TOKEN;
			
			return NULL;
		    }
		    valtype |= __ECC_UNSIGNED_FLAG;
		    newtok->objsize = sizeof(unsigned char);
		    newtok->value.uc = (unsigned char)c;
		} else {
		    newtok->objsize = sizeof(char);
		    newtok->value.c = (char)c;
		}
	    }
	    newtok->valtype = valtype;
	} else {
	    __Erun.error.code = __EARTHQUAKE_UNEXPECTED_TOKEN;

	    return NULL;
	}
    }
    newtok->type = __ECC_CONSTANT_TOKEN;

    return newtok;
}

/*
 * Parse octal and hexadecimal integer and floating-point constants. Note that
 * we use unsigned long long and long double for better resolution.
 */
struct __Ecctoken *
__Eccparsepow2(uint8_t *str, unsigned long log2)
{
    long double ld;
    long double ldpow;
    unsigned long long ull;
    unsigned long valtype;
    unsigned long shift;
    unsigned long mul;
    unsigned long divres;
    unsigned long suf;
    uint8_t *u8ptr;
    struct __Ecctoken *newtok;
    size_t size;
    uint8_t u8;

    shift = log2;
    if ((shift != __EARTHQUAKE_LOG2_HEX) && (shift != __EARTHQUAKE_LOG2_OCT)) {

	return NULL;
    }

    ld = 0;
    ull = 0;
    valtype = 0;
    mul = 1 << shift;
    divres = 0;
    suf = FALSE;
    u8ptr = str;
    newtok = NULL;
    size = 0;
    u8 = *u8ptr++;
    if (u8) {
	while (!__Eisaltfltexppre(u8)) {
	    if (u8 == '.') {
		if (__Erun.opts.input.ansi) {
		    __Erun.error.code = __EARTHQUAKE_INVALID_CONSTANT;

		    return NULL;
		}
		ld = (long double)ull;
		divres = 1U << shift;
	    } else if (suf == FALSE) {
		if (ld) {
		    if (__Eisfltsuf(u8)) {
			suf = TRUE;
		    }
		} else if (__Eisisuf(u8)) {
		    suf = TRUE;
		}
	    }
	    if (suf) {
		switch (u8) {
		    case 'u':
		    case 'U':
			if (ld) {
			    
			    return NULL;
			} else if (valtype & __ECC_UNSIGNED_FLAG) {
			    
			    return NULL;
			}
			valtype |= __ECC_UNSIGNED_FLAG;
			
			break;
		    case 'l':
		    case 'L':
			if (valtype == 0) {
			    if (ld) {
				if (valtype == __ECC_LONG_DOUBLE_TYPE) {
				    
				    return NULL;
				}
				valtype = __ECC_LONG_DOUBLE_TYPE;
			    } else {
				valtype = __ECC_LONG_TYPE;
			    }
			} else if (valtype == __ECC_LONG_TYPE) {
			    valtype &= ~__ECC_TYPE_MASK;
			    valtype |= __ECC_LONG_LONG_TYPE;
			} else {
			    
			    return NULL;
			}
			
			break;
		}
	    } else {
		if (shift == __EARTHQUAKE_LOG2_HEX) {
		    if (!__Eishexdigit(u8)) {
			__Erun.error.code = __EARTHQUAKE_INVALID_CONSTANT;

			return NULL;
		    }
		    if (u8 <= '9') {
			u8 -= '0';
		    } else if (u8 >= 'a') {
			u8 -= 87;
		    } else if (u8 >= 'A') {
			u8 -= 55;
		    }
		} else {
		    if (!__Eisoctdigit(u8)) {
			__Erun.error.code = __EARTHQUAKE_INVALID_CONSTANT;

			return NULL;
		    }
		    u8 -= '0';
		}
		if (ld) {
		    if (divres) {
			ld += (long double)u8 / divres;
			divres <<= shift;
		    } else {
			ld *= mul;
			ld += (long double)u8;
		    }
		} else {
		    ull <<= shift;
		    ull += u8;
		}
	    }
	    u8 = *u8ptr++;
	}
	if (__Eisaltfltexppre(u8)) {
	    if (!ld) {
		__Erun.error.code = __EARTHQUAKE_INVALID_TOKEN;

		return NULL;
	    }
	    u8 = *u8ptr++;
	    if (u8 == '-') {
		ldpow = -1.0;
	    } else if (u8 == '+') {
		ldpow = +1.0;
	    } else {
		u8 -= '0';
		ldpow = u8;
	    }
	    u8 = *u8ptr++;
	    while (u8) {
		u8 -= '0';
		ldpow *= 10;
		ldpow += u8;
		u8 = *u8ptr++;
	    }
	    ld = powl(ld, ldpow);
	}
	newtok = __Eccalloctok(FALSE);
	if (newtok == NULL) {

	    return NULL;
	}
	if (ld) {
	    if (valtype == 0) {
		valtype = __ECC_DOUBLE_TYPE;
		newtok->value.d = (double)ld;
		size = sizeof(double);
	    } else if (valtype == __ECC_LONG_DOUBLE_TYPE) {
		newtok->value.ld = ld;
		size = sizeof(long double);
	    } else {
		newtok->value.f = (float)ld;
		size = sizeof(float);
	    }
	} else if ((valtype & __ECC_TYPE_MASK) == 0) {
	    if (valtype & __ECC_UNSIGNED_FLAG) {
		if (ull <= UINT_MAX) {
		    valtype |= __ECC_INT_TYPE;
		} else if (ull <= ULONG_MAX) {
		    valtype |= __ECC_LONG_TYPE;
		}
	    } else if (ull <= INT_MAX) {
		valtype |= __ECC_INT_TYPE;
	    } else if (ull <= UINT_MAX) {
		valtype |= __ECC_INT_TYPE;
		valtype |= __ECC_UNSIGNED_FLAG;
	    } else if (ull <= LONG_MAX) {
		valtype |= __ECC_LONG_TYPE;
	    } else if (ull <= ULONG_MAX) {
		valtype |= __ECC_LONG_TYPE;
		valtype |= __ECC_UNSIGNED_FLAG;
	    }
	}
	if ((valtype & __ECC_TYPE_MASK) == __ECC_INT_TYPE) {
	    if (valtype & __ECC_UNSIGNED_FLAG) {
		newtok->value.ui = (unsigned int)ull;
		size = sizeof(unsigned int);
	    } else {
		newtok->value.i = (int)ull;
		size = sizeof(int);
	    }
	} else if ((valtype & __ECC_TYPE_MASK) == __ECC_LONG_TYPE) {
	    if (valtype & __ECC_UNSIGNED_FLAG) {
		newtok->value.ul = (unsigned long)ull;
		size = sizeof(unsigned long);
	    } else if (ull >= LONG_MAX) {
		valtype |= __ECC_UNSIGNED_FLAG;
		newtok->value.ul = (unsigned long)ull;
		size = sizeof(unsigned long);
	    } else {
		newtok->value.l = (long)ull;
		size = sizeof(long);
	    }
	} else {
	    if (valtype & __ECC_UNSIGNED_FLAG) {
		newtok->value.ull = ull;
		size = sizeof(unsigned long long);
	    } else {
		newtok->value.ll = (long long)ull;
		size = sizeof(unsigned long long);
	    }
	}
	newtok->valtype = valtype;
	newtok->objsize = size;
    } else {

	return NULL;
    }
    newtok->type = __ECC_CONSTANT_TOKEN;

    return newtok;
}

struct __Ecctoken *
__Eccparsedec(uint8_t *str)
{
    long double ld;
    long double ldpow;
    unsigned long long ull;
    unsigned long valtype;
    unsigned long divres;
    unsigned long suf;
    uint8_t *u8ptr;
    struct __Ecctoken *newtok;
    size_t size;
    uint8_t u8;

    ld = 0;
    ull = 0;
    valtype = 0;
    divres = 0;
    suf = FALSE;
    u8ptr = str;
    newtok = NULL;
    size = 0;
    u8 = *u8ptr++;
    if (u8) {
	while (!__Eisdecfltexppre(u8)) {
	    if (u8 == '.') {
		if (__Erun.opts.input.ansi) {
		    __Erun.error.code = __EARTHQUAKE_INVALID_CONSTANT;

		    return NULL;
		}
		ld = (long double)ull;
		divres = 10;
	    } else if (suf == FALSE) {
		if (ld) {
		    if (__Eisfltsuf(u8)) {
			suf = TRUE;
		    }
		} else if (__Eisisuf(u8)) {
		    suf = TRUE;
		}
	    }
	    if (suf) {
		switch (u8) {
		    case 'u':
		    case 'U':
			if (ld) {
			    
			    return NULL;
			} else if (valtype & __ECC_UNSIGNED_FLAG) {
			    
			    return NULL;
			}
			valtype |= __ECC_UNSIGNED_FLAG;
			
			break;
		    case 'l':
		    case 'L':
			if (valtype == 0) {
			    if (ld) {
				if (valtype == __ECC_LONG_DOUBLE_TYPE) {
				    
				    return NULL;
				}
				valtype = __ECC_LONG_DOUBLE_TYPE;
			    } else {
				valtype = __ECC_LONG_TYPE;
			    }
			} else if (valtype == __ECC_LONG_TYPE) {
			    valtype &= ~__ECC_TYPE_MASK;
			    valtype |= __ECC_LONG_LONG_TYPE;
			} else {
			    
			    return NULL;
			}
			
			break;
		}
	    } else {
		if (!__Eisdigit(u8)) {
		    __Erun.error.code = __EARTHQUAKE_INVALID_CONSTANT;
		    
		    return NULL;
		}
		u8 -= '0';
		if (ld) {
		    if (divres) {
			ld += (long double)u8 / divres;
			divres *= 10;
		    } else {
			ld *= 10;
			ld += (long double)u8;
		    }
		} else {
		    ull *= 10;
		    ull += u8;
		}
	    }
	    u8 = *u8ptr++;
	}
	if (__Eisdecfltexppre(u8)) {
	    if (!ld) {
		__Erun.error.code = __EARTHQUAKE_INVALID_TOKEN;

		return NULL;
	    }
	    u8 = *u8ptr++;
	    if (u8 == '-') {
		ldpow = -1.0;
	    } else if (u8 == '+') {
		ldpow = +1.0;
	    } else {
		u8 -= '0';
		ldpow = u8;
	    }
	    u8 = *u8ptr++;
	    while (u8) {
		u8 -= '0';
		ldpow *= 10;
		ldpow += u8;
		u8 = *u8ptr++;
	    }
	    ld = powl(ld, ldpow);
	}
	newtok = __Eccalloctok(FALSE);
	if (newtok == NULL) {

	    return NULL;
	}
	if (ld) {
	    if (valtype == 0) {
		valtype = __ECC_DOUBLE_TYPE;
		newtok->value.d = (double)ld;
		size = sizeof(double);
	    } else if (valtype == __ECC_LONG_DOUBLE_TYPE) {
		newtok->value.ld = ld;
		size = sizeof(long double);
	    } else {
		newtok->value.f = (float)ld;
		size = sizeof(float);
	    }
	} else if ((valtype & __ECC_TYPE_MASK) == 0) {
	    if (valtype & __ECC_UNSIGNED_FLAG) {
		if (ull <= UINT_MAX) {
		    valtype |= __ECC_INT_TYPE;
		} else if (ull <= ULONG_MAX) {
		    valtype |= __ECC_LONG_TYPE;
		}
	    } else if (ull <= INT_MAX) {
		valtype |= __ECC_INT_TYPE;
	    } else if (ull <= LONG_MAX) {
		valtype |= __ECC_LONG_TYPE;
	    } else if (ull <= ULONG_MAX) {
		valtype |= __ECC_LONG_TYPE;
		valtype |= __ECC_UNSIGNED_FLAG;
	    }
	}
	if ((valtype & __ECC_TYPE_MASK) == __ECC_INT_TYPE) {
	    if (valtype & __ECC_UNSIGNED_FLAG) {
		newtok->value.ui = (unsigned int)ull;
		size = sizeof(unsigned int);
	    } else {
		newtok->value.i = (int)ull;
		size = sizeof(int);
	    }
	} else if ((valtype & __ECC_TYPE_MASK) == __ECC_LONG_TYPE) {
	    if (valtype & __ECC_UNSIGNED_FLAG) {
		newtok->value.ul = (unsigned long)ull;
		size = sizeof(unsigned long);
	    } else if (ull >= LONG_MAX) {
		valtype |= __ECC_UNSIGNED_FLAG;
		newtok->value.ul = (unsigned long)ull;
		size = sizeof(unsigned long);
	    } else {
		newtok->value.l = (long)ull;
		size = sizeof(long);
	    }
	} else {
	    if (valtype & __ECC_UNSIGNED_FLAG) {
		newtok->value.ull = ull;
		size = sizeof(unsigned long long);
	    } else {
		newtok->value.ll = (long long)ull;
		size = sizeof(unsigned long long);
	    }
	}
	newtok->valtype = valtype;
	newtok->objsize = size;
    } else {

	return NULL;
    }
    newtok->type = __ECC_CONSTANT_TOKEN;

    return newtok;
}

struct __Ecctoken *
__Eccparseop(struct __Ecpptoken *cpptok)
{
    uint8_t *u8ptr;
    struct __Ecpptoken *tok;
    struct __Ecctoken *newtok;
    uint8_t u8;

    tok = cpptok;
    u8ptr = tok->name;
    u8 = *u8ptr;
    newtok = __Eccalloctok(FALSE);
    if (newtok == NULL) {
	
	return NULL;
    }
    u8ptr = newtok->name;
    newtok->type = __ECC_OPERATOR_TOKEN;
    *u8ptr++ = u8;
    *u8ptr = '\0';

    return newtok;
}

struct __Ecctoken *
__Eccparsestr(struct __Ecpptoken *cpptok)
{
    uint8_t *u8ptr;
    uint8_t *newstr;
    struct __Ecctoken *newtok;
    size_t newstrlen;
    size_t newstrsize;
    uint8_t u8;

    return NULL;
}

struct __Ecctoken *
__Eccalloctok(int allocname)
{
    uint8_t *name;
    struct __Ecctoken *newtok;

    newtok = calloc(1, sizeof(struct __Ecctoken));
    if (newtok == NULL) {
	__Erun.error.code = __EARTHQUAKE_ALLOCATION_FAILURE;

	return NULL;
    }
    if (allocname) {
	name = calloc(1, __ECC_TOKEN_NAME_MAX);
	if (name == NULL) {
	    __Erun.error.code = __EARTHQUAKE_ALLOCATION_FAILURE;
	    __Eccfreetok(newtok);
	    
	    return NULL;
	}
	newtok->name = name;
    }

    return newtok;
}

/*
 * Reallocate token name.
 */
uint8_t *
__Eccreallocname(struct __Ecctoken *tok, size_t size)
{
    uint8_t *name;

    if (tok->name) {
	name = realloc(tok->name, size);
    } else {
	name = calloc(1, size);
    }
    if (name == NULL) {
	__Erun.error.code = __EARTHQUAKE_ALLOCATION_FAILURE;

	return NULL;
    }
    tok->name = name;

    return name;
}

void
__Eccfreetok(struct __Ecctoken *tok)
{
    if (tok->name) {
	free(tok->name);
    }
    free(tok);

    return;
}

