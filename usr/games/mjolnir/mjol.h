#ifndef __MJOLNIR_MJOL_H__
#define __MJOLNIR_MJOL_H__

/* object types/characters */
#define MJOL_OBJ_FLOOR          '.'
#define MJOL_OBJ_CORRIDOR       '#'
#define MJOL_OBJ_ALTAR          '+'
#define MJOL_OBJ_WATER          '~'
#define MJOL_OBJ_GOLD           '$'
#define MJOL_OBJ_POTION         '?'
#define MJOL_OBJ_PLANT          '*'
#define MJOL_OBJ_PUNCHCARD      '='
#define MJOL_OBJ_SILVER         '%'
#define MJOL_OBJ_DOWN           '<'
#define MJOL_OBJ_UP             '>'
#define MJOL_OBJ_STATUE         '&'
#define MJOL_OBJ_TRAP           '^'
#define MJOL_OBJ_SWORD          '|'
#define MJOL_OBJ_WAND           '\\'
#define MJOL_OBJ_ANT            'a'
#define MJOL_OBJ_ALIEN          'A'
#define MJOL_OBJ_BEE            'b'
#define MJOL_OBJ_BEE_QUEEN      'B'
#define MJOL_OBJ_CAT            'c'
#define MJOL_OBJ_CHEST          'C'
#define MJOL_OBJ_DOG            'd'
#define MJOL_OBJ_DEITY          'D'
#define MJOL_OBJ_FOOD           'f'
#define MJOL_OBJ_SUBMACHINE_GUN 'g'
#define MJOL_OBJ_GHOUL          'G'
#define MJOL_OBJ_HONEY          'h'
#define MJOL_OBJ_HUMAN          'H'
#define MJOL_OBJ_KNIFE          'k'
#define MJOL_OBJ_LOCKPICK       'l'
#define MJOL_OBJ_LASER          'L'
#define MJOL_OBJ_MAINFRAME      'm'
#define MJOL_OBJ_MACE           'M'
#define MJOL_OBJ_SCROLL         'o'
#define MJOL_OBJ_ORACLE         'O'
#define MJOL_OBJ_PISTOL         'p'
#define MJOL_OBJ_PIPE           'P'
#define MJOL_OBJ_RING           'r'
#define MJOL_OBJ_UNICORN        'u'
#define MJOL_OBJ_VAMPIRE        'v'
#define MJOL_OBJ_WELL           'w'
#define MJOL_OBJ_WOLF           'W'
#define MJOL_OBJ_ZOMBIE         'Z'
/* object flags */
#define MJOL_FLG_PICK_UP         0x00000001U    // pick object up automatically
#define MJOL_FLG_LEVITATES       0x00000002U    // character is levitating
#define MJOL_FLG_FROZEN          0x00000004U    // character can't move
#define MJOL_FLG_BLIND           0x00000008U    // character is blind

#endif /* __MJOLNIR_MJOL_H__ */

