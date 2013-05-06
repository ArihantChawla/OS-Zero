#include <stdio.h>
#include <stdint.h>
#include <locale.h>

#define COLLATE_ITEMS_PER_ROW_1 8
#define COLLATE_ITEMS_PER_ROW   4
#define COLLATE_PRINTF_1(i) printf("0x%0.2hx", i)

#if 0
static uint8_t colltbl0[256];
static uint8_t weighttbl0[256];
#endif

static int32_t colltbl1[256];
static uint8_t weighttbl1[256];

static int32_t colltbl2[65536];
static int16_t weighttbl2[65536];
#if 0
static int32_t colltbl[65536];
static uint16_t weighttbl[65536];
#endif

#if 0
const char *names[] = {
    "C",
    "ISO14651T1",
    "FINNISH"
};
#endif

#if 0
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
#endif

#if 0
const char *names[] = {
    "ISO14651T1",
    "ISO14652T2"
};

const char *locales[] = {
    "V_14651",
    "V_14652"
};
#endif

#if 0
const char *names[] = {
    "C",
    "iso14651_t1",
    "finnish",
    "canadien",
    "spanish",
    "estonian",
    "danish",
    "icelandic",
    "norwegian",
    "croatian",
    "hungarian",
    "polish",
    "romanian",
    "czech",
    "welsh",
    "isoir166"
    "koi8r",
    "koi8t",
    "koi8u"
};
#endif

const char *__collatelocnames1[] = {
    "fi_FI", /* also sv_FI, fi_FI@euro, sv_FI@euro */
    "sv_SE",
    NULL
};

#if 0
struct __collateloc __collatelocs[] = {
    { "en_DK", 1 },
    { "fi_FI", 1 },
    { NULL, -1 }
};
#endif

#if 0
struct __collateloc __collatelocs[] = {
    "C",
    "en_DK",
    "fi_FI",
    "en_CA",
    "es_ES",
    "et_EE",
    "fo_FO",
    "is_IS",
    "no_NO",
    "hr_HR",
    "hu_HU",
    "pl_PL",
    "ro_RO",
};
#endif

#if 0
struct __collateloc __loccollates[] = {
    { "C" , 0 },
    { "af_ZA", 1 },
    { "br_FR", 1 },
    { "ca_ES", 3 },
    { "da_DK", 1 },
    { "de_AT", 1 },
    { "de_BE", 1 },
    { "de_CH", 1 },
    { "de_LU", 1 },
    { "en_AU", 1 },
    { "en_BW", 1 },
    { "en_CA", 3 },
    { "en_DK", 1 },
    { "en_GB", 1 },
    { "en_IE", 1 },
    { "en_NZ", 1 },
    { "en_ZA", 1 },
    { "en_ZW", 1 },
#if 0
    { "es_AR", 4 },
    { "es_BO", 4 },
    { "es_CL", 4 },
    { "es_CO", 4 },
    { "es_CR", 4 },
    { "es_DO", 4 },
    { "es_EC", 4 },
    { "es_ES", 4 },
    { "es_GT", 4 },
    { "es_HN", 4 },
    { "es_MX", 4 },
    { "es_NI", 4 },
    { "es_PA", 4 },
    { "es_PE", 4 },
    { "es_PR", 4 },
    { "es_PY", 4 },
    { "es_SV", 4 },
    { "es_US", 4 },
    { "es_UY", 4 },
    { "es_VE", 4 },
#endif
    { "et_EE", 5 },
    { "eu_ES", 1 },
    { "fi_FI", 2 },
#if 0
    { "fo_FO", 6 },
#endif
    { "fr_BE", 1 },
    { "fr_CA", 3 },
    { "fr_CH", 1 },
    { "fr_FR", 1 },
    { "fr_LU", 1 },
    { "ga_IE", 1 },
    { "gd_GB", 1 },
#if 0
    { "gl_ES", 4 },
#endif
    { "gv_GB", 1 },
    { "id_ID", 1 },
#if 0
    { "is_IS", 7 },
#endif
    { "it_CH", 1 },
    { "it_IT", 1 },
    { "kl_GL", 1 },
    { "kw_GB", 1 },
    { "nl_BE", 1 },
    { "nl_NL", 1 },
    { "no_NO", 8 },
    { "pt_BR", 1 },
    { "pt_PT", 1 },
    { "sv_FI", 2 },
    { "sv_SE", 1 },
    { "tl_PH", 1 },
    { "uz_UZ", 1 },
    { "br_FR@euro", 1 },
    { "bs_BA", 1 },
#if 0
    { "hr_HR", 9 },
    { "hu_HU", 10 },
    { "pl_PL", 11 },
    { "ro_RO", 12 },
    { "sk_SK", 13 },
#endif
    { "sl_SI", 2 },
    { "sr_YU", 2 },
    { "mk_MK", 5 },
    { "sr_YU@cyrillic", 5 },
    { "ru_RU", 5 },
    { "ar_SA", 6 },
    { "el_GR", 7 },
    { "he_IL", 8 },
    { "iw_IL", 8 },
    { "tr_TR", 9 },
    { "lug_UG", 10 },
    { "mi_NZ", 13 },
    { "cy_GB", 14 },
//    { "br_FR@euro", 15 },
    { "ca_ES@euro", 3 },
    { "de_AT@euro", 1 },
    { "de_BE@euro", 1 },
    { "de_LU@euro", 1 },
    { "en_IE@euro", 1 },
    { "es_ES@euro", 4 },
    { "eu_ES@euro", 1 },
    { "fi_FI@euro", 2 },
    { "fr_BE@euro", 1 },
    { "fr_FR@euro", 1 },
    { "fr_LU@euro", 1 },
    { "ga_IE@euro", 1 },
//    { "ga_GB", 15 },
    { "gl_ES@euro", 4 },
    { "it_IT@euro", 1 },
    { "nl_BE@euro", 1 },
    { "nl_NL@euro", 1 },
    { "pt_PT@euro", 1 },
    { "sv_FI@euro", 2 },
    { "wa_BE@euro", 1 },
#if 0
    { "oc_FR", 17 },
    { "wa_BE", 17 },
    { "cs_CZ", 18 },
    { "az_AZ", 19 },
#endif
    { "th_TH", 15 },
    { "bg_BG@KOI8-R", 16 },
    { "tg_TJ", 17 },
    { "ru_UA", 18 },
    { "uk_UA", 18 },
    { NULL, -1 }
};
#endif

int
foocoll(const void *str1, const void *str2)
{
    int retval;

    retval = strcoll((char *)str1, (char *)str2);

    return retval;
}

void
xcoll1(int id)
{
    int i, j;
    int32_t val;
    int32_t ndx;

    if (setlocale(LC_COLLATE, __collatelocnames1[id])) {
        fprintf(stderr, "%s\n", __collatelocnames1[id]);
        for (val = 0 ; val < 256 ; val++) {
            colltbl1[val] = val;
        }
        qsort(colltbl1, 256, sizeof(int32_t), foocoll);
        for (val = 0 ; val < 256 ; val++) {
            ndx = colltbl1[val];
            weighttbl1[ndx] = val;
        }
        printf("#define __CTYPE_8_%s_WEIGHT_1_INITIALIZER \\\n{ \\\n",
               __collatelocnames1[id]);
        j = 0;
        for (i = 0 ; i < 256 ; i++) {
            if (j == 0) {
                printf("    ");
            }
            COLLATE_PRINTF_1(weighttbl1[i]);
            if (i != 0xff) {
                printf(",");
            } else {
                printf(" \\\n");
            }
            j++;
            if (j == COLLATE_ITEMS_PER_ROW_1 && i != 0xff) {
                j = 0;
                printf(" \\\n");
            } else if (i != 0xff) {
                printf(" ");
            }
        }
        printf("}\n");
    } else {
        fprintf(stderr, "failed to set LC_COLLATE to %s\n",
                __collatelocnames1[id]);
    }
}

#if 0
void
xcoll2(const char *locname)
{
    int i, j;
    int32_t val;
    int32_t ndx;

    if (setlocale(LC_COLLATE, locname)) {
        fprintf(stderr, "%s\n", locname);
        for (ndx = 0 ; ndx < 256 ; ndx++) {
            colltbl2[ndx] = ndx;
        }
        qsort(colltbl2, 256, sizeof(int32_t), foocoll);
        for (val = 0 ; val < 256 ; val++) {
            ndx = colltbl[val];
            weighttbl[ndx] = val;
        }
        printf("#define __CTYPE_8_%s_WEIGHT_2_INITIALIZER \\\n{ \\\n",
               names[id]);
        j = 0;
        for (i = 0 ; i < 256 ; i++) {
            if (j == 0) {
                printf("    ");
            }
            COLLATE_PRINTF_2(weighttbl1[i]);
            if (i != 0xff) {
                printf(",");
            } else {
                printf(" \\\n");
            }
            j++;
            if (j == COLLATE_ITEMS_PER_ROW_2 && i != 0xff) {
                j = 0;
                printf(" \\\n");
            } else if (i != 0xff) {
                printf(" ");
            }
        }
        printf("}\n");
        }
    } else {
        fprintf(stderr, "failed to set LC_COLLATE to %s\n", locname);
    }
}
#endif

#if 0
void
xcoll1(int id)
{
    int i, j, max;
    uint16_t u16;
    uint16_t ndx;

    if (setlocale(LC_COLLATE, __collatelocs[id].name)) {
        fprintf(stderr, "%s\n", __collatelocs[id].name);
        if (id == 0) {
            for (u16 = 0 ; u16 < 0x80 ; u16++) {
                colltbl[u16] = u16;
            }
            qsort(colltbl, 128, sizeof(u16), foocoll);
            for (u16 = 0 ; u16 < 0x80 ; u16++) {
                ndx = colltbl[u16];
                weighttbl[ndx] = u16;
            }
            for (u16 = 0x80 ; u16 < 256 ; u16++) {
                weighttbl[u16] = u16;
            }
        } else {
            for (u16 = 0 ; u16 < 256 ; u16++) {
                colltbl[u16] = u16;
            }
            qsort(colltbl, 256, sizeof(u16), foocoll);
            for (u16 = 0 ; u16 < 256 ; u16++) {
                ndx = colltbl[u16];
                weighttbl[ndx] = u16;
            }
        }
        printf("#define __CTYPE_8_%s_WEIGHT_INITIALIZER \\\n{ \\\n",
               __collatelocs[id].name);
        j = 0;
        for (i = 0 ; i < 256 ; i++) {
            if (j == 0) {
                printf("    ");
            }
            COLLATE_PRINTF(weighttbl[i]);
            if (i != 0xff) {
                printf(",");
            } else {
                printf(" \\\n");
            }
            j++;
            if (j == COLLATE_ITEMS_PER_ROW && i != 0xff) {
                j = 0;
                printf(" \\\n");
            } else if (i != 0xff) {
                printf(" ");
            }
        }
        printf("}\n");
    } else {
        fprintf(stderr, "cannot set locale to %s\n", __collatelocs[id].name);

        return;
    }

    return;
}
#endif

#if 0
void
xcoll(int id)
{
    int i, j, max;
    int32_t i32;
    int32_t val;

//    if (setlocale(LC_COLLATE, "V_14651") && setlocale(LC_CTYPE, "en_DK")) {
    if (setlocale(LC_ALL, "en_DK")) {
        fprintf(stderr, "%s\n", __collatelocs[0].name);
        for (i32 = 0 ; i32 < 65536 ; i32++) {
            colltbl[i32] = i32;
        }
        qsort(colltbl, 65536, sizeof(int32_t), foocoll);
        for (i32 = 0 ; i32 < 65536 ; i32++) {
            val = colltbl[i32];
            weighttbl[val] = i32;
        }
        printf("#define __CTYPE_16_%s_WEIGHT_INITIALIZER \\\n{ \\\n", names[id]);
        j = 0;
        for (i = 0 ; i < 65536 ; i++) {
            val = weighttbl[i];
            if (j == 0) {
                printf("    ");
            }
            COLLATE_PRINTF(weighttbl[i]);
            if (i != 0xffff) {
                printf(",");
            } else {
                printf(" \\\n");
            }
            j++;
            if (j == COLLATE_ITEMS_PER_ROW && i != 0xffff) {
                j = 0;
                printf(" \\\n");
            } else if (i != 0xffff) {
                printf(" ");
            }
        }
        printf("}\n");
    } else {
        fprintf(stderr, "cannot set locale to %s\n", "i18n");

        return;
    }

    return;
}
#endif

int
main(int argc, char *argv[])
{
    char *arg;
    int i;

#if 0
    for (i = 0 ; (__collatelocnames1[i]) ; i++) {
        xcoll1(i);
    }
#endif
    if (argc != 2) {
        fprintf(stderr, "usage: %s <fi|sv>\n", argv[0]);

        exit (1);
    }
    arg = argv[1];
    if (!strcmp(arg, "fi")) {
        xcoll1(0);
    } else if (!strcmp(arg, "sv")) {
        xcoll1(1);
    } else {
        fprintf(stderr, "usage: %s <fi|sv>\n", argv[0]);

        exit(1);
    }
    printf("\n");

    exit(0);
}

