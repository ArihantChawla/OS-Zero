#ifndef __CPUNK_ITEM_H__
#define __CPUNK_ITEM_H__

#include <cpunk.h>

/* text colors */
#define CPUNK_BLACK           0
#define CPUNK_RED             1         // alerts
#define CPUNK_GREEN           2         // player, doors and exits, walls
#define CPUNK_YELLOW          3         // objects
#define CPUNK_BLUE            4         // water
#define CPUNK_MAGENTA         5         // non-player characters, unknown items
#define CPUNK_CYAN            6         // food
#define CPUNK_WHITE           7         // messages, default color
#define CPUNK_NCOLOR          8

#define NFLAGWORD             4
/* items and characters */
#define CPUNK_GROUND          '.'
#define CPUNK_CODE            ';'
#define CPUNK_ALTAR           '+'
#define CPUNK_SCROLL          '~'
#define CPUNK_DOOR            '^'
#define CPUNK_WALL            '#'
#define CPUNK_LOCKPICK        '-'
#define CPUNK_SWORD           '|'
#define CPUNK_GOLD            '$'
#define CPUNK_UNKNOWN         '?'
#define CPUNK_TELEPORT        '@'
#define CPUNK_CALCULATOR      '='

#define CPUNK_ALIEN           'A'
#define CPUNK_BOOK            'B'
#define CPUNK_CYBORG          'C'
#define CPUNK_DESKTOP         'D'
#define CPUNK_MAINFRAME       'F'
#define CPUNK_HUMAN           'H'
#define CPUNK_KEY             'K'
#define CPUNK_LAPTOP          'L'
#define CPUNK_MOBILE          'M'
#define CPUNK_ORACLE          'O'
#define CPUNK_PLAYER          'P'
#define CPUNK_ROBOT           'R'
#define CPUNK_TERMINAL        'T'
#define CPUNK_SUPER           'S'
#define CPUNK_VIXEN           'V'
#define CPUNK_WIZARD          'W'

#define CPUNK_CAT             'c'
#define CPUNK_DOG             'd'
#define CPUNK_HERB            'g'
#if (CPUNK_ADULT)
#define CPUNK_HOOKER          'h'
#define CPUNK_PIMP            'p'
#endif
#define CPUNK_LIZARD          'l'
#define CPUNK_HOLE            'o'
#define CPUNK_RAT             'r'
#define CPUNK_SPRING          's'
#define CPUNK_WELL            'w'
#define CPUNK_FOOD            'f'

/* commands */
#define CPUNK_UPSTAIRS        ">"
#define CPUNK_DOWNSTAIRS      "<"
#define CPUNK_ICE             "i"
#define CPUNK_WHAT            "w"
#define CPUNK_CRYPT           "c"
#define CPUNK_UPLOAD          "u"
#define CPUNK_DOWNLOAD        "d"
#define CPUNK_RUN             "r"
#define CPUNK_CHAT            "c"
#define CPUNK_DEBUG           "b"
#define CPUNK_EMULATE         "e"
#define CPUNK_HACK            "h"

#define cpunksetaction(ip, action) setbit(&(ip)->aflags, action)
struct cpunkitem {
    long          type;                 // item type
    long          aflags[NFLAGWORD];    // supported actions (bitmap)
    unsigned char ch;                   // character to print in text-mode
};

#endif /* __CPUNK_ITEM_H__ */

