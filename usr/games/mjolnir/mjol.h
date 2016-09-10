#ifndef __MJOLNIR_MJOL_H__
#define __MJOLNIR_MJOL_H__

#if (TEST)
#include <stdio.h>
#endif
#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#include <zero/trix.h>
#include <mjolnir/conf.h>
#if (MJOL_USE_ZERO_RANDMT32)
#include <zero/randmt32.h>
#endif
#if (MJOL_CURSES)
#include <curses.h>
#endif
#include <dungeon/dng.h>
#include <mjolnir/obj.h>
#include <mjolnir/chr.h>

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
extern uint8_t          mjolcanmovetomap[256 / CHAR_BIT];
extern uint8_t          mjolcanpickupmap[256 / CHAR_BIT];
extern uint8_t          mjolcanwearmap[256 / CHAR_BIT];
extern uint8_t          mjolcanwieldmap[256 / CHAR_BIT];

#define MJOL_DEF_NICK   "johndoe"
#define MJOL_LEN_NICK   16

#define MJOL_DEF_NLVL   1
#define MJOL_DEF_WIDTH  80
#define MJOL_DEF_HEIGHT 24

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

#define MJOL_DIR_VERTICAL    0x00000001
#define MJOL_ROOM_CONNECTED  0x00000002
struct mjolroom {
#if (MJOL_ROOM_IDS)
    int              id;
    int              lvlid;
#endif
    long             lvl;
    long             x;
    long             y;
    long             width;
    long             height;
#if (MJOLHACKS)
    long             roomx;
    long             roomy;
    long             roomw;
    long             roomh;
#endif
    long             flg;
//    struct mjolroom *parent;
    struct mjolroom *part1;
    struct mjolroom *part2;
    struct mjolroom *next;
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

typedef struct mjolobj  * mjolcmdfunc(struct mjolchr *chr,
                                      struct mjolobj *dest);
typedef struct mjolobj  * mjolcmdmovefunc(struct mjolgame *game,
                                          struct mjolchr *chr);

extern void               mjolquitsig(int sig);
extern void               mjolinit(struct mjolgame *game,
                                   int argc, char *argv[]);
extern void               mjolgetopt(struct mjolgame *game,
                                     int argc, char *argv[]);
extern struct mjolchr   * mjolmkplayer(void);
extern void               mjolopenscr(struct mjolgame *game);
extern void               mjolinitcmd(void);
extern void               mjolinitobj(void);
extern struct mjolroom ** mjolinitrooms(struct mjolgame *game, long *nroom);
void                      mjolgendng(struct mjolgame *game);
extern void               mjoldocmd(struct mjolgame *game, int ch);
extern long               mjoldoturn(struct mjolgame *game,
                                     struct mjolchr *data);
extern long               mjolchaseall(struct mjolgame *game);

extern void               dnginitobjmjol(void);
extern void               mjolgameloop(struct mjolgame *game);

#endif /* __MJOLNIR_MJOL_H__ */

