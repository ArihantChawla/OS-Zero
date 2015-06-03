#ifndef __MJOLNIR_CHR_H__
#define __MJOLNIR_CHR_H__

#include <dungeon/dng.h>
#include <mjolnir/obj.h>

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
#define MJOL_CHAR_REMNANTS        'R'
#define MJOL_CHAR_UNICORN         'u'
#define MJOL_CHAR_THOR            'T'
#define MJOL_CHAR_VAMPIRE         'v'
#define MJOL_CHAR_DRACULA         'V'
#define MJOL_CHAR_WOLF            'w'
#define MJOL_CHAR_ZOMBIE          'Z'

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

#endif /* __MJOLNIR_CHR_H__ */

