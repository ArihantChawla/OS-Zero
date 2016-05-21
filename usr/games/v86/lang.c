#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <v86/conf.h>
#include <v86/errnum.h>


#if (V86_DEFAULT_CHARSET == V86_ASCII_CHARSET) /* ASCII */

#define V86_CHARSET_SIZE                128
#define V86_CHARSET_MAX                 (V86_CHARSET_SIZE - 1)

typedef char v86char;

#else

#define V86_CHARSET_SIZE                256
#define V86_CHARSET_MAX                 (V86_CHARSET_SIZE - 1)

typedef unsigned char v86char;

#endif

#define V86_CHAR_MNEMONIC_BIT           (1 << 0)
#define V86_CHAR_SYMBOL_BIT             (1 << 1)
#define V86_CHAR_DECIMAL_BIT            (1 << 2)
#define V86_CHAR_HEXADECIMAL_BIT        (1 << 3)
#define V86_CHAR_OCTAL_BIT              (1 << 4)
#define V86_CHAR_OPERATOR_BIT           (1 << 5)
#define V86_CHAR_MACRO_BIT              (1 << 6)
#define V86_CHAR_SPACE_BIT              (1 << 7)
struct v86char {
    int val;
    int flg;
};

static struct v86char *v86charset;

#define v86ismnemchar(c) (isalpha(c))
#define v86isidchar1(c)  (isalpha(c) || ((c) == '_'))
#define v86isidchar(c)   (isalpha(c) || isdigit(c))
#define v86issepchar(c)  (isspace(c)                                    \
                          || ((c) == '[') || ((c) == ']')               \
                          || ((c) == '{') || ((c) == '}'))

static void
v86setalphacharbits(struct v86char *map)
{
    int flg = (V86_CHAR_MNEMONIC_BIT
               | V86_CHAR_SYMBOL_BIT                                    \
               | V86_CHAR_MACRO_BIT);
    int ch;

    for (ch = 'a' ; ch <= 'z' ; ch++) {
        map[ch].flg |= flg;;
    }
    for (ch = 'A' ; ch <= 'Z' ; ch++) {
        map[ch].flg |= flg;;
    }

    return;
}

static void
v86setdigitcharbits(struct v86char *map)
{
    int flg;
    int ch;

    flg = V86_CHAR_DECIMAL_BIT | V86_CHAR_HEXADECIMAL_BIT | V86_CHAR_OCTAL_BIT;
    for (ch = '0' ; ch <= '7' ; ch++) {
        map[ch].flg |= flg;
    };
    flg = V86_CHAR_DECIMAL_BIT | V86_CHAR_HEXADECIMAL_BIT;
    for (ch = '8' ; ch <= '9' ; ch++) {
        map[ch].flg |= flg;
    };
    flg = V86_CHAR_HEXADECIMAL_BIT;
    for (ch = 'A' ; ch <= 'F' ; ch++) {
        map[ch].flg |= flg;
    };

    return;
}

static void
v86setopercharbits(struct v86char *map)
{
    map['!'].flg |= V86_CHAR_OPERATOR_BIT;
    map['%'].flg |= V86_CHAR_OPERATOR_BIT;
    map['&'].flg |= V86_CHAR_OPERATOR_BIT;
    map['('].flg |= V86_CHAR_OPERATOR_BIT;
    map[')'].flg |= V86_CHAR_OPERATOR_BIT;
    map['*'].flg |= V86_CHAR_OPERATOR_BIT;
    map['+'].flg |= V86_CHAR_OPERATOR_BIT;
    map[','].flg |= V86_CHAR_OPERATOR_BIT;
    map['-'].flg |= V86_CHAR_OPERATOR_BIT;
    map['/'].flg |= V86_CHAR_OPERATOR_BIT;
    map[':'].flg |= V86_CHAR_OPERATOR_BIT;
    map[';'].flg |= V86_CHAR_OPERATOR_BIT;
    map['<'].flg |= V86_CHAR_OPERATOR_BIT;
    map['='].flg |= V86_CHAR_OPERATOR_BIT;
    map['>'].flg |= V86_CHAR_OPERATOR_BIT;
    map['?'].flg |= V86_CHAR_OPERATOR_BIT;
    map['['].flg |= V86_CHAR_OPERATOR_BIT;
    map[']'].flg |= V86_CHAR_OPERATOR_BIT;
    map['^'].flg |= V86_CHAR_OPERATOR_BIT;
    map['|'].flg |= V86_CHAR_OPERATOR_BIT;
    map['~'].flg |= V86_CHAR_OPERATOR_BIT;

    return;
}

static void
v86setasciicharbits(struct v86char *map)
{
    int ch;

    for (ch = 1 ; ch < V86_CHARSET_SIZE ; ch++) {
        if (isalpha(ch)) {
            map[ch].flg |= (V86_CHAR_MNEMONIC_BIT
                            | V86_CHAR_SYMBOL_BIT
                            | V86_CHAR_MACRO_BIT);
        } else if (isdigit(ch)) {
            map[ch].flg |= V86_CHAR_MACRO_BIT;
        } else if (ch == '_') {
            map[ch].flg |= (V86_CHAR_SYMBOL_BIT
                            | V86_CHAR_MACRO_BIT);
        } else if (isspace(ch)) {
            map[ch].flg |= V86_CHAR_SPACE_BIT;
        }
    }

    return;
}

static struct v86char *
v86mkasciiset(void)
{
#if (V86_DEFAULT_CHARSET == V86_ASCII_CHARSET)
    struct v86char *charset = calloc(V86_CHARSET_SIZE, sizeof(char));
#endif

    if (!charset) {
        fprintf(stderr, "failed to allocate character set\n");

        return NULL;
    }

    return charset;
}

void
v86initcharset(unsigned long type)
{
    struct v86char *charset;
    
    if (type != V86_ASCII_CHARSET) {
        fprintf(stderr, "non-ASCII character sets not supported\n");
    }
    charset = v86mkasciiset();
    if (charset) {
        v86setalphacharbits(charset);
        v86setdigitcharbits(charset);
        v86setopercharbits(charset);
        v86setasciicharbits(charset);

        v86charset = charset;
    }
    
    return;
}

