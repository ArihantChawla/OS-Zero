/*
 * parse.h - internal parsing header for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#ifndef EARTHQUAKE_INTERNAL_PARSE_H
#define EARTHQUAKE_INTERNAL_PARSE_H

#define __ECC_OPERATOR_LENGTH_MAX 3

/*
 * Preprocessor character classification macros.
 */
/* preprocessor operator. */
#define __Ecppisop(c) \
    ((c) == '#')
#define __Ecppisinclpre(c) \
    (((c) == '<') \
     || ((c) == '\"'))
#define __Ecppisinclsuf(c) \
    (((c) == '>') \
     || ((c) == '\"'))
#define __Ecppistrigpre(c) \
    ((c) == '\?')
#define __Ecppisdigpre(c) \
    (((c) == '%') \
     || ((c) == ':') \
     || ((c) == '<'))
#define __Eisescpre(c) \
    ((c) == '\\')
#define __Eisesc(c) \
    (__Eesctbl[(c)])
/*
 * Compiler character classification.
 */
/* wide character string prefix. */
#define __Eiswcstrpre(c) \
    ((c) == 'L')
/* string separator. */
#define __Eisstrsep(c) \
    ((c) == '\"')
/* constant prefixes. */
#define __Eiswcconstpre(c) \
    ((c) == 'L')
#define __Eischconstsep(c) \
    ((c) == '\'')
#define __Eisconstpre(c) \
    (__Eisdigit(c) \
     || __Eiswcconstpre(c) \
     || __Eischconstsep(c) \
     || ((c) == '.'))
/* octal prefix. */
#define __Eisoctprefix(c) \
    ((c) == '0')
/* hexadecimal prefix. */
#define __Eishexpre(c) \
    (((c) == '0') || ((c) == 'x') || ((c) == 'X'))
/* decimal digit. */
#define __Eisdigit(c) \
    (((c) >= '0') && ((c) <= '9'))
/* octal digit. */
#define __Eisoctdigit(c) \
    (((c) >= '0') && ((c) <= '7'))
/* hexadecimal digit. */
#define __Eishexdigit(c) \
    (__Eisdigit(c) \
     || (((c) >= 'a') && ((c) <= 'f')) \
     || (((c) >= 'A') && ((c) <= 'F')))
/* decimal floating-point exponent prefix. */
#define __Eisdecfltexppre(c) \
    (((c) == 'e') \
     || ((c) == 'E'))
/* octal or hexadecimal floating-point exponent prefix. */
#define __Eisaltfltexppre(c) \
    (((c) == 'p') \
     || ((c) == 'P'))
/* floating-point exponent prefix. */
#define __Eisfltexppre(c) \
    (__Eisdecfltexppre(c) \
     || __Eisaltfltexppre(c))
/* integer suffix. */
#define __Eisisuf(c) \
    ((((c) == 'u') || ((c) == 'U')) \
     || (((c) == 'l') || ((c) == 'L')))
/* floating-point suffix. */
#define __Eisfltsuf(c) \
    ((((c) == 'f') || ((c) == 'F')) \
     || (((c) == 'l') || ((c) == 'L')))
#define __Eissuf(c) \
    (__Eisisuf(c) \
     || __Eisfltsuf(c))
/* decimal integer constant. */
#define __Eisdec(c) \
    (__Eisdigit(c) || __Eisisuf(c))
/* hexadecimal integer constant. */
#define __Eishex(c) \
    (__Eishexpre(c) \
     || __Eishexdigit(c) \
     || __Eisisuf(c))
/* decimal floating-point constant. */
#define __Eisfltconst(c) \
    (__Eisdigit(c) \
     || ((c) == '.') \
     || __Eisfltsuf(c))
/* octal floating-point constant. */
#define __Eisoctfltconst(c) \
    (__Eisoctdigit(c) \
     || ((c) == '.') \
     || __Eisfltsuf(c))
/* hexadecimal floating-point constant. */
#define __Eishexfltconst(c) \
    (__Eisoctdigit(c) \
     || ((c) == '.') \
     || __Eisfltsuf(c))
/* numerical constant. */
#define __Eisnumconst(c) \
    (__Eishexdigit(c) \
     || ((c) == '.') \
     || __Eisfltexppre(c) \
     || __Eisfltsuf(c))
/* alphabetical id prefix with '_' allowed. */
#define __Eisidpre(c) \
    ((((c) >= 'a') && ((c) <= 'z')) \
     || (((c) >= 'A') && ((c) <= 'Z')) \
     || ((c) == '_'))
/* alphanumerical id with '_' allowed. */
#define __Eisid(c) \
    (__Eisidpre(c) \
     || __Eisdigit(c))
/* white space. */
#if (__EARTHQUAKE_INPUT_ASCII_BASED)
#define __Eisspace(c) \
    (((c) == ' ') \
     || (((c) >= '\t') && ((c) <= '\r')))
#else /* !__EARTHQUAKE_INPUT_ASCII_BASED */
#define __Eisspace(c) \
    (((c) == ' ') \
     || ((c) == '\t') \
     || ((c) == '\n') \
     || ((c) == '\r') \
     || ((c) == '\f') \
     || ((c) == '\v'))
#endif
/* operator. */
/*
 * <NOTE>
 * - the following characters are counted as operators: '#', '=', '+', '-',
 *   '*', '/', '!', '&', '|', '~', '^', '<', '>', '[', ']', '.', '\?', ':',
 *   '%', and ','.
 */
#if (__EARTHQUAKE_INPUT_ASCII_BASED)
#define __Eccisop(c) \
    (__Eccoptbl[(c)])
#else /* !__EARTHQUAKE_INPUT_ASCII_BASED */
#define __Eccisop(c) \
    (((c) == '=') \
     || ((c) == '+') \
     || ((c) == '-') \
     || ((c) == '*') \
     || ((c) == '/') \
     || ((c) == '!') \
     || ((c) == '&') \
     || ((c) == '|') \
     || ((c) == '~') \
     || ((c) == '^') \
     || ((c) == '<') \
     || ((c) == '>') \
     || ((c) == '[') \
     || ((c) == ']') \
     || ((c) == '.') \
     || ((c) == '\?') \
     || ((c) == ':') \
     || ((c) == '%') \
     || ((c) == ','))
#endif /* __EARTHQUAKE_INPUT_ASCII_BASED */
/* separator. */
#define __Eissep(c) \
    (__Eseptbl[(c)])

#endif /* EARTHQUAKE_INTERNAL_PARSE_H */

