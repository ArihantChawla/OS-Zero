#ifndef __MJOLNIR_CONF_H__
#define __MJOLNIR_CONF_H__

#define MJOL_ENGLISH_LANGUAGE  1
#define MJOL_VGA_TEXT          0
#define MJOL_TTY               1
#define MJOL_X11               0
#define MJOL_USE_COLORS        1
#define MJOL_USE_ZERO_RANDMT32 1
#if (MJOL_TTY)
#define MJOL_VT                1
#define MJOL_CURSES            1
#endif

/* PLEASE DO NOT EDIT BELOW */

#define MJOL_ROOM_IDS          0
#define MJOL_HASH              0
#if (MJOL_USE_ZERO_RANDMT32)
#define mjolsrand(seed) srandmt32(seed)
#define mjolrand()      (randmt32() & 0x7fffffff)
#else
#define mjolsrand(seed) srand(seed)
#define mjolrand()      (rand() & 0x7fffffff)
#endif

#endif /* __MJOLNIR_CONF_H__ */

