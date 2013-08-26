#ifndef __MJOLNIR_MJOL_H__
#define __MJOLNIR_MJOL_H__

/* object types/characters */
#define MJOL_OBJ_PLAYER          '@'
#define MJOL_OBJ_FLOOR           '.'
#define MJOL_OBJ_CORRIDOR        '#'
#define MJOL_OBJ_ALTAR           '+'
#define MJOL_OBJ_WATER           '~'
#define MJOL_OBJ_GOLD            '$'
#define MJOL_OBJ_POTION          '?'
#define MJOL_OBJ_PLANT           '*'
#define MJOL_OBJ_PUNCHCARD       '='
#define MJOL_OBJ_SILVER          '%'
#define MJOL_OBJ_DOWN            '<'
#define MJOL_OBJ_UP              '>'
#define MJOL_OBJ_STATUE          '&'
#define MJOL_OBJ_TRAP            '^'
#define MJOL_OBJ_SWORD           '|'
#define MJOL_OBJ_WAND            '\\'
#define MJOL_OBJ_ANT             'a'
#define MJOL_OBJ_ALIEN           'A'
#define MJOL_OBJ_BEE             'b'
#define MJOL_OBJ_BEE_QUEEN       'B'
#define MJOL_OBJ_CAT             'c'
#define MJOL_OBJ_CHEST           'C'
#define MJOL_OBJ_DOG             'd'
#define MJOL_OBJ_DEITY           'D'
#define MJOL_OBJ_FOOD            'f'
#define MJOL_OBJ_SUBMACHINE_GUN  'g'
#define MJOL_OBJ_GHOUL           'G'
#define MJOL_OBJ_HONEY           'h'
#define MJOL_OBJ_HUMAN           'H'
#define MJOL_OBJ_KNIFE           'k'
#define MJOL_OBJ_LOCKPICK        'l'
#define MJOL_OBJ_LASER           'L'
#define MJOL_OBJ_MAINFRAME       'm'
#define MJOL_OBJ_MACE            'M'
#define MJOL_OBJ_SCROLL          'o'
#define MJOL_OBJ_ORACLE          'O'
#define MJOL_OBJ_PISTOL          'p'
#define MJOL_OBJ_PIPE            'P'
#define MJOL_OBJ_RING            'r'
#define MJOL_OBJ_UNICORN         'u'
#define MJOL_OBJ_VAMPIRE         'v'
#define MJOL_OBJ_WELL            'w'
#define MJOL_OBJ_WOLF            'W'
#define MJOL_OBJ_ZOMBIE          'Z'

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
#define MJOL_CMD_GO_DOWN          '>'
#define MJOL_CMD_GO_UP            '<'
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
#define MJOLNIR_CMD_USE           'u'
#define MJOLNIR_CMD_TURN_ON       '0'
#define MJOLNIR_CMD_TURN_OFF      '1'

/* event handler function prototype */
typedef void mjolfunc_t(struct dngobj *src, struct dngobj *dest);

/* character flags */
#define MJOL_CHAR_PICK_UP   0x00000001U // pick object up automatically
#define MJOL_CHAR_BLIND     0x00000020U // character is blind
#define MJOL_CHAR_LEVITATES 0x00000040U // character is levitating
/* speed values */
#define nturn(cp)           ((cp)->speed)
#define MJOL_CHAR_FAST      2           // character is moving faster
#define MJOL_CHAR_NORMAL    1           // normal speed
#define MJOL_CHAR_FROZEN    0           // character can't move
struct mjolchardata {
    long flg;           // character flags
    /* Rogue attributes */
    long hp;            // current hitpoints
    long maxhp;         // max hitpoints
    long gold;          // current gold
    long str;           // current strength
    long maxstr;        // max strength
    long arm;           // armor strength
    long exp;           // experience
    long lvl;           // level
    /* mjolnir attributes */
    long turn;          // next turn ID
    long speed;         // FAST, NORMAL, FROZEN
#if 0
    long dex;           // dexterity
    long lock;          // lock-pick skill
#endif
};

#define MJOL_OBJ_BLESSED 1
#define MJOL_OBJ_NEUTRAL 0
#define MJOL_OBJ_CURSED  (-1)
struct mjolobjdata {
    long  weight;       // weight of object
    long  bless;        // BLESSED, NEUTRAL, CURSED
    long  parm;         // e.g. +1 or -1 for armor
}

#endif /* __MJOLNIR_MJOL_H__ */

