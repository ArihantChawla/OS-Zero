/*
 * icpp.c - internal preprocessor routines for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#define FALSE 0
#define TRUE  1

#include <main.h>

static uint8_t *__Ecppinclpaths[] =
{
    "/include/",
    "/usr/include/",
    "/usr/local/include",
    NULL
};

/*
 * Split input file into preprocessor tokens. Return list of tokens.
 *
 * <NOTE>
 * - this stage parses ISO C trigraphs and digraphs, also inside strings and
 *   character constants.
 * - this stage splices lines terminated by '\\' followed by a newline.
 * </NOTE>
 */
struct __Ecpptoken *
__Ecppstage1(struct __Efile *file)
{
    unsigned long errcode;
    struct __Ecpptoken *firsttok;
    struct __Ecpptoken *newtok;
    struct __Ecpptoken *lasttok;

    newtok = __Ecppgettok(file);
    if (newtok == NULL) {

	return NULL;
    }
    firsttok = newtok;
    lasttok = newtok;
    while (newtok) {
	newtok = __Ecppgettok(file);
	lasttok->next = newtok;
	if (newtok->next) {
	    lasttok = __Erun.cpp.parse.lasttok;
	} else {
	    lasttok = newtok;
	}
    }
    errcode = __Erun.error.code;
    if (errcode) {
	__Eprterror(&__Erun.error);
	__Ecppfreetoks(firsttok);

	return NULL;
    }
    __Erun.cpp.parse.lasttok = lasttok;

    return firsttok;;
}

/*
 * Get next input token.
 */
struct __Ecpptoken *
__Ecppgettok(struct __Efile *file)
{
    uint8_t *name;
    struct __Ecpptoken *newtok;
    int sep;
    int c;

    newtok = NULL;
    sep = __Ecppskip(file);
    if (sep < 0) {
	
	return NULL;
    }
    if (sep == ' ') {
	newtok = __Ecppalloctok();
	if (newtok == NULL) {
	    
	    return NULL;
	}
	name = newtok->name;
	*name++ = ' ';
	*name = '\0';
	newtok->type = __ECPP_SEPARATOR_TOKEN;
	newtok->namelen = 1;
    } else if (sep == '\n') {
	newtok = __Ecppalloctok();
	if (newtok == NULL) {
	    
	    return NULL;
	}
	name = newtok->name;
	*name++ = '\n';
	*name = '\0';
	newtok->type = __ECPP_SEPARATOR_TOKEN;
	newtok->namelen = 1;
    } else {
	c = __Egetc(file);
	if (c < 0) {

	    return NULL;
	}
	c = __Ecppscantok(file, c);
	if (c < 0) {

	    return NULL;
	}
	if (__Ecppisop(c)) {
	    newtok = __Ecppgetop(file, c);
	    newtok->next = __Ecppparseop(file);
	} else if (__Eissep(c)) {
	    newtok = __Ecppgetsep(file, c);
	} else if (__Eisconstpre(c)) {
	    newtok = __Ecppgetconst(file, c);
	    if (newtok == NULL) {
		if (__Erun.error.code) {
		    __Eprterror(&__Erun.error);

		    return NULL;
		} else {
		    c = __Egetc(file);
		    if (c < 0) {
			__Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;

			return NULL;
		    }
		    if (__Eccisop(c)) {
			newtok = __Ecppgetop(file, c);
		    } else {
			newtok = __Ecppgetid(file, c);
		    }
		}
	    }
	} else if (__Eccisop(c)) {
	    newtok = __Ecppgetop(file, c);
	}
	if (__Eiswcstrpre(c)) {
	    newtok = __Ecppgetstr(file, c);
	    if (newtok == NULL) {
		if (__Erun.error.code) {
		    __Eprterror(&__Erun.error);
		    
		    return NULL;
		} else {
		    c = __Egetc(file);
		    if (c < 0) {
			__Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
			
			return NULL;
		    }
		    newtok = __Ecppgetid(file, c);
		}
	    }
	} else if (__Eisstrsep(c)) {
	    newtok = __Ecppgetstr(file, c);
	}
    }
    
    return newtok;
}

struct __Ecpptoken *
__Ecppgetop(struct __Efile *file, int ch)
{
    uint8_t *name;
    struct __Ecpptoken *newtok;
    size_t namelen;
    size_t namesize;
    int c;
    uint8_t u8;

    c = ch;
    newtok = __Ecppalloctok();
    if (newtok == NULL) {

	return NULL;
    }
    name = newtok->name;
    u8 = (uint8_t)c;
    *name++ = u8;
    namelen = 1;
    namesize = __ECPP_TOKEN_NAME_MAX;
    c = __Egetc(file);
    if (c < 0) {
	if (__Ecppisop(u8)) {
	    newtok->type = __ECPP_OPERATOR_TOKEN;
	    newtok->namelen = 1;
	    
	    return newtok;
	} else {

	    return NULL;
	}
    }
    c = __Ecppscantok(file, c);
    if (c < 0) {
	if (__Ecppisop(u8)) {
	    newtok->type = __ECPP_OPERATOR_TOKEN;
	    newtok->namelen = 1;

	    return newtok;
	} else {
	    
	    return NULL;
	}
    }
    if (__Ecppisop(c)) {
	while (__Ecppisop(c)) {
	    if (namelen == namesize) {
		namesize <<= 1;
		name = __Ecppreallocname(newtok, namesize);
		if (name == NULL) {
		    
		    return NULL;
		}
		name += namelen;
	    }
	    u8 = (uint8_t)c;
	    *name++ = u8;
	    namelen++;
	    c = __Egetc(file);
	    if (c < 0) {
		
		return NULL;
	    }
	    c = __Ecppscantok(file, c);
	    if (c < 0) {
		
		return NULL;
	    }
	}
    } else {
	while (__Eccisop(c)) {
	    if (namelen == namesize) {
		namesize <<= 1;
		name = __Ecppreallocname(newtok, namesize);
		if (name == NULL) {
		    
		    return NULL;
		}
		name += namelen;
	    }
	    u8 = (uint8_t)c;
	    *name++ = u8;
	    namelen++;
	    c = __Egetc(file);
	    if (c < 0) {
		
		return NULL;
	    }
	    c = __Ecppscantok(file, c);
	    if (c < 0) {
		
		return NULL;
	    }
	}
    }
    u8 = (uint8_t)c;
    __Eungetc(file, u8);
    if (namelen == namesize) {
	namesize <<= 1;
	name = __Ecppreallocname(newtok, namesize);
	if (name == NULL) {
	    
	    return NULL;
	}
	name += namelen;
    }
    *name = '\0';
    newtok->type = __ECPP_OPERATOR_TOKEN;
    newtok->namelen = namelen;

    return newtok;
}

/*
 * Get separator.
 */
struct __Ecpptoken *
__Ecppgetsep(struct __Efile *file, int ch)
{
    uint8_t *name;
    struct __Ecpptoken *newtok;
    int c;
    uint8_t u8;

    c = ch;
    newtok = __Ecppalloctok();
    if (newtok == NULL) {

	return NULL;
    }
    name = newtok->name;
    u8 = (uint8_t)c;
    *name++ = u8;
    *name = '\0';
    newtok->type = __ECPP_SEPARATOR_TOKEN;
    newtok->namelen = 1;

    return newtok;
}

struct __Ecpptoken *
__Ecppgetconst(struct __Efile *file, int ch)
{
    unsigned long escape;
    uint8_t *name;
    struct __Ecpptoken *newtok;
    size_t namelen;
    size_t namesize;
    int c;
    uint8_t u8;

    escape = FALSE;
    newtok = __Ecppalloctok();
    if (newtok == NULL) {

	return NULL;
    }
    name = newtok->name;
    namelen = 0;
    namesize = __ECPP_TOKEN_NAME_MAX;
    c = ch;
    if ((c) == '.') {
	*name++ = '.';
	c = __Egetc(file);
	if (c < 0) {

	    return NULL;
	}
	if (!__Eisdigit(c)) {
	    __Ecppfreetok(newtok);
	    u8 = (uint8_t)c;
	    __Eungetc(file, u8);
	    __Eungetc(file, '.');

	    return NULL;
	}
    }
    if (__Eiswcconstpre(c)) {
	*name++ = 'L';
	namelen++;
	c = __Egetc(file);
	if (c < 0) {

	    return NULL;
	}
	if (!__Eischconstsep(c)) {
	    __Ecppfreetok(newtok);
	    u8 = (uint8_t)c;
	    __Eungetc(file, u8);
	    __Eungetc(file, 'L');

	    return NULL;
	}
    }
    if (__Eischconstsep(c)) {
	*name++ = '\'';
	namelen++;
	c = __Egetc(file);
	if (c < 0) {
	    __Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;

	    return NULL;
	}
	c = __Ecppscantok(file, c);
	while ((!escape) && !__Eischconstsep(c)) {
	    if (namelen == namesize) {
		namesize <<= 1;
		name = __Ecppreallocname(newtok, namesize);
		if (name == NULL) {

		    return NULL;
		}
		name += namelen;
	    }
	    if (escape) {
		escape = FALSE;
	    } else if (__Eisescpre(c)) {
		escape = TRUE;
	    }
	    u8 = (uint8_t)c;
	    *name++ = u8;
	    namelen++;
	    c = __Egetc(file);
	    if (c < 0) {
		__Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;

		return NULL;
	    }
	    c = __Ecppscantok(file, c);
	    if (c < 0) {
		
		return NULL;
	    }
	}
	*name++ = '\'';
	namelen++;
    } else {
	while (__Eisnumconst(c)) {
	    if (namelen == namesize) {
		namesize <<= 1;
		name = __Ecppreallocname(newtok, namesize);
		if (name == NULL) {
		    
		    return NULL;
		}
		name += namelen;
	    }
	    u8 = (uint8_t)c;
	    *name++ = u8;
	    namelen++;
	    c = __Egetc(file);
	    if (c < 0) {
		__Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;

		return NULL;
	    }
	    c = __Ecppscantok(file, c);
	    if (c < 0) {

		return NULL;
	    }
	}
	u8 = (uint8_t)c;
	__Eungetc(file, u8);
    }
    if (namelen == namesize) {
	namesize <<= 1;
	name = __Ecppreallocname(newtok, namesize);
	if (name == NULL) {
	    
	    return NULL;
	}
	name += namelen;
    }
    *name = '\0';
    newtok->type = __ECPP_CONSTANT_TOKEN;
    newtok->namelen = namelen;
    
    return newtok;
}

struct __Ecpptoken *
__Ecppgetstr(struct __Efile *file, int ch)
{
    unsigned long escape;
    uint8_t *name;
    struct __Ecpptoken *newtok;
    size_t namelen;
    size_t namesize;
    int c;
    uint8_t u8;

    escape = FALSE;
    newtok = __Ecppalloctok();
    if (newtok == NULL) {

	return NULL;
    }
    name = newtok->name;
    namelen = 0;
    namesize = __ECPP_TOKEN_NAME_MAX;
    c = ch;
    if (__Eiswcstrpre(c)) {
	*name++ = 'L';
	namelen++;
	c = __Egetc(file);
	if (c < 0) {

	    return NULL;
	}
	if (!__Eisstrsep(c)) {
	    u8 = (uint8_t)c;
	    __Eungetc(file, u8);
	    __Eungetc(file, 'L');

	    return NULL;
	}
    }
    *name++ = '\"';
    namelen = 1;
    namesize = __ECPP_TOKEN_NAME_MAX;
    c = __Egetc(file);
    if (c < 0) {
	__Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
	
	return NULL;
    }
    c = __Ecppscantok(file, c);
    if (c < 0) {

	return NULL;
    }
    while ((!escape) && !__Eisstrsep(c)) {
	if (namelen == namesize) {
	    namesize <<= 1;
	    name = __Ecppreallocname(newtok, namesize);
	    if (name == NULL) {
		
		return NULL;
	    }
	    name += namelen;
	}
	if (escape) {
	    escape = FALSE;
	} else if (__Eisescpre(c)) {
	    escape = TRUE;
	}
	u8 = (uint8_t)c;
	*name++ = u8;
	namelen++;
	c = __Egetc(file);
	if (c < 0) {
	    __Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
	    
	    return NULL;
	}
	c = __Ecppscantok(file, c);
	if (c < 0) {
	    
	    return NULL;
	}
    }
    if (namelen > namesize - 2) {
	namesize <<= 1;
	name = __Ecppreallocname(newtok, namesize);
	if (name == NULL) {
	    
	    return NULL;
	}
	name += namelen;
    }
    *name++ = '\"';
    namelen++;
    *name = '\0';
    newtok->type = __ECPP_STRING_TOKEN;
    newtok->namelen = namelen;

    return newtok;
}

/*
 * Get identifier token.
 */
struct __Ecpptoken *
__Ecppgetid(struct __Efile *file, int ch)
{
    uint8_t *name;
    struct __Ecpptoken *newtok;
    struct __Ecckeyword *keyword;
    size_t namelen;
    size_t namesize;
    int c;
    uint8_t u8;
    
    newtok = __Ecppalloctok();
    if (newtok == NULL) {

	return NULL;
    }
    name = newtok->name;
    namelen = 0;
    namesize = __ECPP_TOKEN_NAME_MAX;
    c = ch;
    while (__Eisid(c)) {
	if (namelen == namesize) {
	    namesize <<= 1;
	    name = __Ecppreallocname(newtok, namesize);
	    if (name == NULL) {
		
		return NULL;
	    }
	    name += namelen;
	}
	u8 = (uint8_t)c;
	*name++ = u8;
	namelen++;
	c = __Egetc(file);
	if (c < 0) {
	    __Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
	    
	    return NULL;
	}
	c = __Ecppscantok(file, c);
	if (c < 0) {

	    return NULL;
	}
    }
    u8 = (uint8_t)c;
    __Eungetc(file, u8);
    if (namelen == namesize) {
	namesize <<= 1;
	name = __Ecppreallocname(newtok, namesize);
	if (name == NULL) {
	    
	    return NULL;
	}
	name += namelen;
    }
    *name = '\0';
    name = newtok->name;
    keyword = __Eccfindkeyword(name);
    if (keyword) {
	newtok->type = __ECPP_KEYWORD_TOKEN;
    } else {
	newtok->type = __ECPP_IDENTIFIER_TOKEN;
    }
    newtok->namelen = namelen;

    return newtok;
}

struct __Ecpptoken *
__Ecppparseop(struct __Efile *file)
{
    unsigned long list;
    uint8_t *name;
    struct __Ecpptoken *firsttok;
    struct __Ecpptoken *dirtok;
    struct __Ecpptoken *newtok;
    size_t namelen;
    size_t namesize;
    int sep;
    int c;
    uint8_t u8;

    list = FALSE;
    newtok = __Ecppalloctok();
    if (newtok == NULL) {

	return NULL;
    } 
    firsttok = newtok;
    name = newtok->name;
    namelen = 0;
    namesize = __ECPP_TOKEN_NAME_MAX;
    sep = __Ecppskip(file);
    if (sep < 0) {
	newtok->operation = __ECPP_NULL_OPERATION;

	return newtok;
    }
    if (sep == ' ') {
	*name++ = ' ';
	*name = '\0';
	newtok->type = __ECPP_SEPARATOR_TOKEN;
	newtok->namelen = 1;
	list = TRUE;
    } else if (sep == '\n') {

	return NULL;
    }
    c = __Egetc(file);
    if (c < 0) {

	return NULL;
    }
    c = __Ecppscantok(file, c);
    if (c < 0) {

	return NULL;
    }
    if (!__Eisidpre(c)) {
	__Erun.error.code = __EARTHQUAKE_UNEXPECTED_TOKEN;

	return NULL;
    }
    if (list) {
	newtok->next = __Ecppalloctok();
	newtok = newtok->next;
	if (newtok == NULL) {

	    return NULL;
	}
    }
    dirtok = newtok;
    name = newtok->name;
    namelen = 0;
    namesize = __ECPP_TOKEN_NAME_MAX;
    while (__Eisid(c)) {
	if (namelen == namesize) {
	    namesize <<= 1;
	    name = __Ecppreallocname(newtok, namesize);
	    if (name == NULL) {

		return NULL;
	    }
	    name += namelen;
	}
	u8 = (uint8_t)c;
	*name++ = u8;
	namelen++;
	c = __Egetc(file);
	if (c < 0) {
	    __Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;

	    return NULL;
	}
	c = __Ecppscantok(file, c);
	if (c < 0) {

	    return NULL;
	}
    }
    u8 = (uint8_t)c;
    __Eungetc(file, u8);
    if (namelen == namesize) {
	namesize <<= 1;
	name = __Ecppreallocname(newtok, namesize);
	if (name == NULL) {
	    
	    return NULL;
	}
	name += namelen;
    }
    *name = '\0';
    newtok->type = __ECPP_DIRECTIVE_TOKEN;
    newtok->namelen = namelen;
    newtok->next = __Ecppalloctok();
    newtok = newtok->next;
    if (newtok == NULL) {

	return NULL;
    }
    name = newtok->name;
    c = __Egetc(file);
    if (c < 0) {

	return NULL;
    }
    c = __Ecppscantok(file, c);
    if (c < 0) {

	return NULL;
    }
    sep = __Ecppskip(file);
    if (sep < 0) {

	return NULL;
    }
    if (sep == ' ') {
	*name++ = ' ';
	*name = '\0';
	newtok->type = __ECPP_SEPARATOR_TOKEN;
	newtok->namelen = 1;
	newtok->next = __Ecppalloctok();
	newtok = newtok->next;
	if (newtok == NULL) {

	    return NULL;
	}
    } else if (sep == '\n') {

	return NULL;
    }
    name = dirtok->name;
    if (!strcmp(name, "define")) {
	newtok->operation = __ECPP_DEFINE_OPERATION;
	newtok->next = __Ecppgetdef(file);
	newtok = newtok->next;
	if (newtok == NULL) {

	    return NULL;
	}
	__Erun.cpp.parse.lasttok = newtok;
    } else if (!strcmp(name, "include")) {
	__Ecppfreetoks(firsttok);
	newtok = __Ecppinclude(file);
	if (newtok == NULL) {

	    return NULL;
	}
	firsttok = newtok;
    } else if (!strcmp(name, "if")) {
	newtok->operation = __ECPP_IF_OPERATION;
	newtok->next = __Ecppgetif(file);
	newtok = newtok->next;
	if (newtok == NULL) {

	    return NULL;
	}
    } else if (!strcmp(name, "ifdef")) {
	newtok->operation = __ECPP_IFDEF_OPERATION;
    } else if (!strcmp(name, "ifndef")) {
	newtok->operation = __ECPP_IFNDEF_OPERATION;
    } else if (!strcmp(name, "line")) {
	newtok->operation = __ECPP_LINE_OPERATION;
    } else if (!strcmp(name, "error")) {
	newtok->operation = __ECPP_ERROR_OPERATION;
    } else if (!strcmp(name, "pragma")) {
	newtok->operation = __ECPP_PRAGMA_OPERATION;
    }

    return firsttok;
}

struct __Ecpptoken *
__Ecppgetdef(struct __Efile *file)
{
    struct __Ecpptoken *newtok;
    int sep;
    int c;

    sep = __Ecppskip(file);
    if (sep == '\n') {
	
	return NULL;
    }
    c = __Egetc(file);
    if (c < 0) {

	return NULL;
    }
    c = __Ecppscantok(file, c);
    if (c < 0) {
	__Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
	
	return NULL;
    }
    newtok = __Ecppgetid(file, c);

    return newtok;
}

struct __Ecpptoken *
__Ecppinclude(struct __Efile *file)
{
    unsigned long local;
    uint8_t *name;
    struct __Ecpptoken *newtok;
    struct __Efile *newfile;
    size_t namelen;
    size_t namesize;
    int sep;
    int c;
    uint8_t u8;

    sep = __Ecppskip(file);
    if (sep == '\n') {
	
	return NULL;
    }
    c = __Egetc(file);
    if (c < 0) {
	__Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
	
	return NULL;
    }
    c = __Ecppscantok(file, c);
    if (c < 0) {
	__Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
	
	return NULL;
    }
    local = FALSE;
    if (c == '\"') {
	local = TRUE;
    } else if (c != '<') {
	__Erun.error.code = __EARTHQUAKE_UNEXPECTED_TOKEN;

	return NULL;
    }
    c = __Egetc(file);
    if (c < 0) {
	__Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;

	return NULL;
    }
    name = calloc(1, MAXPATHLEN + 1);
    u8 = (uint8_t)c;
    *name++ = u8;
    namelen = 1;
    namesize = __ECPP_TOKEN_NAME_MAX;
    while (!__Ecppisinclsuf(c)) {
	if (namelen == namesize) {
	    namesize <<= 1;
	    name = realloc(name, namesize);
	    if (name == NULL) {

		return NULL;
	    }
	    name += namelen;
	}
	if (__Ecppistrigpre(c)) {
	    c = __Ecppgettrig(file);
	    if (c < 0) {

		return NULL;
	    }
	}
	if (__Eisescpre(c)) {
	    c = __Ecppescval(c);
	}
	u8 = (uint8_t)c;
	*name++ = u8;
	namelen++;
	c = __Egetc(file);
	if (c < 0) {
	    __Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;

	    return NULL;
	}
    }
    if (namelen == namesize) {
	namesize <<= 1;
	name = realloc(name, namesize);
	if (name == NULL) {
	    
	    return NULL;
	}
	name += namelen;
    }
    *name = '\0';
    newfile = __Ecppfindincl(name, local);
    if (newfile == NULL) {

	return NULL;
    }
    newtok = __Ecppstage1(newfile);

    return newtok;
}

struct __Ecpptoken *
__Ecppgetif(struct __Efile *file)
{
    return NULL;
}

/*
 * Skip white apace and comments. Return 1 if comments were skipped to
 * indicate that they need to be replaced with a single space. Return 2 if a
 * newline was encountered.
 */
int
__Ecppskip(struct __Efile *file)
{
    unsigned long loop1;
    unsigned long loop2;
    uint8_t *fname;
    int ch;
    int space;
    uint8_t u8;

    ch = __Egetc(file);
    if (ch < 0) {
	fname = file->name;
	__Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
	__Erun.error.str = fname;
	
	return -1;
    }
    space = 0;
    loop1 = TRUE;
    while (loop1) {
	while (__Eisspace(ch)) {
	    if (ch == '\n') {

		return '\n';
	    }
	    ch = __Egetc(file);
	    if (ch < 0) {
		fname = file->name;
		__Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
		__Erun.error.str = fname;
		
		return -1;
	    }
	}
	if (ch == '/') {
	    ch = __Egetc(file);
	    if (ch < 0) {
		fname = file->name;
		__Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
		__Erun.error.str = fname;
		
		return -1;
	    }
	    if (ch == '*') {
		ch = __Egetc(file);
		if (ch < 0) {
		    fname = file->name;
		    __Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
		    __Erun.error.str = fname;
		
		    return -1;
		}
		loop2 = TRUE;
		while (loop2) {
		    while (ch != '*') {
			ch = __Egetc(file);
			if (ch < 0) {	
			    fname = file->name;
			    __Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
			    __Erun.error.str = fname;
		
			    return -1;
			}
		    }
		    ch = __Egetc(file);
		    if (ch < 0) {
			fname = file->name;
			__Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
			__Erun.error.str = fname;
		
			return -1;
		    }
		    if (ch == '/') {
			loop2 = FALSE;
		    }
		}
	    
		space = ' ';
	    } else {
		u8 = (uint8_t)ch;
		__Eungetc(file, u8);
		__Eungetc(file, '/');
	    }
	} else {
	    loop1 = FALSE;
	    u8 = (uint8_t)ch;
	    __Eungetc(file, u8);
	}
    }

    return space;
}

/*
 * Get next token character. Trigraphs and digraphs are parsed, and lines
 * ending with '\' and a newline ignored.
 */
int
__Ecppscantok(struct __Efile *file, int ch)
{
    int c;
    uint8_t u8;

    c = ch;
    if (__Ecppistrigpre(c)) {
	c = __Ecppgettrig(file);
	if (c < 0) {

	    return -1;
	}
    } else if (__Ecppisdigpre(c)) {
	c = __Ecppgetdig(file, c);
	if (c < 0) {

	    return -1;
	}
    }
    while (__Eisescpre(c)) {
	c = __Egetc(file);
	if (c < 0) {
	    __Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
	    
	    return -1;
	}
	if (__Eisesc(c)) {
	    u8 = (uint8_t)c;
	    __Eungetc(file, u8);

	    return '\\';
	}
	while (c != '\n') {
	    c = __Egetc(file);
	    if (c < 0) {
		__Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
		
		return -1;
	    }
	}
    }
    if (__Ecppistrigpre(c)) {
	c = __Ecppgettrig(file);
    } else if (__Ecppisdigpre(c)) {
	c = __Ecppgetdig(file, c);
    }
    
    return c;
}
    

int
__Ecppgettrig(struct __Efile *file)
{
    int c;
    uint8_t u8;

    c = __Egetc(file);
    if (c < 0) {
	__Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
	
	return -1;
    }
    if (__Ecppistrigpre(c)) {
	c = __Egetc(file);
	if (c < 0) {
	    __Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
	    
	    return -1;
	}
	c = __Ecpptrigval(c);
	if (c < 0) {
	    __Erun.error.code = __EARTHQUAKE_UNEXPECTED_TOKEN;
	    
	    return -1;
	}
    } else {
	u8 = (uint8_t)c;
	__Eungetc(file, u8);
	c = '\?';
    }

    return c;
}

int
__Ecpptrigval(int ch)
{
    int retval;
    int c;

    retval = -1;
    c = ch;
    switch (c) {
	case '!':
	    retval = '|';
	    
	    break;
	case '\'':
	    retval = '^';
	    
	    break;
	case '(':
	    retval = '[';
	    
	    break;
	case ')':
	    retval = ']';
	    
	    break;
	case '-':
	    retval = '~';
	    
	    break;
	case '/':
	    retval = '\\';
	    
	    break;
	case '<':
	    retval = '{';
	    
	    break;
	case '=':
	    retval = '#';
	    
	    break;
	case '>':
	    retval = '}';
	    
	    break;
	default:

	    retval = -1;
    }

    return retval;
}

int
__Ecppgetdig(struct __Efile *file, int ch)
{
    int c;
    uint8_t u8;

    c = ch;
    if (c == '%') {
	c = __Egetc(file);
	if (c < 0) {
	    __Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
	    
	    return -1;
	}
	u8 = (uint8_t)c;
	switch (c) {
	    case ':':
		c = '#';
		
		break;
	    case '>':
		c = '}';
		
		break;
	    default:
		c = -1;
		
		break;
	}
	if (c < 0) {
	    __Eungetc(file, u8);
	    c = '%';
	}
    } else if (c == ':') {
	c = __Egetc(file);
	if (c < 0) {
	    __Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
	    
	    return -1;
	}
	u8 = (uint8_t)c;
	switch (c) {
	    case '>':
		c = ']';
		
		break;
	    default:
		c = -1;
		
		break;
	}
	if (c < 0) {
	    __Eungetc(file, u8);
	    c = ':';
	}
    } else if (c == '<') {
	c = __Egetc(file);
	if (c < 0) {
	    __Erun.error.code = __EARTHQUAKE_UNEXPECTED_EOF;
	    
	    return -1;
	}
	u8 = (uint8_t)c;
	switch (c) {
	    case '%':
		c = '{';
		
		break;
	    case ':':
		c = '[';
		
		break;
	    default:
		c = -1;
		
		break;
	}
	if (c < 0) {
	    __Eungetc(file, u8);
	    c = '<';
	}
    }

    return c;
}

int
__Ecppparseesc(uint8_t *str)
{
    uint8_t *u8ptr;
    unsigned long count;
    int c;
    int tmpc;

    u8ptr = str;
    c = *u8ptr++;
    if (__Eisoctdigit(c)) {
	count = 3;
	tmpc = c - '0';
	while (--count) {
	    c = *u8ptr++;
	    if (!__Eisoctdigit(c)) {

		break;
	    }
	    c -= '0';
	    tmpc <<= __EARTHQUAKE_LOG2_OCT;
	    tmpc += c;
	}
	c = tmpc;
    } else if (__Eishexpre(c)) {
	count = 2;
	tmpc = 0;
	while (count--) {
	    c = *u8ptr++;
	    if (!__Eishexdigit(c)) {

		break;
	    }
	    if (c <= '9') {
		c -= '0';
	    } else if (c >= 'a') {
		c -= 87;
	    } else {
		c -= 55;
	    }
	    tmpc <<= __EARTHQUAKE_LOG2_HEX;
	    tmpc += c;
	}
	c = tmpc;
    } else {
	c = __Ecppescval(c);
    }

    return c;
}

int
__Ecppescval(int ch)
{
    int retval;
    int c;

    retval = -1;
    c = ch;
    switch (c) {
	case '\"':
	    retval = '\"';

	    break;
	case '\'':
	    retval = '\'';

	    break;
	case '\?':
	    retval = '\?';

	    break;
	case '\\':
	    retval = '\\';

	    break;
	case 'a':
	    retval = '\a';

	    break;
	case 'b':
	    retval = '\b';

	    break;
	case 'f':
	    retval = '\f';

	    break;
	case 'n':
	    retval = '\n';

	    break;
	case 'r':
	    retval = '\r';

	    break;
	case 't':
	    retval = '\t';

	    break;
	case 'v':
	    retval = '\v';

	    break;
	default:
	    retval = -1;

	    break;
    }

    return retval;
}
    
/*
 * Allocate token.
 */
struct __Ecpptoken *
__Ecppalloctok(void)
{
    uint8_t *name;
    struct __Ecpptoken *newtok;

    newtok = calloc(1, sizeof(struct __Ecpptoken));
    if (newtok == NULL) {
	__Erun.error.code = __EARTHQUAKE_ALLOCATION_FAILURE;

	return NULL;
    }
    name = calloc(1, __ECPP_TOKEN_NAME_MAX);
    if (name == NULL) {
	__Erun.error.code = __EARTHQUAKE_ALLOCATION_FAILURE;

	return NULL;
    }
    newtok->name = name;

    return newtok;
}

/*
 * Reallocate token name.
 */
uint8_t *
__Ecppreallocname(struct __Ecpptoken *tok, size_t size)
{
    uint8_t *name;

    name = realloc(tok->name, size);
    if (name == NULL) {
	__Erun.error.code = __EARTHQUAKE_ALLOCATION_FAILURE;

	return NULL;
    }
    tok->name = name;

    return name;
}

/*
 * Free token.
 */
void
__Ecppfreetok(struct __Ecpptoken *tok)
{
    if (tok->name) {
	free(tok->name);
    }
    free(tok);

    return;
}

void
__Ecppfreetoks(struct __Ecpptoken *tok)
{
    struct __Ecpptoken *freetok;
    struct __Ecpptoken *nexttok;

    freetok = tok;
    while (freetok) {
	nexttok = freetok->next;
	__Ecppfreetok(freetok);
	freetok = nexttok;
    }

    return;
}

struct __Efile *
__Ecppfindincl(uint8_t *name, int local)
{
    unsigned long ndx;
    uint8_t *path;
    struct __Efile *newfile;
    uint8_t pathname[MAXPATHLEN + 1];
    size_t len;

    newfile = NULL;
    if (local) {
	newfile = __Eopen(name, FALSE, TRUE);
    }
    ndx = 0;
    path = __Ecppinclpaths[0];
    while ((newfile == NULL) && (path)) {
	strcat(pathname, path);
	len = strlen(pathname);
	strncat(pathname, name, MAXPATHLEN - len);
	newfile = __Eopen(name, FALSE, TRUE);
	ndx++;
	path = __Ecppinclpaths[0];
    }

    return newfile;
}

int
__Ecppoutput(uint8_t *filename)
{
    uint8_t *fname;
    uint8_t *u8ptr;
    struct __Efile *file;
    struct __Ecpptoken *tok;
    size_t nb;

    fname = filename;
    file = __Eopen(fname, TRUE, FALSE);
    tok = __Erun.cpp.parse.firsttok;
    while (tok) {
	u8ptr = tok->name;
	nb = tok->namelen;
	if (__Ewrite(file, u8ptr, nb) < 0) {
	    unlink(fname);

	    return -1;
	}
	tok = tok->next;
    }

    return 0;
}

void
__Ecppreset(void)
{
    __Ecppfreetoks(__Erun.cpp.parse.firsttok);

    return;
}

