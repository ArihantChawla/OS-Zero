#include <stdio.h>
#include <stdint.h>
#include <locale.h>
#include <wchar.h>
#include <ctype.h>
#include <iconv.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CTYPE_SIZE 2
#define CTYPE_SUPPORT_NEGATIVE_INDICES 0

#define CTYPE_EOF  (-0xffff)
#define CTYPE_WEOF 0xffff
#if (CTYPE_SIZE == 2)
#   define CTYPE_ITEMS_PER_ROW 8
#   define CTYPE_PRINTF(i) printf("0x%0.4hx", i)
#elif (CTYPE_SIZE == 4)
#   define CTYPE_ITEMS_PER_ROW 4
#   define CTYPE_PRINTF(i) printf("0x%0.8hx", i)
#endif
#define CTYPETBL_DEBUG 0

static uint8_t checktbl[256];
static uint16_t ucstmp[256];
#if 0
static uint16_t ucscodes[384];
#endif

/* oc_FR, wa_BE: 8859-1:1987 */
/* cs_CZ: 8859-2:1987 */
/* az_AZ: 8859-9E */

static const char *names[] = {
    "ASCII",
    "ISO1",
    "ISO2",
    "ISO3",
    "ISO4",
    "ISO5",
    "ISO6",
    "ISO7",
    "ISO8",
    "ISO9",
    "ISO10",
    "ISO11",
    "ISO12",
    "ISO13",
    "ISO14",
    "ISO15",
    "ISO16",
    "ISO1_1987",
    "ISO2_1987",
    "ISO3_1988",
    "ISO4_1988",
    "ISO5_1988",
    "ISO6_1987",
    "ISO7_1987",
    "ISO8_1988",
    "ISO9_1989",
    "ISO10_1992",
    NULL,
    NULL,
    NULL,
    "ISO14_1998",
    "ISO15_1998",
    NULL,
    "TIS620", /* ISO IR-166 */
    "KOI8",
    "KOI8R",
    "KOI8T",
    "KOI8U",
};

static const char *charsets[] = {
    NULL,
    "ISO8859-1",
    "ISO8859-2",
    "ISO8859-3",
    "ISO8859-4",
    "ISO8859-5",
    "ISO8859-6",
    "ISO8859-7",
    "ISO8859-8",
    "ISO8859-9",
    "ISO8859-10",
    "ISO8859-11",
    "ISO8859-12",
    "ISO8859-13",
    "ISO8859-14",
    "ISO8859-15",
    "ISO8859-16",
    "ISO_8859-1:1987",
    "ISO_8859-2:1987",
    "ISO_8859-3:1988",
    "ISO_8859-4:1988",
    "ISO_8859-5:1988",
    "ISO_8859-6:1987",
    "ISO_8859-7:1987",
    "ISO_8859-8_1988",
    "ISO_8859-9:1989",
    "ISO_8859-10:1992",
    "ISO_8859-14:1998",
    "ISO_8859-15:1998"
};

#define CTYPE_ASCII              0
#define CTYPE_ISO_8859_1         1
#define CTYPE_ISO_8859_2         2
#define CTYPE_ISO_8859_3         3
#define CTYPE_ISO_8859_4         4
#define CTYPE_ISO_8859_5         5
#define CTYPE_ISO_8859_6         6
#define CTYPE_ISO_8859_7         7
#define CTYPE_ISO_8859_8         8
#define CTYPE_ISO_8859_9         9
#define CTYPE_ISO_8859_10        10
#define CTYPE_ISO_8859_11        11
#define CTYPE_ISO_8859_12        12
#define CTYPE_ISO_8859_13        13
#define CTYPE_ISO_8859_14        14
#define CTYPE_ISO_8859_15        15
#define CTYPE_ISO_8859_16        16
#define CTYPE_ISO_8859_1_1987    17
#define CTYPE_ISO_8859_2_1987    18
#define CTYPE_ISO_8859_3_1988    19
#define CTYPE_ISO_8859_4_1988    20
#define CTYPE_ISO_8859_5_1988    21
#define CTYPE_ISO_8859_6_1987    22
#define CTYPE_ISO_8859_7_1987    23
#define CTYPE_ISO_8859_8_1988    24
#define CTYPE_ISO_8859_9_1989    25
#define CTYPE_ISO_8859_10_1992   26
#define CTYPE_ISO_8859_11_UNUSED 27
#define CTYPE_ISO_8859_12_UNUSED 28
#define CTYPE_ISO_8859_13_UNUSED 29
#define CTYPE_ISO_8859_14_1998   30
#define CTYPE_ISO_8859_15_1998   31
#define CTYPE_ISO_8859_16_UNUSED 32
#define CTYPE_TIS620             33
#define CTYPE_KOI_8              34
#define CTYPE_KOI8_R             35
#define CTYPE_KOI8_T             36
#define CTYPE_KOI8_U             37

static const char *filenames[] = {
    NULL,
    "charmaps/ISO-8859-1",
    "charmaps/ISO-8859-2",
    "charmaps/ISO-8859-3",
    "charmaps/ISO-8859-4",
    "charmaps/ISO-8859-5",
    "charmaps/ISO-8859-6",
    "charmaps/ISO-8859-7",
    "charmaps/ISO-8859-8",
    "charmaps/ISO-8859-9",
    "charmaps/ISO-8859-10",
    "charmaps/ISO-8859-11",
    "charmaps/ISO-8859-12",
    "charmaps/ISO-8859-13",
    "charmaps/ISO-8859-14",
    "charmaps/ISO-8859-15",
    "charmaps/ISO-8859-16",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    "charmaps/TIS-620",
    "charmaps/KOI-8",
    "charmaps/KOI8-R",
    "charmaps/KOI8-T",
    "charmaps/KOI8-U"
};

#define __CTYPE_ALNUM_FLAG  0x00000001
#define __CTYPE_ALPHA_FLAG  0x00000002
#define __CTYPE_BLANK_FLAG  0x00000004
#define __CTYPE_CNTRL_FLAG  0x00000008
#define __CTYPE_GRAPH_FLAG  0x00000010
#define __CTYPE_LOWER_FLAG  0x00000020
#define __CTYPE_PRINT_FLAG  0x00000040
#define __CTYPE_PUNCT_FLAG  0x00000080
#define __CTYPE_SPACE_FLAG  0x00000100
#define __CTYPE_UPPER_FLAG  0x00000200
#define __CTYPE_XDIGIT_FLAG 0x00000400

char inbuf[512];
wchar_t outbuf[512];

void
xcreatetbl(int id)
{
    iconv_t cd;
    char *inptr;
    char *outptr;
    size_t insize;
    size_t outsize;
    size_t retsize;
    int i;

    retsize = -1;
    cd = 0;
    if (charsets[id]) {
        fprintf(stderr, "%s\n", charsets[id]);
        cd = iconv_open("WCHAR_T", charsets[id]);
    } else {
        fprintf(stderr, "no character set specified\n");

        return;
    }
    if ((size_t)cd == (size_t)-1) {
        fprintf(stderr, "conversion failure\n");

        return;
    }
    inptr = inbuf;
    outptr = (char *)outbuf;
    insize = 128;
    outsize = sizeof(wchar_t) * 512;
    for (i = 0 ; i < 256 ; i++) {
        inbuf[i] = i;
    }
    retsize = iconv(cd, &inptr, &insize, &outptr, &outsize);
    fprintf(stderr, "%lu irreversible conversions\n", retsize);

    return;
}

void
xparsetbl(int id)
{
    char string[4096];
    const char *filename;
    char *cp;
    FILE *fp;
    uint16_t *u16ptr;
    int ucsval, chval;
    int i, j;
    signed char c;

    filename = filenames[id];
    fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "failed to open %s\n", filename);
        xcreatetbl(id);

        return;
    }
    while (!feof(fp)) {
        fscanf(fp, "%s\n", string);
        if (string[0] == '<' && string[1] == 'U') {
            cp = &string[2];
            ucsval = 0;
            while (isxdigit(*cp)) {
                ucsval <<= 4;
                if (isdigit(*cp)) {
                    ucsval += *cp - '0';
                } else {
                    ucsval += 10 + toupper(*cp) - 'A';
                }
                cp++;
            }
            if (CTYPETBL_DEBUG) {
                fprintf(stderr, "%x - ", ucsval);
            }
            fscanf(fp, "%s\n", string);
            if (string[0] == '/' && string[1] == 'x') {
                cp = &string[2];
                chval = 0;
                while (isxdigit(*cp)) {
                    chval <<= 4;
                    if (isdigit(*cp)) {
                        chval += *cp - '0';
                    } else {
                        chval += 10 + toupper(*cp) - 'A';
                    }
                    cp++;
                }
                if (CTYPETBL_DEBUG) {
                    fprintf(stderr, "%x\n", chval);
                }
                checktbl[chval] = 1;
                ucstmp[chval] = ucsval;
            } else {
                fprintf(stderr, "/xXX expected\n");

                exit(1);
            }
        }
    }
#if 0
    u16ptr = &ucscodes[128];
    for (c = -128 ; c < 0 ; c++) {
        u16ptr[c] = ucstmp[(unsigned char)c];
    }
    for (i = 0 ; i < 256 ; i++) {
        u16ptr[i] = ucstmp[i];
    }
#endif
    j = 0;
#if (CTYPE_SUPPORT_NEGATIVE_INDICES)
    for (i = -128 ; i < 256 ; i++) {
        if (j == 0) {
            printf("    ");
        }
        CTYPE_PRINTF(u16ptr[i]);
        if (i != 0xff) {
            printf(",");
        } else {
            printf(" \\\n");
        }
        j++;
        if (j == CTYPE_ITEMS_PER_ROW && i != 0xff) {
            j = 0;
            printf(" \\\n");
        } else if (i != 0xff) {
            printf(" ");
        }
    }
#else
    for (i = 0 ; i < 256 ; i++) {
        if (j == 0) {
            printf("    ");
        }
        if (checktbl[i]) {
            CTYPE_PRINTF(ucstmp[i]);
        } else {
            CTYPE_PRINTF(i);
        }
        if (i != 0xff) {
            printf(",");
        } else {
            printf(" \\\n");
        }
        j++;
        if (j == CTYPE_ITEMS_PER_ROW && i != 0xff) {
            j = 0;
            printf(" \\\n");
        } else if (i != 0xff) {
            printf(" ");
        }
    }
#endif

    return;
}

void
xucs(int id) {
    int i, j;
    signed char c;

    printf("#define __CTYPE_8_%s_TOUCS_INITIALIZER \\\n{ \\\n", names[id]);
    if (id <= 1) {
#if (CTYPE_SUPPORT_NEGATIVE_INDICES)
        j = 0;
        for (c = -128 ; c < 0 ; c++) {
            if (j == 0) {
                printf("    ");
            }
            CTYPE_PRINTF((unsigned char)c);
            printf(", ");
            j++;
            if (j == CTYPE_ITEMS_PER_ROW) {
                j = 0;
                printf("\\\n");
            }
        }
#endif
        j = 0;
        for (i = 0 ; i <= 0x7f ; i++) {
            if (j == 0) {
                printf("    ");
            }
            CTYPE_PRINTF(i);
            printf(", ");
            j++;
            if (j == CTYPE_ITEMS_PER_ROW) {
                j = 0;
                printf("\\\n");
            }
        }
        j = 0;
        for (i = 0x7f + 1 ; i < 256 ; i++) {
            if (j == 0) {
                printf("    ");
            }
            if (id == 0) {
                CTYPE_PRINTF(CTYPE_WEOF);
                printf(", ");
            } else {
                CTYPE_PRINTF(i);
                printf(", ");
            }
            j++;
            if (j == CTYPE_ITEMS_PER_ROW) {
                j = 0;
                printf("\\\n");
            }
        }
    } else {
        xparsetbl(id);
    }
    printf("}\n");

    return;
}

void
xtolower(int id)
{
    int i, j;

    printf("#define __CTYPE_8_%s_TOLOWER_INITIALIZER \\\n{ \\\n", names[id]);
#if (CTYPE_SUPPORT_NEGATIVE_INDICES)
    j = 0;
    for (i = -128 ; i < 256 ; i++) {
        if (j == 0) {
            printf("    ");
        }
        CTYPE_PRINTF(tolower((unsigned char)i));
        if (i != 0xff) {
            printf(",");
        } else {
            printf(" \\\n");
        }
        j++;
        if (j == CTYPE_ITEMS_PER_ROW && i != 0xff) {
            j = 0;
            printf(" \\\n");
        } else if (i != 0xff) {
            printf(" ");
        }
    }
#else
    j = 0;
    for (i = 0 ; i < 256 ; i++) {
        if (j == 0) {
            printf("    ");
        }
        CTYPE_PRINTF(tolower((unsigned char)i));
        if (i != 0xff) {
            printf(",");
        } else {
            printf(" \\\n");
        }
        j++;
        if (j == CTYPE_ITEMS_PER_ROW && i != 0xff) {
            j = 0;
            printf(" \\\n");
        } else if (i != 0xff) {
            printf(" ");
        }
    }
#endif
    printf("}\n");
}

void
xtoupper(int id)
{
    int i, j;

    printf("#define __CTYPE_8_%s_TOUPPER_INITIALIZER \\\n{ \\\n", names[id]);
    j = 0;
#if (CTYPE_SUPPORT_NEGATIVE_INDICES)
    for (i = -128 ; i < 256 ; i++) {
        if (j == 0) {
            printf("    ");
        }
        CTYPE_PRINTF(toupper((unsigned char)i));
        if (i != 0xff) {
            printf(",");
        } else {
            printf(" \\\n");
        }
        j++;
        if (j == CTYPE_ITEMS_PER_ROW && i != 0xff) {
            j = 0;
            printf(" \\\n");
        } else if (i != 0xff) {
            printf(" ");
        }
    }
#else
    for (i = 0 ; i < 256 ; i++) {
        if (j == 0) {
            printf("    ");
        }
        CTYPE_PRINTF(toupper((unsigned char)i));
        if (i != 0xff) {
            printf(",");
        } else {
            printf(" \\\n");
        }
        j++;
        if (j == CTYPE_ITEMS_PER_ROW && i != 0xff) {
            j = 0;
            printf(" \\\n");
        } else if (i != 0xff) {
            printf(" ");
        }
    }
#endif
    printf("}\n");
}

uint32_t
xchkflags(int i)
{
    uint32_t u32f = 0;
    unsigned char uc;

    uc = (unsigned char)i;
    if (isalnum(uc)) {
        u32f |= __CTYPE_ALNUM_FLAG;
    }
    if (isalpha(uc)) {
        u32f |= __CTYPE_ALPHA_FLAG;
    }
    if (isblank(uc)) {
        u32f |= __CTYPE_BLANK_FLAG;
    }
    if (iscntrl(uc)) {
        u32f |= __CTYPE_CNTRL_FLAG;
    }
    if (isgraph(uc)) {
        u32f |= __CTYPE_GRAPH_FLAG;
    }
    if (islower(uc)) {
        u32f |= __CTYPE_LOWER_FLAG;
    }
    if (isprint(uc)) {
        u32f |= __CTYPE_PRINT_FLAG;
    }
    if (ispunct(uc)) {
        u32f |= __CTYPE_PUNCT_FLAG;
    }
    if (isspace(uc)) {
        u32f |= __CTYPE_SPACE_FLAG;
    }
    if (isupper(uc)) {
        u32f |= __CTYPE_UPPER_FLAG;
    }
    if (isxdigit(uc)) {
        u32f |= __CTYPE_XDIGIT_FLAG;
    }

    return u32f;
}

void
xflags(int id)
{
    int i, j;

    printf("#define __CTYPE_8_%s_FLAGS_INITIALIZER \\\n{ \\\n", names[id]);
    j = 0;
#if (CTYPE_SUPPORT_NEGATIVE_INDICES)
    for (i = -128 ; i < 256 ; i++) {
        if (j == 0) {
            printf("    ");
        }
        CTYPE_PRINTF(xchkflags(i));
        if (i != 0xff) {
            printf(",");
        } else {
            printf(" \\\n");
        }
        j++;
        if (j == CTYPE_ITEMS_PER_ROW && i != 0xff) {
            j = 0;
            printf(" \\\n");
        } else if (i != 0xff) {
            printf(" ");
        }
    }
#else
    for (i = 0 ; i < 256 ; i++) {
        if (j == 0) {
            printf("    ");
        }
        CTYPE_PRINTF(xchkflags(i));
        if (i != 0xff) {
            printf(",");
        } else {
            printf(" \\\n");
        }
        j++;
        if (j == CTYPE_ITEMS_PER_ROW && i != 0xff) {
            j = 0;
            printf(" \\\n");
        } else if (i != 0xff) {
            printf(" ");
        }
    }
#endif
    printf("}\n");
}

void
xtables(int id)
{
    xucs(id);
    printf("\n");
    xtolower(id);
    printf("\n");
    xtoupper(id);
    printf("\n");
    xflags(id);
    printf("\n");
#if 0
    xcoll(id);
    printf("\n");
#endif

    return;
}

int
main(int argc, char *argv[])
{
    char *locname;
    char *arg;
    int charset;
    int i;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <ascii | 8859-x>\n", argv[0]);

        exit(1);
    }
    locname = NULL;
    arg = argv[1];
    charset = 0;
    if (!strcmp(arg, "ascii")) {
        locname = setlocale(LC_CTYPE, "C");
        charset = 0;
    } else if (!strcmp(arg, "8859-1")) {
        locname = setlocale(LC_CTYPE, "fi_FI");
        charset = 1;
    } else if (!strcmp(arg, "8859-2")) {
        locname = setlocale(LC_CTYPE, "bs_BA");
        charset = 2;
    } else if (!strcmp(arg, "8859-5")) {
        locname = setlocale(LC_CTYPE, "ru_RU");
        charset = 5;
    } else if (!strcmp(arg, "8859-6")) {
        locname = setlocale(LC_CTYPE, "ar_SA");
        charset = 6;
    } else if (!strcmp(arg, "8859-7")) {
        locname = setlocale(LC_CTYPE, "el_GR");
        charset = 7;
    } else if (!strcmp(arg, "8859-8")) {
        locname = setlocale(LC_CTYPE, "he_IL");
        charset = 8;
    } else if (!strcmp(arg, "8859-9")) {
        locname = setlocale(LC_CTYPE, "tr_TR");
        charset = 9;
    } else if (!strcmp(arg, "8859-10")) {
        locname = setlocale(LC_CTYPE, "lug_UG");
        charset = 10;
    } else if (!strcmp(arg, "8859-13")) {
        locname = setlocale(LC_CTYPE, "mi_NZ");
        charset = 13;
    } else if (!strcmp(arg, "8859-14")) {
        locname = setlocale(LC_CTYPE, "cy_GB");
        charset = 14;
    } else if (!strcmp(arg, "8859-15")) {
        locname = setlocale(LC_CTYPE, "fi_FI@euro");
        charset = 15;
#if 0
    } else if (!strcmp(arg, "8859-1_1987")) {
        locname = setlocale(LC_CTYPE, "oc_FR");
        charset = 17;
    } else if (!strcmp(arg, "8859-2_1987")) {
        locname = setlocale(LC_CTYPE, "cs_CZ");
        charset = 18;
#endif
#if 0
    } else if (!strcmp(arg, "koi8")) {
#endif
    } else if (!strcmp(arg, "tis620")) {
        locname = setlocale(LC_CTYPE, "th_TH");
        charset = 33;
    } else if (!strcmp(arg, "koi8-r")) {
        locname = setlocale(LC_CTYPE, "bg_BG@KOI8-R");
        charset = 35;
    } else if (!strcmp(arg, "koi8-t")) {
        locname = setlocale(LC_CTYPE, "tg_TJ");
        charset = 36;
    } else if (!strcmp(arg, "koi8-u")) {
        locname = setlocale(LC_CTYPE, "ru_UA");
        charset = 37;
    } else {
        fprintf(stderr, "unsupported charset: %s\n", arg);

        exit(1);
    }
    if (locname) {
        xtables(charset);
        fprintf(stderr, "LOCALE: %s\n", locname);
    } else {
        fprintf(stderr, "invalid locale\n");

        exit(1);
    }

    exit(0);
}

