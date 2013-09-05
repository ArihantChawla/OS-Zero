#include <stdio.h>
#include <stdlib.h>
#include <mjolnir/conf.h>
#if (MJOL_USE_ZERO_RANDMT32)
#include <zero/randmt32.h>
#endif
#include <zero/trix.h>
#include <mjolnir/mjol.h>

uint8_t mjolisobjmap[32];
uint8_t mjolisitemmap[32];

/* initialise bitmap for objects you can move on top of */
void
mjolinitobj(void)
{
    setbit(mjolisobjmap, MJOL_OBJ_FLOOR);
    setbit(mjolisobjmap, MJOL_OBJ_CORRIDOR);
    setbit(mjolisobjmap, MJOL_OBJ_DOOR);
    setbit(mjolisobjmap, MJOL_OBJ_FOOD);
    setbit(mjolisobjmap, MJOL_OBJ_WATER);
    setbit(mjolisobjmap, MJOL_OBJ_GOLD);
    setbit(mjolisobjmap, MJOL_OBJ_SILVER_BULLET);
    setbit(mjolisobjmap, MJOL_OBJ_POTION);
    setbit(mjolisobjmap, MJOL_OBJ_PLANT);
    setbit(mjolisobjmap, MJOL_OBJ_PUNCHCARD);
    setbit(mjolisobjmap, MJOL_OBJ_STAIR_DOWN);
    setbit(mjolisobjmap, MJOL_OBJ_STAIR_UP);
    setbit(mjolisobjmap, MJOL_OBJ_STATUE);
    setbit(mjolisobjmap, MJOL_OBJ_TRAP);
    setbit(mjolisobjmap, MJOL_OBJ_WAND);
    setbit(mjolisobjmap, MJOL_OBJ_SCROLL);
    setbit(mjolisobjmap, MJOL_OBJ_RING);
    setbit(mjolisobjmap, MJOL_OBJ_CHAIN);
    setbit(mjolisobjmap, MJOL_OBJ_CHEST);
    setbit(mjolisobjmap, MJOL_OBJ_SUBMACHINE_GUN);
    setbit(mjolisobjmap, MJOL_OBJ_HONEY);
    setbit(mjolisobjmap, MJOL_OBJ_KNIFE);
    setbit(mjolisobjmap, MJOL_OBJ_LOCKPICK);
    setbit(mjolisobjmap, MJOL_OBJ_LASER);
    setbit(mjolisobjmap, MJOL_OBJ_MACE);
    setbit(mjolisobjmap, MJOL_OBJ_MAINFRAME);
    setbit(mjolisobjmap, MJOL_OBJ_PIPE);
    setbit(mjolisobjmap, MJOL_OBJ_PISTOL);
    setbit(mjolisobjmap, MJOL_OBJ_SWORD);
    setbit(mjolisobjmap, MJOL_OBJ_WELL);
    setbit(mjolisobjmap, MJOL_OBJ_CROSS);
    setbit(mjolisobjmap, MJOL_OBJ_ALTAR);

    return;
}

/* initialise bitmap for objects you can pick up */
void
mjolinititem(void)
{
    setbit(mjolisitemmap, MJOL_OBJ_FOOD);
    setbit(mjolisitemmap, MJOL_OBJ_WATER);
    setbit(mjolisitemmap, MJOL_OBJ_GOLD);
    setbit(mjolisitemmap, MJOL_OBJ_SILVER_BULLET);
    setbit(mjolisitemmap, MJOL_OBJ_POTION);
    setbit(mjolisitemmap, MJOL_OBJ_PLANT);
    setbit(mjolisitemmap, MJOL_OBJ_PUNCHCARD);
    setbit(mjolisitemmap, MJOL_OBJ_STATUE);
    setbit(mjolisitemmap, MJOL_OBJ_WAND);
    setbit(mjolisitemmap, MJOL_OBJ_SCROLL);
    setbit(mjolisitemmap, MJOL_OBJ_RING);
    setbit(mjolisitemmap, MJOL_OBJ_CHAIN);
    setbit(mjolisitemmap, MJOL_OBJ_CHEST);
    setbit(mjolisitemmap, MJOL_OBJ_SUBMACHINE_GUN);
    setbit(mjolisitemmap, MJOL_OBJ_HONEY);
    setbit(mjolisitemmap, MJOL_OBJ_KNIFE);
    setbit(mjolisitemmap, MJOL_OBJ_LOCKPICK);
    setbit(mjolisitemmap, MJOL_OBJ_LASER);
    setbit(mjolisitemmap, MJOL_OBJ_MACE);
    setbit(mjolisitemmap, MJOL_OBJ_MAINFRAME);
    setbit(mjolisitemmap, MJOL_OBJ_PIPE);
    setbit(mjolisitemmap, MJOL_OBJ_PISTOL);
    setbit(mjolisitemmap, MJOL_OBJ_SWORD);
    setbit(mjolisitemmap, MJOL_OBJ_CROSS);

    return;
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

