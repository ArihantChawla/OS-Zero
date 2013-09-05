#include <stdio.h>
#include <stdlib.h>
#include <mjolnir/conf.h>
#if (MJOL_USE_ZERO_RANDMT32)
#include <zero/randmt32.h>
#endif
#include <zero/trix.h>
#include <mjolnir/mjol.h>

uint8_t mjolisobjtab[32];
uint8_t mjolisitemtab[32];

/* initialise bitmap for objects you can move on top of */
void
mjolinitobj(void)
{
    setbit(mjolisobjtab, MJOL_OBJ_FLOOR);
    setbit(mjolisobjtab, MJOL_OBJ_CORRIDOR);
    setbit(mjolisobjtab, MJOL_OBJ_DOOR);
    setbit(mjolisobjtab, MJOL_OBJ_FOOD);
    setbit(mjolisobjtab, MJOL_OBJ_WATER);
    setbit(mjolisobjtab, MJOL_OBJ_GOLD);
    setbit(mjolisobjtab, MJOL_OBJ_SILVER_BULLET);
    setbit(mjolisobjtab, MJOL_OBJ_POTION);
    setbit(mjolisobjtab, MJOL_OBJ_PLANT);
    setbit(mjolisobjtab, MJOL_OBJ_PUNCHCARD);
    setbit(mjolisobjtab, MJOL_OBJ_DOWN);
    setbit(mjolisobjtab, MJOL_OBJ_UP);
    setbit(mjolisobjtab, MJOL_OBJ_STATUE);
    setbit(mjolisobjtab, MJOL_OBJ_TRAP);
    setbit(mjolisobjtab, MJOL_OBJ_WAND);
    setbit(mjolisobjtab, MJOL_OBJ_SCROLL);
    setbit(mjolisobjtab, MJOL_OBJ_RING);
    setbit(mjolisobjtab, MJOL_OBJ_CHAIN);
    setbit(mjolisobjtab, MJOL_OBJ_CHEST);
    setbit(mjolisobjtab, MJOL_OBJ_SUBMACHINE_GUN);
    setbit(mjolisobjtab, MJOL_OBJ_HONEY);
    setbit(mjolisobjtab, MJOL_OBJ_KNIFE);
    setbit(mjolisobjtab, MJOL_OBJ_LOCKPICK);
    setbit(mjolisobjtab, MJOL_OBJ_LASER);
    setbit(mjolisobjtab, MJOL_OBJ_MACE);
    setbit(mjolisobjtab, MJOL_OBJ_MAINFRAME);
    setbit(mjolisobjtab, MJOL_OBJ_PIPE);
    setbit(mjolisobjtab, MJOL_OBJ_PISTOL);
    setbit(mjolisobjtab, MJOL_OBJ_SWORD);
    setbit(mjolisobjtab, MJOL_OBJ_WELL);
    setbit(mjolisobjtab, MJOL_OBJ_CROSS);
    setbit(mjolisobjtab, MJOL_OBJ_ALTAR);

    return;
}

/* initialise bitmap for objects you can pick up */
void
mjolinititem(void)
{
    setbit(mjolisitemtab, MJOL_OBJ_FOOD);
    setbit(mjolisitemtab, MJOL_OBJ_WATER);
    setbit(mjolisitemtab, MJOL_OBJ_GOLD);
    setbit(mjolisitemtab, MJOL_OBJ_SILVER_BULLET);
    setbit(mjolisitemtab, MJOL_OBJ_POTION);
    setbit(mjolisitemtab, MJOL_OBJ_PLANT);
    setbit(mjolisitemtab, MJOL_OBJ_PUNCHCARD);
    setbit(mjolisitemtab, MJOL_OBJ_STATUE);
    setbit(mjolisitemtab, MJOL_OBJ_WAND);
    setbit(mjolisitemtab, MJOL_OBJ_SCROLL);
    setbit(mjolisitemtab, MJOL_OBJ_RING);
    setbit(mjolisitemtab, MJOL_OBJ_CHAIN);
    setbit(mjolisitemtab, MJOL_OBJ_CHEST);
    setbit(mjolisitemtab, MJOL_OBJ_SUBMACHINE_GUN);
    setbit(mjolisitemtab, MJOL_OBJ_HONEY);
    setbit(mjolisitemtab, MJOL_OBJ_KNIFE);
    setbit(mjolisitemtab, MJOL_OBJ_LOCKPICK);
    setbit(mjolisitemtab, MJOL_OBJ_LASER);
    setbit(mjolisitemtab, MJOL_OBJ_MACE);
    setbit(mjolisitemtab, MJOL_OBJ_MAINFRAME);
    setbit(mjolisitemtab, MJOL_OBJ_PIPE);
    setbit(mjolisitemtab, MJOL_OBJ_PISTOL);
    setbit(mjolisitemtab, MJOL_OBJ_SWORD);
    setbit(mjolisitemtab, MJOL_OBJ_CROSS);

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

