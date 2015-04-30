#ifndef __MJOLNIR_MJOL_H__
#define __MJOLNIR_MJOL_H__

#if (TEST)
#include <stdio.h>
#endif
#include <stddef.h>
#include <stdlib.h>
#include <zero/trix.h>
#include <dungeon/dng.h>
#include <mjolnir/conf.h>
#if (MJOL_USE_ZERO_RANDMT32)
#include <zero/randmt32.h>
#endif
#if (MJOL_CURSES)
#include <curses.h>
#endif
//#include <mjolnir/obj.h>

extern struct mjolgame *mjolgame;
extern struct mjolchr  *chaseq;

#define mjolismove(ch)  bitset(mjolcmdismovemap, ch)
#define mjolsetmove(ch) setbit(mjolcmdismovemap, ch)
#define mjolhasdir(ch)  bitset(mjolcmdhasdirmap, ch)
#define mjolsetdir(ch)  setbit(mjolcmdhasdirmap, ch)
#define mjolhasarg(ch)  bitset(mjolcmdhasargmap, ch)
#define mjolsetarg(ch)  setbit(mjolcmdhasargmap, ch)
extern uint8_t          mjolcmdismovemap[256];
extern uint8_t          mjolcmdhasdirmap[256];
extern uint8_t          mjolcmdhasargmap[256];

#define mjolcanmoveto(type) bitset(mjolcanmovetomap, type)
#define mjolcanpickup(type) bitset(mjolcanpickupmap, type)
#define mjolcanwear(type)   bitset(mjolcanwearmap, type)
#define mjolcanwield(type)  bitset(mjolcanwieldmap, type)
extern uint8_t          mjolcanmovetomap[256];
extern uint8_t          mjolcanpickupmap[256];
extern uint8_t          mjolcanwearmap[256];
extern uint8_t          mjolcanwieldmap[256];

#define MJOL_DEF_NICK   "johndoe"
#define MJOL_LEN_NICK   16

#define MJOL_DEF_NLVL   1
#define MJOL_DEF_WIDTH  80
#define MJOL_DEF_HEIGHT 24

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
#define MJOL_OBJ_SILVER_BULLET    'S'
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
#define MJOL_OBJ_KNIFE            'k'
#define MJOL_OBJ_LOCKPICK         'l'
#define MJOL_OBJ_LASER            'L'
#define MJOL_OBJ_MACE             'm'
#define MJOL_OBJ_MAINFRAME        'M'
#define MJOL_OBJ_PIPE             'p'
#define MJOL_OBJ_PISTOL           'P'
#define MJOL_OBJ_SWORD            's'
#define MJOL_OBJ_TELEPORT         't'
#define MJOL_OBJ_WELL             'w'
#define MJOL_OBJ_CROSS            'x'
#define MJOL_OBJ_ALTAR            'X'

/* characters */
#define MJOL_CHAR_PLAYER          '@'
#define MJOL_CHAR_ANT             'a'
#define MJOL_CHAR_ALIEN           'A'
#define MJOL_CHAR_BEE             'b'
#define MJOL_CHAR_BEE_QUEEN       'B'
#define MJOL_CHAR_DOG             'd'
#define MJOL_CHAR_DEITY           'D'
#define MJOL_CHAR_GHOUL           'G'
#define MJOL_CHAR_HUMAN           'H'
#define MJOL_CHAR_ORACLE          'O'
#define MJOL_OBJ_JATIMATIC        'J'
#define MJOL_CHAR_REMNANTS        'R'
#define MJOL_CHAR_UNICORN         'u'
#define MJOL_CHAR_THOR            'T'
#define MJOL_CHAR_VAMPIRE         'v'
#define MJOL_CHAR_DRACULA         'V'
#define MJOL_CHAR_WOLF            'w'
#define MJOL_CHAR_ZOMBIE          'Z'

/* special items */
#define MJOL_ITEM_DEMON_WHIP      1
#define MJOL_ITEM_JATIMATIC       2
#define MJOL_ITEM_MJOLNIR         3
#define MJOL_ITEM_PLEIGNIR        4
#define MJOL_ITME_STORMBRINGER    5
#define MJOL_ITEM_EXCALIBUR       6
#define MJOL_ITEM_HOLY_CROSS      7

/* commands */
#define ESC                       '\033'
#define CTRL(x)              l    ((x) & 0x80)
/* rogue commands */
#define MJOL_CMD_BOSS             ESC
#define MJOL_CMD_MENU             'm'
#define MJOL_CMD_IDENTIFY         '/'
#define MJOL_CMD_MOVE_LEFT        'h'
#define MJOL_CMD_MOVE_DOWN        'j'
#define MJOL_CMD_MOVE_UP          'k'
#define MJOL_CMD_MOVE_RIGHT       'l'
#define MJOL_CMD_MOVE_UPLEFT      'y'
#define MJOL_CMD_MOVE_UPRIGHT     'u'
#define MJOL_CMD_MOVE_DOWNLEFT    'b'
#define MJOL_CMD_MOVE_DOWNRIGHT   'n'
#if (MJOL_CURSES)
#define MJOL_CMD_LEFT             (KEY_LEFT)
#define MJOL_CMD_DOWN             (KEY_DOWN)
#define MJOL_CMD_UP               (KEY_UP)
#define MJOL_CMD_RIGHT            (KEY_RIGHT)
#endif
#define MJOL_CMD_RUN_LEFT         'H'
#define MJOL_CMD_RUN_DOWN         'J'
#define MJOL_CMD_RUN_UP           'K'
#define MJOL_CMD_RUN_RIGHT        'L'
#define MJOL_CMD_RUN_UPLEFT       'Y'
#define MJOL_CMD_RUN_UPRIGHT      'U'
#define MJOL_CMD_RUN_DOWNLEFT     'B'
#define MJOL_CMD_RUN_DOWNRIGHT    'N'
#define MJOL_CMD_GO_LEFT          CTRL('h')
#define MJOL_CMD_GO_DOWN          CTRL('j')
#define MJOL_CMD_GO_UP            CTRL('k')
#define MJOL_CMD_GO_RIGHT         CTRL('l')
#define MJOL_CMD_GO_UPLEFT        CTRL('y')
#define MJOL_CMD_GO_UPRIGHT       CTRL('u')
#define MJOL_CMD_GO_DOWNLEFT      CTRL('b')
#define MJOL_CMD_GO_DOWNRIGHT     CTRL('n')
#define MJOL_CMD_FIGHT            'f'
#define MJOL_CMD_THROW            't'
#define MJOL_CMD_MOVE             'm'
#define MJOL_CMD_ZAP              'z'
#define MJOL_CMD_IDENTIFY_TRAP    '^'
#define MJOL_CMD_SEARCH           's'
#define MJOL_CMD_LEVEL_DOWN       '>'
#define MJOL_CMD_LEVEL_UP         '<'
#define MJOL_CMD_REST             '.'
#define MJOL_CMD_PICK_UP          ','
#define MJOL_CMD_INVENTORY        'i'
#define MJOL_CMD_INVENTORY_SINGLE 'I'
#define MJOL_CMD_QUAFF            'q'
#define MJOL_CMD_READ             'r'
#define MJOL_CMD_EAT              'e'
#define MJOL_CMD_WIELD            'w'
#define MJOL_CMD_WEAR             'W'
#define MJOL_CMD_TAKE_OFF         'T'
#define MJOL_CMD_PUT_RING_ON      'P'
#define MJOL_CMD_REMOVE_RING      'R'
#define MJOL_CMD_DROP             'd'
#define MJOL_CMD_CALL             'c'
#define MJOL_CMD_REPEAT           'a'
#define MJOL_CMD_PRINT_WEAPON     ')'
#define MJOL_CMD_PRINT_ARMOR      ']'
#define MJOL_CMD_PRINT_RINGS      '='
#define MJOL_CMD_PRINT_STATS      '@'
#define MJOL_CMD_RECALL           'D'
#define MJOL_CMD_OPTIONS          'o'
#define MJOL_CMD_REDRAW           CTRL('R')
#define MJOL_CMD_REPEAT_MESSAGE   CTRL('P')
#define MJOL_CMD_CANCEL           CTRL('[')
#define MJOL_CMD_SAVE             'S'
#define MJOL_CMD_QUIT             'Q'
/* mjolnir extensions */
#define MJOL_CMD_USE              'u'
#define MJOL_CMD_TURN_ON          '0'
#define MJOL_CMD_TURN_OFF         '1'

#define MJOL_SCR_NONE             0
#define MJOL_SCR_VGA_TEXT         1
#define MJOL_SCR_TTY              2
#define MJOL_SCR_X11              3
struct mjolgame {
    struct dnggame      data;
    struct mjolchr     *player;
    char               *nick;           // names of players
    struct mjolobj    **inventory;      // inventory
    long                scrtype;        // type of screen to use
    struct mjolscr     *scr;            // screen interface
    struct mjolchr  ****chrtab;         // in-dungeon live characters
    long                nlvl;           // # of levels
    long                lvl;            // current level
    struct mjolobj  ****objtab;         // level data
    long               *nroomtab;       // per-level # of rooms
    struct mjolroom  ***lvltab;         // level data
    long                width;          // width of level in cells
    long                height;         // height of level
    long                nobj;           // # of objects
};

struct mjolobjfunc {
    char  *str;
    long (*hit)(void *, void *);
    long (*def)(void *, void *);
    long (*pick)(void *, void *);
};

#if 0
/* event handler function prototype */
typedef void mjolfunc_t(struct dnggame *game,
                        struct dngobj *src, struct dngobj *dest);
#endif

/* character flags */
#define MJOL_CHR_NO_PICK   0x00000001U // do not pick object up automatically
#define MJOL_CHR_BLIND     0x00000020U // character is blind
#define MJOL_CHR_LEVITATES 0x00000040U // character is levitating
/* speed values */

#define MJOL_CHR_FAST      2           // character is moving faster
#define MJOL_CHR_NORMAL    1           // normal speed
#define MJOL_CHR_FROZEN    0           // character can't move
#define MJOL_CHR_SLOW      (-1)        // slow speed
struct mjolchr {
    struct dngobj      data;            // common character data
    struct mjolobjfunc func;
    /* Rogue [visible] attributes */
    long               hp;              // current hitpoints
    long               maxhp;           // max hitpoints
    long               gold;            // current gold
    long               str;             // current strength
    long               maxstr;          // max strength
    long               arm;             // armor strength
    long               exp;             // experience
    long               lvl;             // level
    long               aln;             // alignment; CHAOTIC, NEUTRAL, LAWFUL
    /* mjolnir [hidden] attributes */
    long               pwr;
    long               dex;             // dexterity
    long               lock;            // lock-pick skill
    long               intl;            // intelligence
    long               def;             // defense
    long               speed;           // FAST, NORMAL, FROZEN, SLOW
    unsigned long      turn;            // next turn ID
    unsigned long      nturn;           // # of turns used
};

/* data.flg values */
#define MJOL_OBJ_HIDDEN  0x00000001
/* bless values */
#define MJOL_OBJ_BLESSED 1
#define MJOL_OBJ_NEUTRAL 0
#define MJOL_OBJ_CURSED  (-1)
struct mjolobj {
#if (MJOL_VT) || (MJOL_CURSES)
    int                 id;
#endif
    struct dngobj       data;           // common object data
    struct mjolobjfunc  func;
    long                weight;         // weight of object
    long                bless;          // BLESSED, NEUTRAL, CURSED
    long                parm;           // e.g. +1 or -1 for armor
    struct mjolobj     *prev;
    struct mjolobj     *next;
};

#define MJOL_DIR_VERTICAL    0x00000001
#define MJOL_ROOM_CONNECTED  0x00000002
struct mjolroom {
#if (MJOL_ROOM_IDS)
    int              id;
#endif
    long             lvl;
    long             x;
    long             y;
    long             width;
    long             height;
    long             flg;
    long             treelvl;
    struct mjolroom *part1;
    struct mjolroom *part2;
};

struct mjollvl {
    long              nroom;
    struct mjolroom **roomtab;
};

#if (TEST)
static __inline__ void
mjolprintroom(struct mjolroom *room)
{
    fprintf(stderr, "x = %ld, y = %ld, width = %ld, height = %ld\n",
            room->x, room->y, room->width, room->height);
}
#endif

static __inline__ void
mjolpushchase(struct mjolchr *data)
{
    struct mjolchr *next;

    data->data.prev = NULL;
    next = chaseq;
    if (chaseq) {
        chaseq->data.prev = data;
    }
    data->data.next = next;
    chaseq = data;

    return;
}

static __inline__ struct mjolchr *
mjolpopchase(void)
{
    struct mjolchr *next = chaseq->data.next;
    struct mjolchr *data = chaseq;

    if (data) {
        if (next) {
            next->data.prev = NULL;
        }
        chaseq = next;
    }

    return data;
}

static __inline__ void
mjolrmchase(struct mjolchr *data)
{
    struct mjolchr *prev = data->data.prev;
    struct mjolchr *next = data->data.next;

    if (next) {
        next->data.prev = prev;
    }
    if (prev) {
        prev->data.next = next;
    } else {
        if (next) {
            next->data.prev = NULL;
        }
        chaseq = next;
    }

    return;
}

typedef long             mjolcmdfunc(struct mjolchr *src,
                                     struct mjolobj *dest);
typedef struct mjolobj * mjolcmdmovefunc(struct mjolgame *game,
                                         struct mjolchr *chr);

#endif /* __MJOLNIR_MJOL_H__ */

