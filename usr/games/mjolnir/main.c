#include <stdlib.h>
#include <dungeon/dng.h>
#include <mjolnir/mjol.h>

extern void mjolinitgame(struct dnggame *game);
//void mjolgameintro(void);
extern void mjolgameloop(struct dnggame *dng);

volatile long mjolquit;

int
main(int argc, char *argv[])
{
    struct dnggame game = { 0 };

    game.argc = argc;
    game.argv = argv;
    mjolinitgame(&game);
    while (!mjolquit) {
        mjolgameloop(&game);
    }

    exit(0);
}

