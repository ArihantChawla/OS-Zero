#include <zero/trix.h>
#include <mjolnir/mjol.h>

static uint8_t chdirbitmap[32];
static uint8_t chargbitmap[32];

void
mjolchardoturn(struct mjolchardata *chardata)
{
    long n = mjolcharnturn(chardata);
    int  ch1;
    int  ch2;

    if (n) {
        mjolprintmsg("You have %ld turns\n", n);
        while (n--) {
            ch1 = mjolgetch();
            if (bitset(chdirbitmap, ch1)) {
                ch2 = mjolgetch();
            }
            chardata->nturn++;
        }
    }
}

