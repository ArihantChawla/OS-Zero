#ifndef __MJOLNIR_OBJ_H__
#define __MJOLNIR_OBJ_H__

#include <dungeon/conf.h>
#include <dungeon/dng.h>

/* objects */
#define MJOL_OBJ_FLOOR            '.'
#define MJOL_OBJ_CORRIDOR         '#'
#define MJOL_OBJ_HORIZONTAL_WALL  '-'
#define MJOL_OBJ_VERTICAL_WALL    '|'
#define MJOL_OBJ_DOOR             '+'
#define MJOL_OBJ_FOOD             '%'
#define MJOL_OBJ_WATER            '~'
#define MJOL_OBJ_FOUNTAIN         '{'
#define MJOL_OBJ_GOLD             '$'
#define MJOL_OBJ_BULLET           'o'
#define MJOL_OBJ_POTION           '!'
#define MJOL_OBJ_PLANT            '*'
#define MJOL_OBJ_PUNCHCARD        '='
#define MJOL_OBJ_TAPE             '8'
#define MJOL_OBJ_STAIR_DOWN       '<'
#define MJOL_OBJ_STAIR_UP         '>'
#define MJOL_OBJ_STATUE           '&'
#define MJOL_OBJ_TRAP             '^'
#define MJOL_OBJ_WAND             '\\'
#define MJOL_OBJ_SCROLL           '?'
#define MJOL_OBJ_RING             '='
#define MJOL_OBJ_WHIP             '/'
#define MJOL_OBJ_ARMOR            ']'
#define MJOL_OBJ_CHAIN            'c'
#define MJOL_OBJ_CHEST            'C'
#define MJOL_OBJ_SUBMACHINE_GUN   'g'
#define MJOL_OBJ_HONEY            'h'
#define MJOL_OBJ_JATIMATIC        'J'
#define MJOL_OBJ_KNIFE            'k'
#define MJOL_OBJ_LOCKPICK         'l'
#define MJOL_OBJ_LASER            'L'
#define MJOL_OBJ_MACE             'm'
#define MJOL_OBJ_MAINFRAME        'M'
#define MJOL_OBJ_PIPE             'p'
#define MJOL_OBJ_PISTOL           'P'
#define MJOL_OBJ_LONGBOW         ')'
#define MJOL_OBJ_SWORD            's'
#define MJOL_OBJ_TELEPORT         't'
#define MJOL_OBJ_WELL             'w'
#define MJOL_OBJ_CROSS            'x'
#define MJOL_OBJ_ALTAR            'X'

/* special items */
#define MJOL_ITEM_DEMON_WHIP      1
#define MJOL_ITEM_JATIMATIC       2
#define MJOL_ITEM_MJOLNIR         3
#define MJOL_ITEM_PLEIGNIR        4
#define MJOL_ITEM_STORMBRINGER    5
#define MJOL_ITEM_EXCALIBUR       6
#define MJOL_ITEM_HOLY_CROSS      7
#define MJOL_ITEM_HOLY_WATER      8
#define MJOL_ITEM_SILVER_BULLET   9

struct mjolobjfunc {
    char  *str;
    long (*hit)(void *, void *);
    long (*def)(void *, void *);
    long (*pick)(void *, void *);
};

/* data.flg values */
#define MJOL_OBJ_HIDDEN  0x00000001
/* bless values */
#define MJOL_OBJ_BLESSED 1
#define MJOL_OBJ_NEUTRAL 0
#define MJOL_OBJ_CURSED  (-1)
struct mjolobj {
#if (MJOL_VT) || (MJOL_CURSES)
//    int                 id;
    long                id;
    long                special;        // non-0 for special items
#endif
    long                flg;
    struct dngobj       data;           // common object data
    struct mjolobjfunc  func;
    long                weight;         // weight of object
    long                bless;          // BLESSED, NEUTRAL, CURSED
    long                parm;           // e.g. +1 or -1 for armor
    struct mjolobj     *prev;
    struct mjolobj     *next;
};

#if 0
/* event handler function prototype */
typedef void mjolfunc_t(struct dnggame *game,
                        struct dngobj *src, struct dngobj *dest);
#endif

typedef struct mjolobj * mjolmkfunc(void);

extern struct mjolobj * mjolmkfloor(void);
extern struct mjolobj * mjolmkcorridor(void);
extern struct mjolobj * mjolmkdoor(void);
extern struct mjolobj * mjolmkhorizwall(void);
extern struct mjolobj * mjolmkvertwall(void);
extern struct mjolobj * mjolmkstair(void);

#endif /* __MJOLNIR_OBJ_H__ */

