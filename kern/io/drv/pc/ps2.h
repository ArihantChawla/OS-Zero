#ifndef __KERN_IO_DRV_PC_PS2_H__
#define __KERN_IO_DRV_PC_PS2_H__

#include <kern/conf.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <zero/ring.h>

#define PS2KBD_NTAB                 128

/* KEYBOARD */
#define PS2KBD_PORT                 0x60

/* keyboard commands */
#define PS2KBD_RESET                0xffU
#define PS2KBD_SETDEFAULT           0xf6U
#define PS2KBD_RESEND               0xfeU
#define PS2KBD_ENABLE               0xf4U
#define PS2KBD_SETSCAN              0xf0U
#define PS2KBD_SETKEYMODE           0xf8U /* all typematic, make, break */
#define PS2KBD_TYPEMATIC            0xf3U
#define PS2KBD_RATEANDDELAY         0x00U /* 250 ms, 30x/sec */

/* protocol codes */
#define PS2KBD_ERROR                0x00U
#define PS2KBD_ECHO                 0xeeU
#define PS2KBD_BAT                  0xaaU
#define PS2KBD_ACK                  0xfaU
#define PS2KBD_BATORMOUSEERR        0xfcU
#define PS2KBD_INTERR               0xfdU
#define PS2KBD_ACKFAIL              0xfeU
#define PS2KBD_ERROR2               0xffU

#define PS2KBD_VAL_MASK             0xffU
#define PS2KBD_PREFIX_BYTE          0xe0U
#define PS2KBD_UP_BIT               0x80U
#define PS2KBD_UP_BYTE              0xf0U

#define PS2KBD_SHIFT_CASE_BYTE3     0x12U
#define PS2KBD_NUM_CASE_BYTE2       0x12U

#define PS2KBD_PAUSE_BYTE1          0xe1U
#define PS2KBD_PAUSE_UP_BYTE2       0x14U
/* after 0xe0 */
//#define PS2KBD_CTRLPAUSE_BYTE2      0x46U
#define PS2KBD_PRINT_BYTE2          0x12U
#define PS2KBD_PRINT_BYTE3          0xe0U
#define PS2KBD_PRINT_UP_BYTE3       0x7cU

/* invalid. */
#define PS2KBD_INVAL_SYM            0x00U

#if 0
#define PS2KBD_LEFTCTRL_FLAG        0x01
#define PS2KBD_LEFTSHIFT_FLAG       0x02
#define PS2KBD_RIGHTSHIFT_FLAG      0x04
#define PS2KBD_LEFTALT_FLAG         0x08
#define PS2KBD_RIGHTALT_FLAG        0x10
#define PS2KBD_CAPSLOCK_FLAG        0x20
#define PS2KBD_NUMLOCK_FLAG         0x40
#define PS2KBD_SCROLLLOCK_FLAG      0x80
#endif

/* US scancodes for set 2. */

#if (KBDUS) && (PS2KBDSET2)

#define kbdupcode(down)                                                 \
    ((!((down) & 0xffffff00))                                           \
     ? ((down) | 0x00000080)                                            \
     : ((((down) & 0xffff00ff) == 0x000000e0)                           \
        ? (0x0000f0e0 | (((down) & 0x0000ff00) << 8))                   \
        : (((down) == 0x7ce012e0)                                       \
           ? UINT64_C(0x12f0e07cf0e0)                                   \
           : PS2KBD_NOCODE)))

/* alphabets */
#define PS2KBD_A                    0x1c
#define PS2KBD_B                    0x32
#define PS2KBD_C                    0x21
#define PS2KBD_D                    0x23
#define PS2KBD_E                    0x24
#define PS2KBD_F                    0x2b
#define PS2KBD_G                    0x34
#define PS2KBD_H                    0x33
#define PS2KBD_I                    0x43
#define PS2KBD_J                    0x3b
#define PS2KBD_K                    0x42
#define PS2KBD_L                    0x4b
#define PS2KBD_M                    0x3a
#define PS2KBD_N                    0x31
#define PS2KBD_O                    0x44
#define PS2KBD_P                    0x4d
#define PS2KBD_Q                    0x15
#define PS2KBD_R                    0x2d
#define PS2KBD_S                    0x1b
#define PS2KBD_T                    0x2c
#define PS2KBD_U                    0x3c
#define PS2KBD_V                    0x2a
#define PS2KBD_W                    0x1d
#define PS2KBD_X                    0x22
#define PS2KBD_Y                    0x35
#define PS2KBD_Z                    0x1a
/* digits */
#define PS2KBD_0                    0x45
#define PS2KBD_1                    0x16
#define PS2KBD_2                    0x1e
#define PS2KBD_3                    0x26
#define PS2KBD_4                    0x25
#define PS2KBD_5                    0x2e
#define PS2KBD_6                    0x36
#define PS2KBD_7                    0x3d
#define PS2KBD_8                    0x3e
#define PS2KBD_9                    0x46
/* special characters */
#define PS2KBD_BACKQUOTE            0x0e
#define PS2KBD_MINUS                0x4e
#define PS2KBD_EQUAL                0x55
#define PS2KBD_BACKSLASH            0x5d
/* backspace */
#define PS2KBD_BACKSPACE            0x66
/* modifier keys */
#define PS2KBD_LEFTSHIFT            0x12
#define PS2KBD_LEFTCTRL             0x14
#define PS2KBD_LEFTALT              0x11
#define PS2KBD_RIGHTSHIFT           0x59
#define PS2KBD_RIGHTCTRL            0x14e0
#define PS2KBD_RIGHTALT             0x11e0
/* state modifiers */
#define PS2KBD_CAPSLOCK             0x58
#define PS2KBD_NUMLOCK              0x77
/* enter */
#define PS2KBD_ENTER                0x5a
/* ESC */
#define PS2KBD_ESC                  0x76
/* function keys */
#define PS2KBD_F1                   0x05
#define PS2KBD_F2                   0x06
#define PS2KBD_F3                   0x04
#define PS2KBD_F4                   0x0c
#define PS2KBD_F5                   0x03
#define PS2KBD_F6                   0x0b
#define PS2KBD_F7                   0x83
#define PS2KBD_F8                   0x0a
#define PS2KBD_F9                   0x01
#define PS2KBD_F10                  0x09
#define PS2KBD_F11                  0x78
#define PS2KBD_F12                  0x07
/* special keys */
#define PS2KBD_PRINT                0x7ce012e0
#define PS2KBD_SCROLL               0x7e
#define PS2KBD_PAUSE                0xe17714e1
#define PS2KBD_TAB                  0x0d
#define PS2KBD_OPENBRACKET          0x54
#define PS2KBD_CLOSEBRACKET         0x5b
#define PS2KBD_SPACE                0x29
#define PS2KBD_MENUS                0x2fe0
#define PS2KBD_INSERT               0x70e0
#define PS2KBD_HOME                 0x6ce0
#define PS2KBD_PAGEUP               0x7de0
#define PS2KBD_PAGEDOWN             0x7ae0
#define PS2KBD_DELETE               0x71e0
#define PS2KBD_END                  0x69e0
#define PS2KBD_UP                   0x75e0
#define PS2KBD_DOWN                 0x72e0
#define PS2KBD_LEFT                 0x6be0
#define PS2KBD_RIGHT                0x74e0
#define PS2KBD_DOT                  0x49
#define PS2KBD_SLASH                0x4a
#define PS2KBD_COMMA                0x41
#define PS2KBD_SEMICOLON            0x4c
#define PS2KBD_QUOTE                0x52
#define PS2KBD_KEYPADPLUS           0x79
#define PS2KBD_KEYPADMINUS          0x7b
#define PS2KBD_KEYPADASTERISK       0x7c
#define PS2KBD_KEYPADSLASH          0x4ae0
#define PS2KBD_KEYPAD0              0x70
#define PS2KBD_KEYPAD1              0x69
#define PS2KBD_KEYPAD2              0x72
#define PS2KBD_KEYPAD3              0x7a
#define PS2KBD_KEYPAD4              0x6b
#define PS2KBD_KEYPAD5              0x73
#define PS2KBD_KEYPAD6              0x74
#define PS2KBD_KEYPAD7              0x6c
#define PS2KBD_KEYPAD8              0x75
#define PS2KBD_KEYPAD9              0x7d
#define PS2KBD_KEYPADENTER          0x5ae0
#define PS2KBD_KEYPADDOT            0x71
/* extra keys */
#define PS2KBD_APPS                 0x2fe0
#define PS2KBD_LEFTGUI              0x1fe0
#define PS2KBD_RIGHTGUI             0x27e0
#define PS2KBD_LEFTWIN              0x1fe0
#define PS2KBD_RIGHTWIN             0x27e0
/* acpi codes. */
#define PS2KBD_POWER                0x37e0
#define PS2KBD_SLEEP                0x3fe0
#define PS2KBD_WAKE                 0x5ee0

/* windows multimedia codes. */

#endif /* (KBDUS) && (PS2KBDSET2) */

/* MOUSE */

#define PS2MOUSE_INPORT    0x60

/* state. */
#define PS2MOUSE_LEFTBTN   0x01 /* left button flag. */
#define PS2MOUSE_RIGHTBTN  0x02 /* right button flag. */
#define PS2MOUSE_MIDDLEBTN 0x04 /* middle button flag. */
#define PS2MOUSE_3BTNMASK  0x07 /* 3-button mask. */
#define PS2MOUSE_BTN4      0x08 /* button 4 flag. */
#define PS2MOUSE_BTN5      0x10 /* button 5 flag. */
#define PS2MOUSE_XBTNMASK  0x18 /* extra button (4 & 5) mask. */
/* other data. */
#define PS2MOUSE_XSIGN     0x10 /* x-movement sign. */
#define PS2MOUSE_YSIGN     0x20 /* y-movement sign. */
#define PS2MOUSE_XOVERFLOW 0x40 /* x-movement overflow. */
#define PS2MOUSE_YOVERFLOW 0x80 /* y-movement overflow. */
/* extra byte. */
#define PS2MOUSE_ZSIGN     0x08 /* z-movement sign. */
#define PS2MOUSE_ZMASK     0x07 /* z-movement mask. */
#define PS2MOUSE_5BTNMASK  0x30 /* extra button (4 & 5) mask. */
/* flags. */
#define PS2MOUSE_WHEEL     0x01 /* scroll wheel flag. */
#define PS2MOUSE_WHEEL5BTN 0x02 /* 5-button flag. */
#define PS2MOUSE_WHEELMASK 0x03 /* intellimouse mask. */
struct ps2mousestate {
    uint16_t flg;
    uint16_t state;
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t xmax;
    uint32_t ymax;
    uint32_t zmax;
    int32_t shift;
};

#if 0
struct ps2drv {
#if 0
    uint64_t          keybuf[PS2KBD_NBUFKEY];
    int32_t           keyvalbuf[PS2KBD_NBUFKEY];
#endif
    volatile long     keylk;
    uint64_t         *curkey;
    uint64_t         *lastkey;
    int32_t          *curkeyval;
    int32_t          *lastkeyval;
/* modifier keys. */
    int32_t           keytabmod[PS2KBD_NTAB];
/* single-code values. */
    int32_t           keytab1b[PS2KBD_NTAB] ALIGNED(PAGESIZE);
/* 0xe0-prefixed values. */
    int32_t           keytabmb[PS2KBD_NTAB];
/* release values. */
    int32_t           keytabup[PS2KBD_NTAB];
    struct mousestate mousestate;
    int32_t           modmask;
} ALIGNED(PAGESIZE);
#endif

struct ps2drv {
    uint32_t              modtab[PS2KBD_NTAB];
    uint32_t              symtab1b[PS2KBD_NTAB];
    uint32_t              symtab2b[PS2KBD_NTAB];
    uint32_t              symtabshift[PS2KBD_NTAB];
    uint32_t              symtabnum[PS2KBD_NTAB];
    struct ringbuf       *buf;
    struct ps2mousestate  mousestate;
};

#endif /* __KERN_IO_DRV_PC_PS2_H__ */

