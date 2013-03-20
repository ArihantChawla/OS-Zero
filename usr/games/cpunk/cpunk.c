/* Copyright (C) Tuomo Petteri Venäläinen 2011 */

/* plans for zero cpunk RPG */

#include <stdio.h>
#include <stdlib.h>

#include <cpunk.h>

#define CPUNK_BLACK_TEXT   "\033[30;m"
#define CPUNK_RED_TEXT     "\033[31;m"
#define CPUNK_GREEN_TEXT   "\033[32;m"
#define CPUNK_YELLOW_TEXT  "\033[33;m"
#define CPUNK_BLUE_TEXT    "\033[34;m"
#define CPUNK_MAGENTA_TEXT "\033[35;m"
#define CPUNK_CYAN_TEXT    "\033[36;m"
#define CPUNK_WHITE_TEXT   "\033[37;m"

const char *textcolortab[256];

void
cpunkinitcolors(void)
{
    textcolortab[CPUNK_GROUND] = CPUNK_BLACK_TEXT;
    textcolortab[CPUNK_CODE] = CPUNK_CYAN_TEXT;
    textcolortab[CPUNK_ALTAR] = CPUNK_GREEN_TEXT;
    textcolortab[CPUNK_SCROLL] = CPUNK_YELLOW_TEXT;
    textcolortab[CPUNK_DOOR] = CPUNK_GREEN_TEXT;
    textcolortab[CPUNK_WALL] = CPUNK_RED_TEXT;
    textcolortab[CPUNK_LOCKPICK] = CPUNK_YELLOW_TEXT;
    textcolortab[CPUNK_SWORD] = CPUNK_YELLOW_TEXT;
    textcolortab[CPUNK_GOLD] = CPUNK_YELLOW_TEXT;
    textcolortab[CPUNK_UNKNOWN] = CPUNK_RED_TEXT;
    textcolortab[CPUNK_TELEPORT] = CPUNK_GREEN_TEXT;
    textcolortab[CPUNK_CALCULATOR] = CPUNK_CYAN_TEXT;
    textcolortab[CPUNK_ALIEN] = CPUNK_MAGENTA_TEXT;
    textcolortab[CPUNK_BOOK] = CPUNK_YELLOW_TEXT;
    textcolortab[CPUNK_CYBORG] = CPUNK_MAGENTA_TEXT;
    textcolortab[CPUNK_DESKTOP] = CPUNK_CYAN_TEXT;
    textcolortab[CPUNK_MAINFRAME] = CPUNK_CYAN_TEXT;
    textcolortab[CPUNK_HUMAN] = CPUNK_MAGENTA_TEXT;
    textcolortab[CPUNK_KEY] = CPUNK_YELLOW_TEXT;
    textcolortab[CPUNK_LAPTOP] = CPUNK_CYAN_TEXT;
    textcolortab[CPUNK_MOBILE] = CPUNK_CYAN_TEXT;
    textcolortab[CPUNK_ORACLE] = CPUNK_YELLOW_TEXT;
    textcolortab[CPUNK_PLAYER] = CPUNK_GREEN_TEXT;
    textcolortab[CPUNK_ROBOT] = CPUNK_MAGENTA_TEXT;
    textcolortab[CPUNK_TERMINAL] = CPUNK_CYAN_TEXT;
    textcolortab[CPUNK_SUPER] = CPUNK_CYAN_TEXT;
    textcolortab[CPUNK_VIXEN] = CPUNK_MAGENTA_TEXT;
    textcolortab[CPUNK_WIZARD] = CPUNK_MAGENTA_TEXT;
    textcolortab[CPUNK_CAT] = CPUNK_MAGENTA_TEXT;
    textcolortab[CPUNK_DOG] = CPUNK_MAGENTA_TEXT;
    textcolortab[CPUNK_HERB] = CPUNK_GREEN_TEXT;
#if (CPUNK_ADULT)
    textcolortab[CPUNK_HOOKER] = CPUNK_MAGENTA_TEXT;
    textcolortab[CPUNK_PIMP] = CPUNK_MAGENTA_TEXT;
#endif
    textcolortab[CPUNK_LIZARD] = CPUNK_MAGENTA_TEXT;
    textcolortab[CPUNK_HOLE] = CPUNK_GREEN_TEXT;
    textcolortab[CPUNK_RAT] = CPUNK_MAGENTA_TEXT;
    textcolortab[CPUNK_SPRING] = CPUNK_BLUE_TEXT;
    textcolortab[CPUNK_WELL] = CPUNK_BLUE_TEXT;
    textcolortab[CPUNK_FOOD] = CPUNK_GREEN_TEXT;
}


int
main(int argc, char *argv[])
{
    struct cpunkcharacter *player;
    struct cpunklevel     *level;

    cpunkinitcolors();
    player = cpunknewchar(argv[1], CPUNK_RACE_WIZARD);
    level = cpunknewlevel();
    {
        long x;
        long y;
        unsigned char uc;

        for (y = 0 ; y < CPUNK_LEVEL_HEIGHT ; y++) {
            for (x = 0 ; x < CPUNK_LEVEL_WIDTH ; x++) {
                uc = level->items[y][x];
                if (uc) {
#if 0
                    fprintf(stderr, "%c", uc);
#endif
                    fprintf(stderr, "%s%c%s",
                            textcolortab[uc], uc, "\033[37;m");
                }
            }
            fprintf(stderr, "\n");
        }
    }
    cpunkprintchar(player);

    exit(0);
}

