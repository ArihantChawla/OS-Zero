#include <stdio.h>
#include <stdlib.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <mjolnir/mjol.h>
#include <mjolnir/scr.h>

static uint8_t   chdirbitmap[32] ALIGNED(CLSIZE);
static uint8_t   chargbitmap[32];
struct mjolchar *mjolchaseq;

struct mjolchar *
mjolmkplayer(struct mjolgame *game)
{
    struct mjolchar *data = calloc(1, sizeof(struct mjolchar));

    if (!data) {
        fprintf(stderr, "memory allocation failure\n");

        exit(1);
    }
    data->data.type = MJOL_CHAR_PLAYER;

    return data;
}

void
mjoldoturn(struct mjolgame *game, struct mjolchar *data)
{
    long   n = mjolcharnturn(data);
    int  (*printmsg)(const char *, ...) = game->scr->printmsg;
    int  (*getkbd)(void) = game->scr->getch;
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

