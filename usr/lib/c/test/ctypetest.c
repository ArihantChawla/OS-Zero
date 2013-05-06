#include <Tornado/main.h>
#include <ctype.h>
#include <locale.h>
#include <sys/resource.h>

#define CTYPE_SUPPORT_NEGATIVE_INDICES 0

int32_t colltbl[65536];
int32_t weighttbl[65536];

/* types */
typedef uint16_t __ctypeconv_t;
typedef uint16_t __ctypeflag_t;
typedef uint16_t __ctypesize_t;
typedef uint16_t __ctypeweight_t;

struct __ctypeptrs {
    const __ctypeconv_t *toucstbl;
    const __ctypeconv_t *tolowertbl;
    const __ctypeconv_t *touppertbl;
    const __ctypeflag_t *typeflags;
    const __ctypesize_t *utf8lentbl;
    const __ctypesize_t *utf8sizetbl;
    const __ctypeweight_t *weighttbl;
};

extern struct __ctypeptrs __ctypelocale;
#if 0
extern uint16_t __iso14651t1weighttbl[65536];
#endif
#define __testisascii(c) (((c) & ~0x7f) == 0)
#define __testisdigit(c) (((c) >= '0') && ((c) <= '9'))
#define __toascii(c) ((c) & 0x7f)

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

#define __CTYPE_VALID_CHAR(c) (((c) >= -128) && ((c) <= 255))

#if (CTYPE_SUPPORT_NEGATIVE_INDICES)
#define __testisalnum(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(c)] & __CTYPE_ALNUM_FLAG) \
     : 0)
#define __testisalpha(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(c)] & __CTYPE_ALPHA_FLAG) \
     : 0)
#define __testisblank(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(c)] & __CTYPE_BLANK_FLAG) \
     : 0)
#define __testiscntrl(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(c)] & __CTYPE_CNTRL_FLAG) \
     : 0)
#define __testisgraph(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(c)] & __CTYPE_GRAPH_FLAG) \
     : 0)
#define __testislower(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(c)] & __CTYPE_LOWER_FLAG) \
     : 0)
#define __testisprint(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(c)] & __CTYPE_PRINT_FLAG) \
     : 0)
#define __testispunct(c) \
    ((__CTYPE_VALID_CHAR(c)) \
     ? (__ctypelocale.typeflags[(c)] & __CTYPE_PUNCT_FLAG) \
     : 0)
#define __testisspace(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(c)] & __CTYPE_SPACE_FLAG) \
     : 0)
#define __testisupper(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(c)] & __CTYPE_UPPER_FLAG) \
     : 0)
#define __testisxdigit(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(c)] & __CTYPE_XDIGIT_FLAG) \
     : 0)

#define __tolower(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.tolowertbl[(c)]) \
     : (c))
#define __toupper(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.touppertbl[(c)]) \
     : (c))
#else
#define __testisalnum(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(unsigned char)(c)] & __CTYPE_ALNUM_FLAG) \
     : 0)
#define __testisalpha(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(unsigned char)(c)] & __CTYPE_ALPHA_FLAG) \
     : 0)
#define __testisblank(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(unsigned char)(c)] & __CTYPE_BLANK_FLAG) \
     : 0)
#define __testiscntrl(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(unsigned char)(c)] & __CTYPE_CNTRL_FLAG) \
     : 0)
#define __testisgraph(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(unsigned char)(c)] & __CTYPE_GRAPH_FLAG) \
     : 0)
#define __testislower(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(unsigned char)(c)] & __CTYPE_LOWER_FLAG) \
     : 0)
#define __testisprint(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(unsigned char)(c)] & __CTYPE_PRINT_FLAG) \
     : 0)
#define __testispunct(c) \
    ((__CTYPE_VALID_CHAR(c)) \
     ? (__ctypelocale.typeflags[(unsigned char)(c)] & __CTYPE_PUNCT_FLAG) \
     : 0)
#define __testisspace(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(unsigned char)(c)] & __CTYPE_SPACE_FLAG) \
     : 0)
#define __testisupper(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(unsigned char)(c)] & __CTYPE_UPPER_FLAG) \
     : 0)
#define __testisxdigit(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.typeflags[(unsigned char)(c)] & __CTYPE_XDIGIT_FLAG) \
     : 0)

#define __tolower(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.tolowertbl[(unsigned char)(c)]) \
     : (c))
#define __toupper(c) \
    (__CTYPE_VALID_CHAR(c) \
     ? (__ctypelocale.touppertbl[(unsigned char)(c)]) \
     : (c))
#endif

#if 0
static const char *locnames[] = {
    "C",
    "fi_FI",
    "bs_BA",
    NULL,
    NULL,
    "ru_RU",
    "ar_SA",
    "el_GR",
    "he_IL",
    "tr_TR",
    "lug_UG",
    NULL,
    NULL,
    "mi_NZ",
    "cy_GB",
    "fi_FI@euro",
    NULL
};
#endif

#if 0
uint16_t colltbl[256];
uint16_t weighttbl[256];
#endif

struct __ctypeloc {
    const char *name;
    int id;
};

struct __ctypeloc __ctypelocs[] = {
    { "C" , 0 },
    { "af_ZA", 1 },
    { "br_FR", 1 },
    { "ca_ES", 1 },
    { "da_DK", 1 },
    { "de_AT", 1 },
    { "de_BE", 1 },
    { "de_CH", 1 },
    { "de_LU", 1 },
    { "en_AU", 1 },
    { "en_BW", 1 },
    { "en_CA", 1 },
    { "en_DK", 1 },
    { "en_GB", 1 },
    { "en_IE", 1 },
    { "en_NZ", 1 },
    { "en_ZA", 1 },
    { "en_ZW", 1 },
    { "es_AR", 1 },
    { "es_BO", 1 },
    { "es_CL", 1 },
    { "es_CO", 1 },
    { "es_CR", 1 },
    { "es_DO", 1 },
    { "es_EC", 1 },
    { "es_ES", 1 },
    { "es_GT", 1 },
    { "es_HN", 1 },
    { "es_MX", 1 },
    { "es_NI", 1 },
    { "es_PA", 1 },
    { "es_PE", 1 },
    { "es_PR", 1 },
    { "es_PY", 1 },
    { "es_SV", 1 },
    { "es_US", 1 },
    { "es_UY", 1 },
    { "es_VE", 1 },
    { "et_EE", 1 },
    { "eu_ES", 1 },
    { "fi_FI", 1 },
    { "fo_FO", 1 },
    { "fr_BE", 1 },
    { "fr_CA", 1 },
    { "fr_CH", 1 },
    { "fr_FR", 1 },
    { "fr_LU", 1 },
    { "ga_IE", 1 },
    { "gd_GB", 1 },
    { "gl_ES", 1 },
    { "gv_GB", 1 },
    { "id_ID", 1 },
    { "is_IS", 1 },
    { "it_CH", 1 },
    { "it_IT", 1 },
    { "kl_GL", 1 },
    { "kw_GB", 1 },
    { "nl_BE", 1 },
    { "nl_NL", 1 },
    { "no_NO", 1 },
    { "pt_BR", 1 },
    { "pt_PT", 1 },
    { "sv_FI", 1 },
    { "sv_SE", 1 },
    { "tl_PH", 1 },
    { "uz_UZ", 1 },
    { "br_FR@euro", 1 },
    { "bs_BA", 2 },
    { "hr_HR", 2 },
    { "hu_HU", 2 },
    { "pl_PL", 2 },
    { "ro_RO", 2 },
    { "sk_SK", 2 },
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
    { "ca_ES@euro", 15 },
    { "de_AT@euro", 15 },
    { "de_BE@euro", 15 },
    { "de_LU@euro", 15 },
    { "en_IE@euro", 15 },
    { "es_ES@euro", 15 },
    { "eu_ES@euro", 15 },
    { "fi_FI@euro", 15 },
    { "fr_BE@euro", 15 },
    { "fr_FR@euro", 15 },
    { "fr_LU@euro", 15 },
    { "ga_IE@euro", 15 },
    { "ga_GB", 15 },
    { "gl_ES@euro", 15 },
    { "it_IT@euro", 15 },
    { "nl_BE@euro", 15 },
    { "nl_NL@euro", 15 },
    { "pt_PT@euro", 15 },
    { "sv_FI@euro", 15 },
    { "wa_BE@euro", 15 },
#if 0
    { "oc_FR", 17 },
    { "wa_BE", 17 },
    { "cs_CZ", 18 },
    { "az_AZ", 19 },
#endif
    { "th_TH", 33 },
    { "bg_BG@KOI8-R", 35 },
    { "tg_TJ", 36 },
    { "ru_UA", 37 },
    { "uk_UA", 37 },
    { NULL, -1 }
};

/* oc_FR, wa_BE: 8859-1:1987 */
/* cs_CZ: 8859-2:1987 */
/* az_AZ: 8859-9E */

void
xtest(const char *locname, int charset)
{
    int val1, val2;
    int i;

    if (!setlocale(LC_CTYPE, locname)) {
        fprintf(stderr, "cannot set locale to %s\n", locname);

        return;
    }
    __setctype(charset);
    for (i = -128 ; i < -1 ; i++) {
        val1 = (isalnum(i) == 0);
        val2 = (__testisalnum(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isalnum(%s/%x, %d): %d, %d\n",
                    locname, (unsigned char)i, charset, val1, val2);
        }
        val1 = (isalnum(i) == 0);
        val2 = (__testisalnum(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isalpha()\n");
        }
        val1 = (isalnum(i) == 0);
        val2 = (__testisalnum(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isblank()\n");
        }
        val1 = (isgraph(i) == 0);
        val2 = (__testisgraph(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isgraph()\n");
        }
        val1 = (islower(i) == 0);
        val2 = (__testislower(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: islower()\n");
        }
        val1 = (isprint(i) == 0);
        val2 = (__testisprint(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isprint()\n");
        }
        val1 = (ispunct(i) == 0);
        val2 = (__testispunct(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: ispunct(%s - %d): %d, %d\n",
                    locname, i, val1, val2);
        }
        val1 = (isspace(i) == 0);
        val2 = (__testisspace(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isspace(%d): %d, %d\n", i, val1, val2);
        }
        val1 = (isupper(i) == 0);
        val2 = (__testisupper(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isupper()\n");
        }
        val1 = (isxdigit(i) == 0);
        val2 = (__testisxdigit(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isxdigit()\n");
        }

        val1 = (isascii(i) == 0);
        val2 = (__testisascii(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isascii()\n");
        }
        val1 = (iscntrl(i) == 0);
        val2 = (__testiscntrl(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "%d - %d\n", iscntrl(i), __testiscntrl(i));
            fprintf(stderr, "ERROR: iscntrl(%s:%d), %x, %x\n",
                locname, i, val1, val2);
        }
        val1 = (isdigit(i) == 0);
        val2 = (__testisdigit(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isdigit()\n");
        }
        val1 = (toascii(i) == 0);
        val2 = (__toascii(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: toascii()\n");
        }

        if (tolower(i) != __tolower(i)) {
            fprintf(stderr, "ERROR: tolower(%d)\n", i);
        }
        if (toupper(i) != __toupper(i)) {
            fprintf(stderr, "ERROR: toupper(%d)\n", i);
        }
    }

    val1 = (isalnum(-1) == 0);
    val2 = (__testisalnum(-0xffff) == 0);
    if (val1 != val2) {
        fprintf(stderr, "ERROR: isalnum(%s/%d, %d): %d, %d\n",
                locname, i, charset, val1, val2);
        
        exit(1);
    }
    val1 = (isalnum(-1) == 0);
    val2 = (__testisalnum(-0xffff) == 0);
    if (val1 != val2) {
        fprintf(stderr, "ERROR: isalpha()\n");
        
        exit(1);
    }
    val1 = (isalnum(-1) == 0);
    val2 = (__testisalnum(-0xffff) == 0);
    if (val1 != val2) {
        fprintf(stderr, "ERROR: isblank()\n");
        
        exit(1);
    }
    val1 = (isgraph(-1) == 0);
    val2 = (__testisgraph(-0xffff) == 0);
    if (val1 != val2) {
        fprintf(stderr, "ERROR: isgraph()\n");
        
        exit(1);
    }
    val1 = (islower(-1) == 0);
    val2 = (__testislower(-0xffff) == 0);
    if (val1 != val2) {
        fprintf(stderr, "ERROR: islower()\n");
        
        exit(1);
    }
    val1 = (isprint(-1) == 0);
    val2 = (__testisprint(-0xffff) == 0);
    if (val1 != val2) {
        fprintf(stderr, "ERROR: isprint()\n");
        
        exit(1);
    }
    val1 = (ispunct(-1) == 0);
    val2 = (__testispunct(-0xffff) == 0);
    if (val1 != val2) {
        fprintf(stderr, "ERROR: ispunct(%s - %d): %d, %d\n",
                locname, i, val1, val2);
        
        exit(1);
    }
    val1 = (isspace(-1) == 0);
    val2 = (__testisspace(-0xffff) == 0);
    if (val1 != val2) {
        fprintf(stderr, "ERROR: isspace(%d): %d, %d\n", i, val1, val2);
        
        exit(1);
    }
    val1 = (isupper(-1) == 0);
    val2 = (__testisupper(-0xffff) == 0);
    if (val1 != val2) {
        fprintf(stderr, "ERROR: isupper()\n");
        
        exit(1);
    }
    val1 = (isxdigit(-1) == 0);
    val2 = (__testisxdigit(-0xffff) == 0);
    if (val1 != val2) {
        fprintf(stderr, "ERROR: isxdigit()\n");
        
        exit(1);
    }
    
    val1 = (isascii(-1) == 0);
    val2 = (__testisascii(-0xffff) == 0);
    if (val1 != val2) {
        fprintf(stderr, "ERROR: isascii()\n");
        
        exit(1);
    }
    val1 = (iscntrl(-1) == 0);
    val2 = (__testiscntrl(-0xffff) == 0);
    if (val1 != val2) {
        fprintf(stderr, "%d - %d\n", iscntrl(i), __testiscntrl(i));
        fprintf(stderr, "ERROR: iscntrl(%s:%d), %x, %x\n",
                locname, i, val1, val2);
        
        exit(1);
    }
    val1 = (isdigit(-1) == 0);
    val2 = (__testisdigit(-0xffff) == 0);
    if (val1 != val2) {
        fprintf(stderr, "ERROR: isdigit()\n");
        
        exit(1);
    }
    val1 = (toascii(-1) == 0);
    val2 = (__toascii(-0xffff) == 0);
    if (val1 != val2) {
        fprintf(stderr, "ERROR: toascii()\n");
        
        exit(1);
    }

#if 0    
    val1 = tolower(-1);
    val2 = __tolower(-1);
    if (val1 != val2) {
        fprintf(stderr, "ERROR: tolower(%d): %d, %d\n", i, val1, val2);
    }
    val1 = toupper(-1);
    val2 = __toupper(-1);
    if (val1 != val2) {
        fprintf(stderr, "ERROR: toupper(%d): %d, %d\n", i, val1, val2);
    }
#endif
    
    for (i = 0 ; i < 256 ; i++) {
        val1 = (isalnum(i) == 0);
        val2 = (__testisalnum(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isalnum(%s/%d, %d): %d, %d\n",
                    locname, i, charset, val1, val2);

            exit(1);
        }
        val1 = (isalnum(i) == 0);
        val2 = (__testisalnum(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isalpha()\n");

            exit(1);
        }
        val1 = (isalnum(i) == 0);
        val2 = (__testisalnum(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isblank()\n");

            exit(1);
        }
        val1 = (isgraph(i) == 0);
        val2 = (__testisgraph(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isgraph()\n");

            exit(1);
        }
        val1 = (islower(i) == 0);
        val2 = (__testislower(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: islower()\n");

            exit(1);
        }
        val1 = (isprint(i) == 0);
        val2 = (__testisprint(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isprint()\n");

            exit(1);
        }
        val1 = (ispunct(i) == 0);
        val2 = (__testispunct(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: ispunct(%s - %d): %d, %d\n",
                    locname, i, val1, val2);

            exit(1);
        }
        val1 = (isspace(i) == 0);
        val2 = (__testisspace(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isspace(%d): %d, %d\n", i, val1, val2);

            exit(1);
        }
        val1 = (isupper(i) == 0);
        val2 = (__testisupper(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isupper()\n");

            exit(1);
        }
        val1 = (isxdigit(i) == 0);
        val2 = (__testisxdigit(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isxdigit()\n");

            exit(1);
        }

        val1 = (isascii(i) == 0);
        val2 = (__testisascii(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isascii()\n");

            exit(1);
        }
        val1 = (iscntrl(i) == 0);
        val2 = (__testiscntrl(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "%d - %d\n", iscntrl(i), __testiscntrl(i));
            fprintf(stderr, "ERROR: iscntrl(%s:%d), %x, %x\n",
                locname, i, val1, val2);

            exit(1);
        }
        val1 = (isdigit(i) == 0);
        val2 = (__testisdigit(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: isdigit()\n");

            exit(1);
        }
        val1 = (toascii(i) == 0);
        val2 = (__toascii(i) == 0);
        if (val1 != val2) {
            fprintf(stderr, "ERROR: toascii()\n");

            exit(1);
        }

        if (tolower(i) != __tolower(i)) {
            fprintf(stderr, "ERROR: tolower(%d)\n", i);
            
            exit(1);
        }
        if (toupper(i) != __toupper(i)) {
            fprintf(stderr, "ERROR: toupper(%d)\n", i);
            
            exit(1);
        }
    }

    return;
}

void
xtestucs(int id)
{
    int i, tmp;
    int32_t val32;
    uint16_t val16;

    if (id < 0 || id > 1) {

        return;
    }
    __setctype(id);
    if (id == 0) {
        for (i = -128 ; i < 0 ; i++) {
            val32 = toucs(i);
            val16 = val32;
            if ((val32 != 0xffff) || (val16 != 0xffff)) {
                fprintf(stderr, "ERROR: toucs(%d): %d, %d\n", id, val16, i);
                
                exit(1);
            }
        }
        for (i = 0 ; i < 128 ; i++) {
            val32 = toucs(i);
            val16 = val32;
            if ((val32 != i) || (val16 != i)) {
                fprintf(stderr, "ERROR: toucs(%d): %d, %d\n", id, val16, i);
                
                exit(1);
            }
        }
        for (i = 128 ; i < 256 ; i++) {
            val32 = toucs(i);
            val16 = val32;
            if ((val32 != 0xffff) || (val16 != 0xffff)) {
                fprintf(stderr, "ERROR: toucs(%d): %d, %d\n", id, val16, i);
                
                exit(1);
            }
        }
    } else {
        for (i = -128 ; i < 256 ; i++) {
            tmp = (unsigned char)i;
            val32 = toucs(tmp);
            val16 = val32;
            if ((val32 != tmp) || (val16 != tmp)) {
                fprintf(stderr, "ERROR: toucs(%d): %d, %d\n", id, val16, i);
                
                exit(1);
            }
        }
    }

    return;
}

#if 0
int
foocoll(const void *str1, const void *str2)
{
    return (strcoll(str1, str2));
}

void
xtestcoll(char *locname, int id)
{
    int i;
    int val1, val2;
    uint16_t u16;
    uint16_t ndx;

    if (!setlocale(LC_COLLATE, locname)) {
        fprintf(stderr, "cannot set locale to %s\n", locname);

        return;
    }
    __setctype(id);
    for (u16 = 0 ; u16 < 256 ; u16++) {
        colltbl[u16] = u16;
    }
    qsort(colltbl, 256, sizeof(uint16_t), foocoll);
    for (u16 = 0 ; u16 < 256 ; u16++) {
        ndx = colltbl[u16];
        weighttbl[ndx] = u16;
    }
    for (i = 0 ; i < 256 ; i++) {
        val1 = weighttbl[i];
        val2 = __ctypelocale.weighttbl[i];
        if (val1 != val2) {
            fprintf(stderr, "ERROR: collate %d %x %x\n", i, val1, val2);

            return;
        }
    }

    return;
}
#endif

#if 0
int
foocoll(const void *str1, const void *str2)
{
    uint16_t val1;
    uint16_t val2;

    while ((*str1) && (*str2)) {
        val1 = __iso14651weighttbl[*str1];
        val2 = __iso14651weighttbl[*str2];
        if (val1 < val2) {

            return -1;
        } else if (val2 < val1) {

            return 1;
        }
    }
    if (*str1 == 0) {

        return -1;
    } else if (*str2 == 0) {

        return 1;
    }

    /* NOTREACHED */
    return 0;
}
#endif

#if 0
int
foocoll1(const void *str1, const void *str2)
{
    return (strcoll(str1, str2));
}

int
foocoll2(const void *str1, const void *str2)
{
    int32_t *i32ptr1;
    int32_t *i32ptr2;
    uint16_t val1;
    uint16_t val2;
    uint16_t weight1;
    uint16_t weight2;

    i32ptr1 = str1;
    i32ptr2 = str2;
#if 0
    val1 = toucs(*i32ptr1);
    val2 = toucs(*i32ptr2);
#endif
    val1 = *i32ptr1;
    val2 = *i32ptr2;
    weight1 = __iso14651t1weighttbl[val1];
    weight2 = __iso14651t1weighttbl[val2];
    if (weight1 < weight2) {

        return -1;
    } else if (weight1 > weight2) {

        return 1;
    } else {

        return 0;
    }
}
#endif

#if 0
void
xtestcoll(char *locname, int id)
{
    int i;
    int val1, val2;
    int32_t i32;
    uint16_t ndx;

#if 0
    if (!setlocale(LC_COLLATE, locname)) {
        fprintf(stderr, "cannot set locale to %s\n", locname);

        return;
    }
    for (i32 = 0 ; i32 < 65536 ; i32++) {
        colltbl[i32] = i32;
    }
    qsort(colltbl, 65536, sizeof(int32_t), foocoll);
    for (i32 = 0 ; i32 < 65536 ; i32++) {
        ndx = colltbl[i32];
        weighttbl[ndx] = i32;
    }
    for (i = 0 ; i < 65536 ; i++) {
        val1 = weighttbl[i];
        val2 = __iso14651t1weighttbl[i];
        if (val1 != val2) {
            fprintf(stderr, "ERROR: collate %d %x %x\n", i, val1, val2);

            return;
        }
    }
#endif

#if 0
    if (!setlocale(LC_COLLATE, locname)) {
        fprintf(stderr, "cannot set locale to %s\n", locname);

        return;
    }
    __setctype(id);
    for (i32 = 0 ; i32 < 256 ; i32++) {
        colltbl[i32] = i32;
        weighttbl[i32] = i32;
    }
    qsort(colltbl, 256, sizeof(int32_t), foocoll1);
    qsort(weighttbl, 256, sizeof(int32_t), foocoll2);
    for (i32 = 0 ; i32 < 256 ; i32++) {
        val1 = colltbl[i32];
        val2 = weighttbl[i32];
        if (val1 != val2) {
            fprintf(stderr, "ERROR: collate: %x - %x/%x\n", i32, val1, val2);
        }
    }
#endif

    if (!setlocale(LC_COLLATE, locname) || !setlocale(LC_CTYPE, locname)) {
        fprintf(stderr, "cannot set locale to %s\n", locname);

        return;
    }
//    __setctype(id);
    for (i32 = 0 ; i32 < 65536 ; i32++) {
        colltbl[i32] = i32;
        weighttbl[i32] = i32;
    }
    qsort(colltbl, 65536, sizeof(int32_t), foocoll1);
    qsort(weighttbl, 65536, sizeof(int32_t), foocoll2);
    for (i32 = 0 ; i32 < 65536 ; i32++) {
        val1 = colltbl[i32];
        val2 = weighttbl[i32];
        if (val1 != val2) {
            fprintf(stderr, "ERROR: collate: %x - %x/%x\n", i32, val1, val2);
        }
    }

    return;
}
#endif

void
xprof(const char *locname, int id)
{
    int i, j, k;
    PROFILE_CPU_VARS;

    setlocale(LC_CTYPE, locname);
    __setctype(id);
    PROFILE_CPU_INIT();
    PROFILE_CPU_START();
    k = 0;
    for (i = 0 ; i < 1048576 ; i++) {
        for (j = -128 ; j < 256 ; j++) {
            k += isalnum(j);
            k += isalpha(j);
            k += isblank(j);
            k += iscntrl(j);
            k += isgraph(j);
            k += islower(j);
            k += isprint(j);
            k += ispunct(j);
            k += isspace(j);
            k += isupper(j);
            k += isxdigit(j);
            k += tolower(j);
            k += toupper(j);
        }
    }
    PROFILE_CPU_STOP();
    fprintf(stderr, "GNU: %lu\n", PROFILE_CPU_TIME());

    PROFILE_CPU_INIT();
    PROFILE_CPU_START();
    k = 0;
    for (i = 0 ; i < 1048576 ; i++) {
        for (j = -128 ; j < 256 ; j++) {
            k += __testisalnum(j);
            k += __testisalpha(j);
            k += __testisblank(j);
            k += __testiscntrl(j);
            k += __testisgraph(j);
            k += __testislower(j);
            k += __testisprint(j);
            k += __testispunct(j);
            k += __testisspace(j);
            k += __testisupper(j);
            k += __testisxdigit(j);
            k += __tolower(j);
            k += __toupper(j);
        }
    }
    PROFILE_CPU_STOP();
    fprintf(stderr, "T1: %lu\n", PROFILE_CPU_TIME());

    PROFILE_CPU_INIT();
    PROFILE_CPU_START();
    for (i = 0 ; i < 1048576 ; i++) {
        k = 0;
        for (j = -128 ; j < 256 ; j++) {
            k += toucs(j);
        }
    }
    PROFILE_CPU_STOP();
    fprintf(stderr, "UCS: %lu\n", PROFILE_CPU_TIME());

    return;
}

int
main(int argc, char *argv[])
{
    const char *locname;
    int i;

#if 0
    xtestcoll("en_DK", 1);
    xtestcoll("af_ZA", 1);
    xtestcoll("ar_AE", 1);
#endif
//    xtestcoll("zh_CN", 1);
#if 0
    for (i = 0 ; (__ctypelocs[i].name) ; i++) {
        locname = __ctypelocs[i].name;
        fprintf(stderr, "testing %s\n", locname);
        xtest(locname, __ctypelocs[i].id);
        xtestcoll(locname, __ctypelocs[i].id);
    }
#endif

#if 0
    xprof("C", 0);
    xtestucs(0);
    xtestucs(1);

    for (i = 0 ; i < 17 ; i++) {
        locname = locnames[i];
        if (locname) {
            fprintf(stderr, "testing %s\n", locname);
            xtest(locname, i);
        }
    }
#endif
#if 0    
    xtest("C", 0);
    xtest("fi_FI", 1);
    xtest("bs_BA", 2);
    xtest("ru_RU", 5);
    xtest("ar_SA", 6);
    xtest("el_GR", 7);
    xtest("he_IL", 8);
    xtest("tr_TR", 9);
    xtest("mi_NZ", 13);
    xtest("cy_GB", 14);
    xtest("fi_FI@euro", 15);
#endif

    exit(0);
}

