#include <stdio.h>
#include <stdlib.h>
#include <zero/randmt32.h>
#include <zero/trix.h>
#include <mjolnir/mjol.h>

struct mjolobj *
mjolmkcorridor(void)
{
    struct mjolobj *obj = calloc(1, sizeof(struct mjolobj));
    long            l = randmt32() & 0x0f;

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
    long            l = randmt32() & 0x0f;

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

