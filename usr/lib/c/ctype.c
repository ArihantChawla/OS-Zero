#include <stdint.h>
#include <ctype.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <bits/ctype.h>

uint8_t *ctypeparamtab[NCTYPE]
= {
    ctypeparamtab_c,
    ctypeparamtab_iso8859_1,
    ctypeparamtab_iso8859_2,
    ctypeparamtab_iso8859_3,
    ctypeparamtab_iso8859_4,
    ctypeparamtab_iso8859_5,
    ctypeparamtab_iso8859_6,
    ctypeparamtab_iso8859_7,
    ctypeparamtab_iso8859_8,
    ctypeparamtab_iso8859_9,
    ctypeparamtab_iso8859_10,
    ctypeparamtab_iso8859_11,
    ctypeparamtab_iso8859_12,
    ctypeparamtab_iso8859_13,
    ctypeparamtab_iso8859_14,
    ctypeparamtab_iso8859_15,
    ctypeparamtab_iso8859_16,
};

volatile uint8_t *ctypeparamptr = ctypeparamtab_c;

int
setctype(int ctype)
{
    if (ctype < 0 || ctype >= NCTYPE) {

        return -1;
    }
    ctypeparamptr = ctypeparamtab[ctype];

    return 0;
}

