#include <stdio.h>
#include <zero/trix.h>
#include <mjolnir/mjol.h>
#include <mjolnir/scr.h>

extern struct mjolgamescr mjolgamescr;

static uint8_t chdirbitmap[32];
static uint8_t chargbitmap[32];

void
mjolchardoturn(struct mjolgame *game, struct mjolchar *data)
{
    long   n = mjolcharnturn(data);
    int  (*printmsg)(const char *, ...) = mjolgamescr.printmsg;
    int  (*getkbd)(void) = mjolgamescr.getch;
    int    cmd;
    int    dir;
    int    obj;

    if (n) {
        printmsg("You have %ld turns", n);
        while (n--) {
            cmd = getkbd();
//            clrmsg();
            if (bitset(chdirbitmap, cmd)) {
                printmsg("Which direction?");
                dir = getkbd();
            }
            if (bitset(chargbitmap, cmd)) {
                obj = getkbd();
            }
            data->nturn++;
        }
    }
}

