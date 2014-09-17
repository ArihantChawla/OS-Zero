#ifndef __KERN_IO_DRV_PC_PS2_H__
#define __KERN_IO_DRV_PC_PS2_H__

#include <zero/param.h>
#include <zero/cdecl.h>

/* KEYBOARD */

//#define PS2KBD_NBUFKEY              (PAGESIZE >> 3)

#define PS2KBD_NTAB                 128

#define PS2KBD_US                   0x00

#define PS2KBD_PORT                 0x60

/* keyboard commands */
#define PS2KBD_ENABLE               0xf4
#define PS2KBD_SETSCAN              0xf0

/* protocol scancodes */
#define PS2KBD_ERROR                0x00
#define PS2KBD_BAT                  0xaa
#define PS2KBD_ECHO                 0xee
#define PS2KBD_ACK                  0xfa
#define PS2KBD_BATORMOUSEERR        0xfc
#define PS2KBD_INTERR               0xfd
#define PS2KBD_ACKFAIL              0xfe
#define PS2KBD_ERROR2               0xff

#define PS2KBD_VAL_MASK             0xff
#define PS2KBD_PREFIX_BYTE          0xe0
#define PS2KBD_UP_BIT               0x80
#define PS2KBD_UP_BYTE              0xf0

#define PS2KBD_PAUSE_BYTE1          0xe1
/* after 0xe0 */
#define PS2KBD_CTRLPAUSE_BYTE2      0x46
#define PS2KBD_PRINT_BYTE2          0x2a

/* invalid. */
#define PS2KBD_INVAL_SYM            0x00

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

/* US scancodes. */

#if (KBDUS)

/* single-byte values. */

#define PS2KBD_ESC                  0x01
#define PS2KBD_1                    0x02
#define PS2KBD_2                    0x03
#define PS2KBD_3                    0x04
#define PS2KBD_4                    0x05
#define PS2KBD_5                    0x06
#define PS2KBD_6                    0x07
#define PS2KBD_7                    0x08
#define PS2KBD_8                    0x09
#define PS2KBD_9                    0x0a
#define PS2KBD_0                    0x0b
#define PS2KBD_MINUS                0x0c
#define PS2KBD_PLUS                 0x0d
#define PS2KBD_BACKSPACE            0x0e

#define PS2KBD_TAB                  0x0f
#define PS2KBD_q                    0x10
#define PS2KBD_w                    0x11
#define PS2KBD_e                    0x12
#define PS2KBD_r                    0x13
#define PS2KBD_t                    0x14
#define PS2KBD_y                    0x15
#define PS2KBD_u                    0x16
#define PS2KBD_i                    0x17
#define PS2KBD_o                    0x18
#define PS2KBD_p                    0x19
#define PS2KBD_OPENBRACKET          0x1a   // [{
#define PS2KBD_CLOSEBRACKET         0x1b   // ]}

#define PS2KBD_ENTER                0x1c

#define PS2KBD_LEFTCTRL             0x1d

#define PS2KBD_a                    0x1e
#define PS2KBD_s                    0x1f
#define PS2KBD_d                    0x20
#define PS2KBD_f                    0x21
#define PS2KBD_g                    0x22
#define PS2KBD_h                    0x23
#define PS2KBD_j                    0x24
#define PS2KBD_k                    0x25
#define PS2KBD_l                    0x26
#define PS2KBD_SEMICOLON            0x27   // ;:

#define PS2KBD_QUOTE                0x28   // '"

/* modifier scancodes. */

#define PS2KBD_BACKQUOTE            0x29   // `~

#define PS2KBD_LEFTSHIFT            0x2a

#define PS2KBD_BACKSLASH            0x2b   // \|

#define PS2KBD_z                    0x2c
#define PS2KBD_x                    0x2d
#define PS2KBD_c                    0x2e
#define PS2KBD_v                    0x2f
#define PS2KBD_b                    0x30
#define PS2KBD_n                    0x31
#define PS2KBD_m                    0x32
#define PS2KBD_COMMA                0x33   // ,<
#define PS2KBD_DOT                  0x34   // .>
#define PS2KBD_SLASH                0x35   // /?

#define PS2KBD_RIGHTSHIFT           0x36

#define PS2KBD_KEYPADASTERISK       0x37  

#define PS2KBD_LEFTALT              0x38

#define PS2KBD_SPACE                0x39

#define PS2KBD_CAPSLOCK             0x3a

#define PS2KBD_F1                   0x3b
#define PS2KBD_F2                   0x3c
#define PS2KBD_F3                   0x3d
#define PS2KBD_F4                   0x3e
#define PS2KBD_F5                   0x3f
#define PS2KBD_F6                   0x40
#define PS2KBD_F7                   0x41
#define PS2KBD_F8                   0x42
#define PS2KBD_F9                   0x43
#define PS2KBD_F10                  0x44
#define PS2KBD_F11                  0x57
#define PS2KBD_F12                  0x58

#define PS2KBD_NUMLOCK              0x45
#define PS2KBD_SCROLLLOCK           0x46

#define PS2KBD_KEYPAD7              0x47
#define PS2KBD_KEYPAD8              0x48
#define PS2KBD_KEYPAD9              0x49
#define PS2KBD_KEYPADMINUS2         0x4a
#define PS2KBD_KEYPAD4              0x4b
#define PS2KBD_KEYPAD5              0x4c
#define PS2KBD_KEYPAD6              0x4d
#define PS2KBD_KEYPADPLUS           0x4e
#define PS2KBD_KEYPADEND            0x4f
#define PS2KBD_KEYPADDOWN           0x50
#define PS2KBD_KEYPADPGDN           0x51
#define PS2KBD_KEYPADINS            0x52
#define PS2KBD_KEYPADDEL            0x53

#define PS2KBD_SYSRQ                0x54

/* dual-byte sequences. */

#define PS2KBD_KEYPADENTER          0x1ce0
#define PS2KBD_RIGHTCTRL            0x1de0
#define PS2KBD_FAKELEFTSHIFT        0x2ae0
#define PS2KBD_KEYPADMINUS3         0x35e0
#define PS2KBD_FAKERIGHTSHIFT       0x36e0
#define PS2KBD_CTRLPRINTSCREEN      0x37e0
#define PS2KBD_RIGHTALT             0x38e0
#define PS2KBD_CTRLBREAK            0x46e0
#define PS2KBD_HOME                 0x47e0
#define PS2KBD_UP                   0x48e0
#define PS2KBD_PGUP                 0x49e0
#define PS2KBD_LEFT                 0x4be0
#define PS2KBD_RIGHT                0x4de0
#define PS2KBD_END                  0x4fe0
#define PS2KBD_DOWN                 0x50e0
#define PS2KBD_PGDN                 0x51e0
#define PS2KBD_INS                  0x52e0
#define PS2KBD_DEL                  0x53e0

#define PS2KBD_LEFTWIN              0x5be0
#define PS2KBD_RIGHTWIN             0x5ce0
#define PS2KBD_MENU                 0x5de0

/* acpi codes. */
#define PS2KBD_POWER                0x37e0
#define PS2KBD_SLEEP                0x3fe0
#define PS2KBD_WAKE                 0x5ee0
#define PS2KBD_POWERUP              0x37f0e0
#define PS2KBD_SLEEPUP              0x3ff0e0
#define PS2KBD_WAKEUP               0x5ef0e0

/* windows multimedia codes. */

/* windows multimedia codes. */

#endif /* KBDUS */

/* symbols. */

/* printable symbols */

#define PS2KBD_ESC_SYM             '\033'
#define PS2KBD_1_SYM               '1'
#define PS2KBD_2_SYM               '2'
#define PS2KBD_3_SYM               '3'
#define PS2KBD_4_SYM               '4'
#define PS2KBD_5_SYM               '5'
#define PS2KBD_6_SYM               '6'
#define PS2KBD_7_SYM               '7'
#define PS2KBD_8_SYM               '8'
#define PS2KBD_9_SYM               '9'
#define PS2KBD_0_SYM               '0'
#define PS2KBD_MINUS_SYM           '-'
#define PS2KBD_PLUS_SYM            '+'

#define PS2KBD_TAB_SYM             '\t'
#define PS2KBD_q_SYM               'q'
#define PS2KBD_w_SYM               'w'
#define PS2KBD_e_SYM               'e'
#define PS2KBD_r_SYM               'r'
#define PS2KBD_t_SYM               't'
#define PS2KBD_y_SYM               'y'
#define PS2KBD_u_SYM               'u'
#define PS2KBD_i_SYM               'i'
#define PS2KBD_o_SYM               'o'
#define PS2KBD_p_SYM               'p'
#define PS2KBD_OPENBRACKET_SYM     '['
#define PS2KBD_CLOSEBRACKET_SYM    ']'

#define PS2KBD_ENTER_SYM           '\n'

#define PS2KBD_a_SYM               'a'
#define PS2KBD_s_SYM               's'
#define PS2KBD_d_SYM               'd'
#define PS2KBD_f_SYM               'f'
#define PS2KBD_g_SYM               'g'
#define PS2KBD_h_SYM               'h'
#define PS2KBD_i_SYM               'i'
#define PS2KBD_j_SYM               'j'
#define PS2KBD_k_SYM               'k'
#define PS2KBD_l_SYM               'l'
#define PS2KBD_SEMICOLON_SYM       ';'
#define PS2KBD_QUOTE_SYM           '\''

#define PS2KBD_BACKQUOTE_SYM       '`'

#define PS2KBD_BACKSLASH_SYM       '\\'

#define PS2KBD_z_SYM               'z'
#define PS2KBD_x_SYM               'x'
#define PS2KBD_c_SYM               'c'
#define PS2KBD_v_SYM               'v'
#define PS2KBD_b_SYM               'b'
#define PS2KBD_n_SYM               'n'
#define PS2KBD_m_SYM               'm'
#define PS2KBD_COMMA_SYM           ','
#define PS2KBD_DOT_SYM             '.'
#define PS2KBD_SLASH_SYM           '/'

#define PS2KBD_KEYPADASTERISK_SYM  '*'

#define PS2KBD_SPACE_SYM           ' '

#define PS2KBD_KEYPAD7_SYM         '7'
#define PS2KBD_KEYPAD8_SYM         '8'
#define PS2KBD_KEYPAD9_SYM         '9'

#define PS2KBD_KEYPADMINUS2_SYM    '-'

#define PS2KBD_KEYPAD4_SYM         '4'
#define PS2KBD_KEYPAD5_SYM         '5'
#define PS2KBD_KEYPAD6_SYM         '6'

#define PS2KBD_KEYPADPLUS_SYM      '+'

/* -0x01..-0x1f are reserved for modifier keys */

#define PS2KBD_LEFTALT_SYM         (-0x01)
#define PS2KBD_RIGHTALT_SYM        (-0x02)
#define PS2KBD_LEFTCTRL_SYM        (-0x03)
#define PS2KBD_RIGHTCTRL_SYM       (-0x04)
#define PS2KBD_LEFTSHIFT_SYM       (-0x05)
#define PS2KBD_RIGHTSHIFT_SYM      (-0x06)
#define PS2KBD_CAPSLOCK_SYM        (-0x07)
#define PS2KBD_NUMLOCK_SYM         (-0x08)
#define PS2KBD_SCROLLLOCK_SYM      (-0x09)
#define PS2KBD_FAKELEFTSHIFT_SYM   (-0x0a)
#define PS2KBD_FAKERIGHTSHIFT_SYM  (-0x0b)
#define PS2KBD_BACKSPACE_SYM       (-0x0c)

/* -0x21..-0x2f are for function keys */
#define PS2KBD_F1_SYM              (-0x21)
#define PS2KBD_F2_SYM              (-0x22)
#define PS2KBD_F3_SYM              (-0x23)
#define PS2KBD_F4_SYM              (-0x24)
#define PS2KBD_F5_SYM              (-0x25)
#define PS2KBD_F6_SYM              (-0x26)
#define PS2KBD_F7_SYM              (-0x27)
#define PS2KBD_F8_SYM              (-0x28)
#define PS2KBD_F9_SYM              (-0x29)
#define PS2KBD_F10_SYM             (-0x30)
#define PS2KBD_F11_SYM             (-0x31)
#define PS2KBD_F12_SYM             (-0x32)

/* -0x30..-0x4f are for keypad and special keys */
#define PS2KBD_KEYPADEND_SYM       (-0x30)
#define PS2KBD_KEYPADDOWN_SYM      (-0x31)
#define PS2KBD_KEYPADPGDN_SYM      (-0x32)
#define PS2KBD_KEYPADINS_SYM       (-0x33)
#define PS2KBD_KEYPADDEL_SYM       (-0x34)
#define PS2KBD_SYSRQ_SYM           (-0x35)
#define PS2KBD_KEYPADENTER_SYM     (-0x36)
#define PS2KBD_KEYPADMINUS3_SYM    (-0x37)
#define PS2KBD_CTRLPRINTSCREEN_SYM (-0x38)
#define PS2KBD_CTRLBREAK_SYM       (-0x39)
#define PS2KBD_HOME_SYM            (-0x3a)
#define PS2KBD_UP_SYM              (-0x3b)
#define PS2KBD_PGUP_SYM            (-0x3c)
#define PS2KBD_LEFT_SYM            (-0x3d)
#define PS2KBD_RIGHT_SYM           (-0x3e)
#define PS2KBD_END_SYM             (-0x3f)
#define PS2KBD_DOWN_SYM            (-0x40)
#define PS2KBD_PGDN_SYM            (-0x41)
#define PS2KBD_INS_SYM             (-0x42)
#define PS2KBD_DEL_SYM             (-0x43)

/* -0x50..-0xXX are extra keys */
#define PS2KBD_POWER_SYM           (-0x50)
#define PS2KBD_SLEEP_SYM           (-0x51)
#define PS2KBD_WAKE_SYM            (-0x52)
#define PS2KBD_POWERUP_SYM         (-0x53)
#define PS2KBD_SLEEPUP_SYM         (-0x54)
#define PS2KBD_WAKEUP_SYM          (-0x55)

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
struct mousestate {
    uint16_t flags;
    uint16_t state;
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t xmax;
    uint32_t ymax;
    uint32_t zmax;
    int32_t shift;
} PACK();

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
#if 0
/* modifier keys. */
    int32_t           keytabmod[PS2KBD_NTAB];
#endif
/* single-code values. */
    int32_t           keytab1b[PS2KBD_NTAB] ALIGNED(PAGESIZE);
/* 0xe0-prefixed values. */
    int32_t           keytabmb[PS2KBD_NTAB];
/* release values. */
    int32_t           keytabup[PS2KBD_NTAB];
    struct mousestate mousestate;
    int32_t           modmask;
} ALIGNED(PAGESIZE);

#endif /* __KERN_IO_DRV_PC_PS2_H__ */

