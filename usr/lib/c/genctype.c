#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <locale.h>
#include <string.h>

#define NLANG       2

const char *colltab[NLANG]
= {
    "en_US",
    "fi_FI"
};

#define CHARISALNUM 0x01
#define CHARISALPHA 0x02
#define CHARISGRAPH 0x04
#define CHARISLOWER 0x08
#define CHARISPRINT 0x10
#define CHARISPUNCT 0x20
#define CHARISSPACE 0x40
#define CHARISUPPER 0x80

static const char *ctypetab[32]
= {
    "C",
    "fi_FI.ISO8859-1",
    "fi_FI.ISO8859-2",
    "fi_FI.ISO8859-3",
    "fi_FI.ISO8859-4",
    "fi_FI.ISO8859-5",
    "fi_FI.ISO8859-6",
    "fi_FI.ISO8859-7",
    "fi_FI.ISO8859-8",
    "fi_FI.ISO8859-9",
    "fi_FI.ISO8859-10",
    "fi_FI.ISO8859-11",
    "fi_FI.ISO8859-12",
    "fi_FI.ISO8859-13",
    "fi_FI.ISO8859-14",
    "fi_FI.ISO8859-15",
    "fi_FI.ISO8859-16",
    NULL
};

static const char *ctypepostfixtab[32]
= {
    "c",
    "iso8859_1",
    "iso8859_2",
    "iso8859_3",
    "iso8859_4",
    "iso8859_5",
    "iso8859_6",
    "iso8859_7",
    "iso8859_8",
    "iso8859_9",
    "iso8859_10",
    "iso8859_11",
    "iso8859_12",
    "iso8859_13",
    "iso8859_14",
    "iso8859_15",
    "iso8859_16"
};

static const char *collpostfixtab[NLANG]
= {
    "en_US",
    "fi_FI"
};

unsigned char collwtab[256];
unsigned char collwtmptab[256];

void
printctype(const char *postfix, FILE *fp)
{
    int     i;
    uint8_t u8;

    fprintf(fp, "/* generated automatically by genctype */\n\n");
    fprintf(fp, "uint8_t ctypeparamtab_%s[256]\n= {\n", postfix);
    for (i = 0 ; i < 256 ; i++) {
        u8 = 0;
        if (isalnum(i)) {
            u8 |= CHARISALNUM;
        }
        if (isalpha(i)) {
            u8 |= CHARISALPHA;
        }
        if (isgraph(i)) {
            u8 |= CHARISGRAPH;
        }
        if (islower(i)) {
            u8 |= CHARISLOWER;
        }
        if (isprint(i)) {
            u8 |= CHARISPRINT;
        }
        if (ispunct(i)) {
            u8 |= CHARISPUNCT;
        }
        if (isspace(i)) {
            u8 |= CHARISSPACE;
        }
        if (isupper(i)) {
            u8 |= CHARISUPPER;
        }
        fprintf(fp, (i < 255) ?  "    0x%x,\n" : "    0x%x\n", u8);
    }
    fprintf(fp, "};\n\n");

    return;
}

int
collcmp(const void *arg1, const void *arg2, const void *dummy)
{
    return strcoll(arg1, arg2);
}

void
printcoll(const char *typestr, const char *langstr, FILE *fp)
{
    int i;

    memset(collwtab, 0, sizeof(collwtab));
    memset(collwtmptab, 0, sizeof(collwtmptab));
    for (i = 0 ; i < 256 ; i++) {
        collwtab[i] = i;
    }
    qsort(collwtab, 256, sizeof(unsigned char), collcmp);
    fprintf(fp, "uint8_t stringcolltab_%s_%s[256]\n= {\n", typestr, langstr);
    for (i = 0 ; i < 256 ; i++) {
        collwtmptab[collwtab[i]] = i;
    }
    for (i = 0 ; i < 256 ; i++) {
        fprintf(fp, (i < 255) ? "    0x%x,\n" : "    0x%x\n", collwtmptab[i]);
    }
    fprintf(fp, "};\n\n");
}

int
main(int argc, char *argv[])
{
    int   i;
    int   j;
    FILE *fp1;
    FILE *fp2;

    i = 0;
    fp1 = fopen("bits/ctype.h", "w");
    fp2 = fopen("bits/string.h", "w");
    if ((fp1) && (fp2)) {
        while (ctypetab[i]) {
            setlocale(LC_CTYPE, ctypetab[i]);
            printctype(ctypepostfixtab[i], fp1);
            for (j = 0 ; j < NLANG ; j++) {
                setlocale(LC_COLLATE, colltab[j]);
                printcoll(ctypepostfixtab[i], collpostfixtab[j], fp2);
                j++;
            }
            i++;
        }
        fclose(fp1);
        fclose(fp2);
    }

    return 0;
}

