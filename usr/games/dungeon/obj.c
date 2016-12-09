#include <string.h>
#include <dungeon/conf.h>
#include <dungeon/cell.h>
#include <mjolnir/obj.h>
#include <mjolnir/chr.h>

#if (DNG_MJOLNIR)

static mjolmkfunc *dnginitobjtab[256];

void
dnginitobjmjol(void)
{
/* objects */
    dnginitobjtab[MJOL_OBJ_FLOOR] = mjolmkfloor;
    dnginitobjtab[MJOL_OBJ_CORRIDOR] = mjolmkcorridor;
    dnginitobjtab[MJOL_OBJ_HORIZONTAL_WALL] = mjolmkhorizwall;
    dnginitobjtab[MJOL_OBJ_VERTICAL_WALL] = mjolmkvertwall;
    dnginitobjtab[MJOL_OBJ_DOOR] = mjolmkdoor;
    dnginitobjtab[MJOL_OBJ_FOOD] = NULL;
    dnginitobjtab[MJOL_OBJ_WATER] = NULL;
    dnginitobjtab[MJOL_OBJ_FOUNTAIN] = NULL;
    dnginitobjtab[MJOL_OBJ_GOLD] = NULL;
    dnginitobjtab[MJOL_OBJ_POTION] = NULL;
    dnginitobjtab[MJOL_OBJ_PLANT] = NULL;
    dnginitobjtab[MJOL_OBJ_PUNCHCARD] = NULL;
    dnginitobjtab[MJOL_OBJ_TAPE] = NULL;
    dnginitobjtab[MJOL_OBJ_STAIR_DOWN] = NULL;
    dnginitobjtab[MJOL_OBJ_STAIR_UP] = NULL;
    dnginitobjtab[MJOL_OBJ_STATUE] = NULL;
    dnginitobjtab[MJOL_OBJ_TRAP] = NULL;
    dnginitobjtab[MJOL_OBJ_WAND] = NULL;
    dnginitobjtab[MJOL_OBJ_SCROLL] = NULL;
    dnginitobjtab[MJOL_OBJ_RING] = NULL;
    dnginitobjtab[MJOL_OBJ_WHIP] = NULL;
    dnginitobjtab[MJOL_OBJ_ARMOR] = NULL;
    dnginitobjtab[MJOL_OBJ_CHAIN] = NULL;
    dnginitobjtab[MJOL_OBJ_CHEST] = NULL;
    dnginitobjtab[MJOL_OBJ_SUBMACHINE_GUN] = NULL;
    dnginitobjtab[MJOL_OBJ_HONEY] = NULL;
    dnginitobjtab[MJOL_OBJ_KNIFE] = NULL;
    dnginitobjtab[MJOL_OBJ_LOCKPICK] = NULL;
    dnginitobjtab[MJOL_OBJ_LASER] = NULL;
    dnginitobjtab[MJOL_OBJ_MACE] = NULL;
    dnginitobjtab[MJOL_OBJ_MAINFRAME] = NULL;
    dnginitobjtab[MJOL_OBJ_PIPE] = NULL;
    dnginitobjtab[MJOL_OBJ_PISTOL] = NULL;
    dnginitobjtab[MJOL_OBJ_SWORD] = NULL;
    dnginitobjtab[MJOL_OBJ_TELEPORT] = NULL;
    dnginitobjtab[MJOL_OBJ_WELL] = NULL;
    dnginitobjtab[MJOL_OBJ_CROSS] = NULL;
    dnginitobjtab[MJOL_OBJ_ALTAR] = NULL;
/* characters */
    dnginitobjtab[MJOL_CHAR_PLAYER] = NULL;
    dnginitobjtab[MJOL_CHAR_ANT] = NULL;
    dnginitobjtab[MJOL_CHAR_ALIEN] = NULL;
    dnginitobjtab[MJOL_CHAR_BEE] = NULL;
    dnginitobjtab[MJOL_CHAR_BEE_QUEEN] = NULL;
    dnginitobjtab[MJOL_CHAR_DOG] = NULL;
    dnginitobjtab[MJOL_CHAR_DEITY] = NULL;
    dnginitobjtab[MJOL_CHAR_GHOUL] = NULL;
    dnginitobjtab[MJOL_CHAR_HUMAN] = NULL;
    dnginitobjtab[MJOL_CHAR_ORACLE] = NULL;
    dnginitobjtab[MJOL_CHAR_REMNANTS] = NULL;
    dnginitobjtab[MJOL_CHAR_UNICORN] = NULL;
    dnginitobjtab[MJOL_CHAR_THOR] = NULL;
    dnginitobjtab[MJOL_CHAR_VAMPIRE] = NULL;
    dnginitobjtab[MJOL_CHAR_DRACULA] = NULL;
    dnginitobjtab[MJOL_CHAR_WOLF] = NULL;
    dnginitobjtab[MJOL_CHAR_ZOMBIE] = NULL;
    /* special items */
    dnginitobjtab[MJOL_ITEM_DEMON_WHIP] = NULL;
    dnginitobjtab[MJOL_ITEM_JATIMATIC] = NULL;
    dnginitobjtab[MJOL_ITEM_MJOLNIR] = NULL;
    dnginitobjtab[MJOL_ITEM_PLEIGNIR] = NULL;
    dnginitobjtab[MJOL_ITEM_STORMBRINGER] = NULL;
    dnginitobjtab[MJOL_ITEM_EXCALIBUR] = NULL;
    dnginitobjtab[MJOL_ITEM_HOLY_CROSS] = NULL;
    dnginitobjtab[MJOL_ITEM_HOLY_WATER] = NULL;
    dnginitobjtab[MJOL_ITEM_SILVER_BULLET ] = NULL;

    return;
}

struct mjolobj *
dngmkobjmjol(struct celldng *dng, long type, long flg, struct dngobj *data)
{
    mjolmkfunc     *mkfunc = dnginitobjtab[type];
    struct mjolobj *obj = (mkfunc) ? mkfunc(flg, data) : NULL;

    if (obj) {
        obj->data.flg = flg;
        memcpy(&obj->data, data, sizeof(struct dngobj));
    }
        
    return obj;
}

#endif /* DNG_MJOLNIR */

