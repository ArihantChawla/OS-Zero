#ifndef __MJOLNIR_MJOL_H__
#define __MJOLNIR_MJOL_H__

#include <stddef.h>
#include <dungeon/dng.h>

#define MJOL_DEF_NICK   "johndoe"
#define MJOL_LEN_NICK   16

#define MJOL_DEF_NLVL   64
#define MJOL_DEF_WIDTH  80
#define MJOL_DEF_HEIGHT 24

/* objects */
#define MJOL_OBJ_FLOOR           '.'
#define MJOL_OBJ_CORRIDOR        '#'
#define MJOL_OBJ_HORIZONTAL_WALL '-'
#define MJOL_OBJ_VERTICAL_WALL   '|'
#define MJOL_OBJ_DOOR            '+'
#define MJOL_OBJ_FOOD            '%'
#define MJOL_OBJ_WATER           '~'
#define MJOL_OBJ_GOLD            '$'
#define MJOL_OBJ_SILVER_BULLET   '§'
#define MJOL_OBJ_POTION          '!'
#define MJOL_OBJ_PLANT           '*'
#define MJOL_OBJ_PUNCHCARD       '='
#define MJOL_OBJ_DOWN            '<'
#define MJOL_OBJ_UP              '>'
#define MJOL_OBJ_STATUE          '&'
#define MJOL_OBJ_TRAP            '^'
#define MJOL_OBJ_WAND            '\\'
#define MJOL_OBJ_SCROLL          '?'
#define MJOL_OBJ_RING            '='
#define MJOL_OBJ_CHAIN           '8'
#define MJOL_OBJ_CHEST           'C'
#define MJOL_OBJ_SUBMACHINE_GUN  'g'
#define MJOL_OBJ_HONEY           'h'
#define MJOL_OBJ_KNIFE           'k'
#define MJOL_OBJ_LOCKPICK        'l'
#define MJOL_OBJ_LASER           'L'
#define MJOL_OBJ_MAINFRAME       'm'
#define MJOL_OBJ_MACE            'M'
#define MJOL_OBJ_PIPE            'p'
#define MJOL_OBJ_PISTOL          'P'
#define MJOL_OBJ_SWORD           's'
#define MJOL_OBJ_WELL            'w'
#define MJOL_OBJ_CROSS           'x'
#define MJOL_OBJ_ALTAR           'X'

/* characters */
#define MJOL_CHAR_PLAYER         '@'
#define MJOL_CHAR_ANT            'a'
#define MJOL_CHAR_ALIEN          'A'
#define MJOL_CHAR_BEE            'b'
#define MJOL_CHAR_BEE_QUEEN      'B'
#define MJOL_CHAR_DOG            'd'
#define MJOL_CHAR_DEITY          'D'
#define MJOL_CHAR_GHOUL          'G'
#define MJOL_CHAR_HUMAN          'H'
#define MJOL_CHAR_ORACLE         'O'
#define MJOL_CHAR_REMNANTS       'R'
#define MJOL_CHAR_UNICORN        'u'
#define MJOL_CHAR_VAMPIRE        'v'
#define MJOL_CHAR_DRACULA        'V'
#define MJOL_CHAR_WOLF           'W'
#define MJOL_CHAR_ZOMBIE         'Z'

/* commands */
#define CTRL(x) ((x) & 0x80)
/* rogue commands */
#define MJOL_CMD_IDENTIFY         '/'
#define MJOL_CMD_LEFT             'h'
#define MJOL_CMD_DOWN             'j'
#define MJOL_CMD_UP               'k'
#define MJOL_CMD_RIGHT            'l'
#define MJOL_CMD_UPLEFT           'y'
#define MJOL_CMD_UPRIGHT          'u'
#define MJOL_CMD_DOWNLEFT         'b'
#define MJOL_CMD_DOWNRIGHT        'n'
#define MJOL_CMD_RUN_LEFT         'H'
#define MJOL_CMD_RUN_DOWN         'J'
#define MJOL_CMD_RUN_UP           'K'
#define MJOL_CMD_RUN_RIGHT        'L'
#define MJOL_CMD_RUN_UPLEFT       'Y'
#define MJOL_CMD_RUN_UPRIGHT      'U'
#define MJOL_CMD_RUN_DOWNLEFT     'B'
#define MJOL_CMD_RUN_DOWNRIGHT    'N'
#define MJOL_CMD_GO_LEFT          'h'
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
    struct dnggame        data;
    char                **nicks;        // names of players
    long                  scrtype;      // type of screen to use
    struct mjolgamescr   *scr;          // screen infrastructure
    size_t                nlvl;         // # of levels
    size_t                width;        // width of level in cells
    size_t                height;       // height of level
    size_t                nobj;         // # of objects
    struct mjolobj     ***objtab;       // objects on the level
};

/* event handler function prototype */
typedef void mjolfunc_t(struct dnggame *game,
                        struct dngobj *src, struct dngobj *dest);

/* character flags */
#define MJOL_CHAR_NO_PICK   0x00000001U // do not pick object up automatically
#define MJOL_CHAR_BLIND     0x00000020U // character is blind
#define MJOL_CHAR_LEVITATES 0x00000040U // character is levitating
/* speed values */

#define MJOL_CHAR_FAST      2           // character is moving faster
#define MJOL_CHAR_NORMAL    1           // normal speed
#define MJOL_CHAR_FROZEN    0           // character can't move
#define MJOL_CHAR_SLOW      (-1)        // slow speed
struct mjolchar {
    struct dngobj data;                 // common character data
    /* Rogue attributes */
    long          hp;                   // current hitpoints
    long          maxhp;                // max hitpoints
    long          gold;                 // current gold
    long          str;                  // current strength
    long          maxstr;               // max strength
    long          arm;                  // armor strength
    long          exp;                  // experience
    long          lvl;                  // level
    /* mjolnir attributes */
    long          turn;                 // next turn ID
    long          nturn;                // # of turns used
    long          speed;                // FAST, NORMAL, FROZEN, SLOW
#if 0
    long          dex;                  // dexterity
    long          lock;                 // lock-pick skill
#endif
};

/* determine how many continuous turns a character has */
static __inline__ long
mjolcharnturn(struct mjolchar *chardata)
{
    long speed = chardata->speed;
    long turn;
    long retval = 0;

    if (speed < 0) {
        turn = chardata->turn;
        /* only move every abs(speed) turns */
        if (chardata->nturn == turn) {
            /* allow movement */
            retval = 1;
            turn -= speed;
            chardata->turn = turn;
        }
    } else {
        /* return speed */
        retval = speed;
    }

    return retval;
}

/* data.flg values */
#define MJOL_OBJ_HIDDEN  0x00000001
/* bless values */
#define MJOL_OBJ_BLESSED 1
#define MJOL_OBJ_NEUTRAL 0
#define MJOL_OBJ_CURSED  (-1)
struct mjolobj {
    struct dngobj data;         // common object data
    long          weight;       // weight of object
    long          bless;        // BLESSED, NEUTRAL, CURSED
    long          parm;         // e.g. +1 or -1 for armor
};

#define MJOLNOBJSTK 14
struct mjolobjstk {
    long  top;                  // cached top object character
    long  cur;                  // current stack index
    char *stk[MJOLNOBJSTK];     // object stack
};

struct mjolrect {
    long             x;
    long             y;
    long             width;
    long             height;
    struct mjolrect *left;
    struct mjolrect *right;
};

#endif /* __MJOLNIR_MJOL_H__ */

