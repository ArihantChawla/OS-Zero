#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <mjolnir/conf.h>
#if (MJOL_USE_ZERO_RANDMT32)
#include <zero/randmt32.h>
#endif
#include <zero/trix.h>
#include <mjolnir/mjol.h>

unsigned char mjolcanmovetomap[256 / CHAR_BIT];
unsigned char mjolcanpickupmap[256 / CHAR_BIT];
unsigned char mjolcanwearmap[256 / CHAR_BIT];
unsigned char mjolcanwieldmap[256 / CHAR_BIT];

/* initialise bitmap for objects you can move on top of */
void
mjolinitcanmoveto(void)
{
    setbit(mjolcanmovetomap, MJOL_OBJ_FLOOR);
    setbit(mjolcanmovetomap, MJOL_OBJ_CORRIDOR);
    setbit(mjolcanmovetomap, MJOL_OBJ_DOOR);
    setbit(mjolcanmovetomap, MJOL_OBJ_FOOD);
    setbit(mjolcanmovetomap, MJOL_OBJ_WATER);
    setbit(mjolcanmovetomap, MJOL_OBJ_FOUNTAIN);
    setbit(mjolcanmovetomap, MJOL_OBJ_GOLD);
    setbit(mjolcanmovetomap, MJOL_OBJ_SILVER_BULLET);
    setbit(mjolcanmovetomap, MJOL_OBJ_POTION);
    setbit(mjolcanmovetomap, MJOL_OBJ_PLANT);
    setbit(mjolcanmovetomap, MJOL_OBJ_PUNCHCARD);
    setbit(mjolcanmovetomap, MJOL_OBJ_STAIR_DOWN);
    setbit(mjolcanmovetomap, MJOL_OBJ_STAIR_UP);
    setbit(mjolcanmovetomap, MJOL_OBJ_STATUE);
    setbit(mjolcanmovetomap, MJOL_OBJ_TRAP);
    setbit(mjolcanmovetomap, MJOL_OBJ_WAND);
    setbit(mjolcanmovetomap, MJOL_OBJ_SCROLL);
    setbit(mjolcanmovetomap, MJOL_OBJ_RING);
    setbit(mjolcanmovetomap, MJOL_OBJ_ARMOR);
    setbit(mjolcanmovetomap, MJOL_OBJ_CHAIN);
    setbit(mjolcanmovetomap, MJOL_OBJ_CHEST);
    setbit(mjolcanmovetomap, MJOL_OBJ_SUBMACHINE_GUN);
    setbit(mjolcanmovetomap, MJOL_OBJ_HONEY);
    setbit(mjolcanmovetomap, MJOL_OBJ_KNIFE);
    setbit(mjolcanmovetomap, MJOL_OBJ_LOCKPICK);
    setbit(mjolcanmovetomap, MJOL_OBJ_LASER);
    setbit(mjolcanmovetomap, MJOL_OBJ_MACE);
    setbit(mjolcanmovetomap, MJOL_OBJ_MAINFRAME);
    setbit(mjolcanmovetomap, MJOL_OBJ_PIPE);
    setbit(mjolcanmovetomap, MJOL_OBJ_PISTOL);
    setbit(mjolcanmovetomap, MJOL_OBJ_SWORD);
    setbit(mjolcanmovetomap, MJOL_OBJ_WELL);
    setbit(mjolcanmovetomap, MJOL_OBJ_CROSS);
    setbit(mjolcanmovetomap, MJOL_OBJ_ALTAR);

    return;
}

/* initialise bitmap for objects you can pick up */
void
mjolinitcanpickup(void)
{
    setbit(mjolcanpickupmap, MJOL_OBJ_FOOD);
    setbit(mjolcanpickupmap, MJOL_OBJ_WATER);
    setbit(mjolcanpickupmap, MJOL_OBJ_GOLD);
    setbit(mjolcanpickupmap, MJOL_OBJ_SILVER_BULLET);
    setbit(mjolcanpickupmap, MJOL_OBJ_POTION);
    setbit(mjolcanpickupmap, MJOL_OBJ_PLANT);
    setbit(mjolcanpickupmap, MJOL_OBJ_PUNCHCARD);
    setbit(mjolcanpickupmap, MJOL_OBJ_STATUE);
    setbit(mjolcanpickupmap, MJOL_OBJ_WAND);
    setbit(mjolcanpickupmap, MJOL_OBJ_SCROLL);
    setbit(mjolcanpickupmap, MJOL_OBJ_RING);
    setbit(mjolcanpickupmap, MJOL_OBJ_ARMOR);
    setbit(mjolcanpickupmap, MJOL_OBJ_CHAIN);
    setbit(mjolcanpickupmap, MJOL_OBJ_CHEST);
    setbit(mjolcanpickupmap, MJOL_OBJ_SUBMACHINE_GUN);
    setbit(mjolcanpickupmap, MJOL_OBJ_HONEY);
    setbit(mjolcanpickupmap, MJOL_OBJ_KNIFE);
    setbit(mjolcanpickupmap, MJOL_OBJ_LOCKPICK);
    setbit(mjolcanpickupmap, MJOL_OBJ_LASER);
    setbit(mjolcanpickupmap, MJOL_OBJ_MACE);
    setbit(mjolcanpickupmap, MJOL_OBJ_MAINFRAME);
    setbit(mjolcanpickupmap, MJOL_OBJ_PIPE);
    setbit(mjolcanpickupmap, MJOL_OBJ_PISTOL);
    setbit(mjolcanpickupmap, MJOL_OBJ_SWORD);
    setbit(mjolcanpickupmap, MJOL_OBJ_CROSS);

    return;
}

/* initialise bitmap for objects you can wear */
void
mjolinitcanwear(void)
{
    setbit(mjolcanwearmap, MJOL_OBJ_ARMOR);

    return;
}

/* initialise bitmap for weapons you can wield */
void
mjolinitcanwield(void)
{
    setbit(mjolcanwieldmap, MJOL_OBJ_CHAIN);
    setbit(mjolcanwieldmap, MJOL_OBJ_SUBMACHINE_GUN);
    setbit(mjolcanwieldmap, MJOL_OBJ_KNIFE);
    setbit(mjolcanwieldmap, MJOL_OBJ_LASER);
    setbit(mjolcanwieldmap, MJOL_OBJ_MACE);
    setbit(mjolcanwieldmap, MJOL_OBJ_PIPE);
    setbit(mjolcanwieldmap, MJOL_OBJ_PISTOL);
    setbit(mjolcanwieldmap, MJOL_OBJ_SWORD);
    setbit(mjolcanwieldmap, MJOL_OBJ_CROSS);

    return;
}

void
mjolinitobj(void)
{
    mjolinitcanmoveto();
    mjolinitcanpickup();
    mjolinitcanwear();
    mjolinitcanwield();
}

struct mjolobj *
mjolmkcorridor(void)
{
    struct mjolobj *obj = calloc(1, sizeof(struct mjolobj));
    long            l = mjolrand() & 0x0f;

    if (!obj) {
        fprintf(stderr, "memory allocation failure\n");

        exit(1);
    }
    obj->data.type = MJOL_OBJ_CORRIDOR;
    if (!l) {
        /* 1/16 chance of a hidden door */
        obj->data.flg |= MJOL_OBJ_HIDDEN;
    }

    return obj;
}

struct mjolobj *
mjolmkdoor(void)
{
    struct mjolobj *obj = calloc(1, sizeof(struct mjolobj));
    long            l = mjolrand() & 0x0f;

    if (!obj) {
        fprintf(stderr, "memory allocation failure\n");

        exit(1);
    }
    obj->data.type = MJOL_OBJ_DOOR;
    if (!l) {
        /* 1/16 chance of a hidden door */
        obj->data.flg |= MJOL_OBJ_HIDDEN;
    }

    return obj;
}

struct mjolobj *
mjolmkstair(long type)
{
    struct mjolobj *obj = calloc(1, sizeof(struct mjolobj));
    long            l = mjolrand() & 0x0f;

    if (!obj) {
        fprintf(stderr, "memory allocation failure\n");

        exit(1);
    }
    obj->data.type = type;
    if (!l) {
        /* 1/16 chance of a hidden stairway */
        obj->data.flg |= MJOL_OBJ_HIDDEN;
    }

    return obj;
}

long
mjolhit(struct mjolchar *src, struct mjolchar *dest)
{
    long   retval = 0;
    long (*func)(void *, void *);

    func = src->func.hit;
    if (func) {
        retval = func(src, dest);
    }

    return retval;
}

long
mjoltrap(struct mjolobj *trap, struct mjolchar *dest)
{
    long   retval = 0;
    long (*func)(void *, void *);

    func = trap->func.hit;
    if (func) {
        retval = func(trap, dest);
    }

    return retval;
}

