#ifndef __KERN_IO_DRV_PC_PS2_H__
#define __KERN_IO_DRV_PC_PS2_H__

/* KEYBOARD */

#define KBD_NTAB            128

#define KBD_US              0x00

#define KBD_PORT            0x60

/* keyboard commands */
#define KBD_ENABLE          0xf4
#define KBD_SETSCAN         0xf0

/* protocol scancodes */
#define KBD_ERROR           0x00
#define KBD_BAT             0xaa
#define KBD_ECHO            0xee
#define KBD_ACK             0xfa
#define KBD_BATORMOUSEERR   0xfc
#define KBD_INTERR          0xfd
#define KBD_ACKFAIL         0xfe
#define KBD_ERROR2          0xff

#define KBD_VAL_MSK         0xff
#define KBD_PREFIX_BYTE     0xe0
#define KBD_UP_BIT          0x80
#define KBD_UP_BYTE         0xf0

#define KBD_PAUSE_BYTE1     0xe1
/* after 0xe0 */
#define KBD_CTRLPAUSE_BYTE2 0x46
#define KBD_PRINT_BYTE2     0x2a

/* invalid. */
#define KBD_INVAL_SYM       0x00

#if 0
#define KBD_LEFTCTRL_FLAG   0x01
#define KBD_LEFTSHIFT_FLAG  0x02
#define KBD_RIGHTSHIFT_FLAG 0x04
#define KBD_LEFTALT_FLAG    0x08
#define KBD_RIGHTALT_FLAG   0x10
#define KBD_CAPSLOCK_FLAG   0x20
#define KBD_NUMLOCK_FLAG    0x40
#define KBD_SCROLLLOCK_FLAG 0x80
#endif

/* US scancodes. */

#if (KBDUS)

/* single-byte values. */

#define KBD_ESC                  0x01
#define KBD_1                    0x02
#define KBD_2                    0x03
#define KBD_3                    0x04
#define KBD_4                    0x05
#define KBD_5                    0x06
#define KBD_6                    0x07
#define KBD_7                    0x08
#define KBD_8                    0x09
#define KBD_9                    0x0a
#define KBD_0                    0x0b
#define KBD_MINUS                0x0c
#define KBD_PLUS                 0x0d
#define KBD_BACKSPACE            0x0e

#define KBD_TAB                  0x0f
#define KBD_q                    0x10
#define KBD_w                    0x11
#define KBD_e                    0x12
#define KBD_r                    0x13
#define KBD_t                    0x14
#define KBD_y                    0x15
#define KBD_u                    0x16
#define KBD_i                    0x17
#define KBD_o                    0x18
#define KBD_p                    0x19
#define KBD_OPENBRACKET          0x1a   // [{
#define KBD_CLOSEBRACKET         0x1b   // ]}

#define KBD_ENTER                0x1c

#define KBD_LEFTCTRL             0x1d

#define KBD_a                    0x1e
#define KBD_s                    0x1f
#define KBD_d                    0x20
#define KBD_f                    0x21
#define KBD_g                    0x22
#define KBD_h                    0x23
#define KBD_j                    0x24
#define KBD_k                    0x25
#define KBD_l                    0x26
#define KBD_SEMICOLON            0x27   // ;:

#define KBD_QUOTE                0x28   // '"

/* modifier scancodes. */

#define KBD_BACKQUOTE            0x29   // `~

#define KBD_LEFTSHIFT            0x2a

#define KBD_BACKSLASH            0x2b   // \|

#define KBD_z                    0x2c
#define KBD_x                    0x2d
#define KBD_c                    0x2e
#define KBD_v                    0x2f
#define KBD_b                    0x30
#define KBD_n                    0x31
#define KBD_m                    0x32
#define KBD_COMMA                0x33   // ,<
#define KBD_DOT                  0x34   // .>
#define KBD_SLASH                0x35   // /?

#define KBD_RIGHTSHIFT           0x36

#define KBD_KEYPADASTERISK       0x37  

#define KBD_LEFTALT              0x38

#define KBD_SPACE                0x39

#define KBD_CAPSLOCK             0x3a

#define KBD_F1                   0x3b
#define KBD_F2                   0x3c
#define KBD_F3                   0x3d
#define KBD_F4                   0x3e
#define KBD_F5                   0x3f
#define KBD_F6                   0x40
#define KBD_F7                   0x41
#define KBD_F8                   0x42
#define KBD_F9                   0x43
#define KBD_F10                  0x44
#define KBD_F11                  0x57
#define KBD_F12                  0x58

#define KBD_NUMLOCK              0x45
#define KBD_SCROLLLOCK           0x46

#define KBD_KEYPAD7              0x47
#define KBD_KEYPAD8              0x48
#define KBD_KEYPAD9              0x49
#define KBD_KEYPADMINUS2         0x4a
#define KBD_KEYPAD4              0x4b
#define KBD_KEYPAD5              0x4c
#define KBD_KEYPAD6              0x4d
#define KBD_KEYPADPLUS           0x4e
#define KBD_KEYPADEND            0x4f
#define KBD_KEYPADDOWN           0x50
#define KBD_KEYPADPGDN           0x51
#define KBD_KEYPADINS            0x52
#define KBD_KEYPADDEL            0x53

#define KBD_SYSRQ                0x54

/* dual-byte sequences. */

#define KBD_KEYPADENTER          0x1ce0
#define KBD_RIGHTCTRL            0x1de0
#define KBD_FAKELEFTSHIFT        0x2ae0
#define KBD_KEYPADMINUS3         0x35e0
#define KBD_FAKERIGHTSHIFT       0x36e0
#define KBD_CTRLPRINTSCREEN      0x37e0
#define KBD_RIGHTALT             0x38e0
#define KBD_CTRLBREAK            0x46e0
#define KBD_HOME                 0x47e0
#define KBD_UP                   0x48e0
#define KBD_PGUP                 0x49e0
#define KBD_LEFT                 0x4be0
#define KBD_RIGHT                0x4de0
#define KBD_END                  0x4fe0
#define KBD_DOWN                 0x50e0
#define KBD_PGDN                 0x51e0
#define KBD_INS                  0x52e0
#define KBD_DEL                  0x53e0

#define KBD_LEFTWIN              0x5be0
#define KBD_RIGHTWIN             0x5ce0
#define KBD_MENU                 0x5de0

/* acpi codes. */
#define KBD_POWER                0x37e0
#define KBD_SLEEP                0x3fe0
#define KBD_WAKE                 0x5ee0
#define KBD_POWERUP              0x37f0e0
#define KBD_SLEEPUP              0x3ff0e0
#define KBD_WAKEUP               0x5ef0e0

/* windows multimedia codes. */

/* windows multimedia codes. */

#endif /* KBDUS */

/* symbols. */

/* printable symbols */

#define KBD_ESC_SYM             '\033'
#define KBD_1_SYM               '1'
#define KBD_2_SYM               '2'
#define KBD_3_SYM               '3'
#define KBD_4_SYM               '4'
#define KBD_5_SYM               '5'
#define KBD_6_SYM               '6'
#define KBD_7_SYM               '7'
#define KBD_8_SYM               '8'
#define KBD_9_SYM               '9'
#define KBD_0_SYM               '0'
#define KBD_MINUS_SYM           '-'
#define KBD_PLUS_SYM            '+'

#define KBD_TAB_SYM             '\t'
#define KBD_q_SYM               'q'
#define KBD_w_SYM               'w'
#define KBD_e_SYM               'e'
#define KBD_r_SYM               'r'
#define KBD_t_SYM               't'
#define KBD_y_SYM               'y'
#define KBD_u_SYM               'u'
#define KBD_i_SYM               'i'
#define KBD_o_SYM               'o'
#define KBD_p_SYM               'p'
#define KBD_OPENBRACKET_SYM     '['
#define KBD_CLOSEBRACKET_SYM    ']'

#define KBD_ENTER_SYM           '\n'

#define KBD_a_SYM               'a'
#define KBD_s_SYM               's'
#define KBD_d_SYM               'd'
#define KBD_f_SYM               'f'
#define KBD_g_SYM               'g'
#define KBD_h_SYM               'h'
#define KBD_i_SYM               'i'
#define KBD_j_SYM               'j'
#define KBD_k_SYM               'k'
#define KBD_l_SYM               'l'
#define KBD_SEMICOLON_SYM       ';'
#define KBD_QUOTE_SYM           '\''

#define KBD_BACKQUOTE_SYM       '`'

#define KBD_BACKSLASH_SYM       '\\'

#define KBD_z_SYM               'z'
#define KBD_x_SYM               'x'
#define KBD_c_SYM               'c'
#define KBD_v_SYM               'v'
#define KBD_b_SYM               'b'
#define KBD_n_SYM               'n'
#define KBD_m_SYM               'm'
#define KBD_COMMA_SYM           ','
#define KBD_DOT_SYM             '.'
#define KBD_SLASH_SYM           '/'

#define KBD_KEYPADASTERISK_SYM  '*'

#define KBD_SPACE_SYM           ' '

#define KBD_KEYPAD7_SYM         '7'
#define KBD_KEYPAD8_SYM         '8'
#define KBD_KEYPAD9_SYM         '9'

#define KBD_KEYPADMINUS2_SYM    '-'

#define KBD_KEYPAD4_SYM         '4'
#define KBD_KEYPAD5_SYM         '5'
#define KBD_KEYPAD6_SYM         '6'

#define KBD_KEYPADPLUS_SYM      '+'

/* -0x01..-0x1f are reserved for modifier keys */

#define KBD_LEFTALT_SYM         (-0x01)
#define KBD_RIGHTALT_SYM        (-0x02)
#define KBD_LEFTCTRL_SYM        (-0x03)
#define KBD_RIGHTCTRL_SYM       (-0x04)
#define KBD_LEFTSHIFT_SYM       (-0x05)
#define KBD_RIGHTSHIFT_SYM      (-0x06)
#define KBD_CAPSLOCK_SYM        (-0x07)
#define KBD_NUMLOCK_SYM         (-0x08)
#define KBD_SCROLLLOCK_SYM      (-0x09)
#define KBD_FAKELEFTSHIFT_SYM   (-0x0a)
#define KBD_FAKERIGHTSHIFT_SYM  (-0x0b)
#define KBD_BACKSPACE_SYM       (-0x0c)

/* -0x21..-0x2f are for function keys */
#define KBD_F1_SYM              (-0x21)
#define KBD_F2_SYM              (-0x22)
#define KBD_F3_SYM              (-0x23)
#define KBD_F4_SYM              (-0x24)
#define KBD_F5_SYM              (-0x25)
#define KBD_F6_SYM              (-0x26)
#define KBD_F7_SYM              (-0x27)
#define KBD_F8_SYM              (-0x28)
#define KBD_F9_SYM              (-0x29)
#define KBD_F10_SYM             (-0x30)
#define KBD_F11_SYM             (-0x31)
#define KBD_F12_SYM             (-0x32)

/* -0x30..-0x4f are for keypad and special keys */
#define KBD_KEYPADEND_SYM       (-0x30)
#define KBD_KEYPADDOWN_SYM      (-0x31)
#define KBD_KEYPADPGDN_SYM      (-0x32)
#define KBD_KEYPADINS_SYM       (-0x33)
#define KBD_KEYPADDEL_SYM       (-0x34)
#define KBD_SYSRQ_SYM           (-0x35)
#define KBD_KEYPADENTER_SYM     (-0x36)
#define KBD_KEYPADMINUS3_SYM    (-0x37)
#define KBD_CTRLPRINTSCREEN_SYM (-0x38)
#define KBD_CTRLBREAK_SYM       (-0x39)
#define KBD_HOME_SYM            (-0x3a)
#define KBD_UP_SYM              (-0x3b)
#define KBD_PGUP_SYM            (-0x3c)
#define KBD_LEFT_SYM            (-0x3d)
#define KBD_RIGHT_SYM           (-0x3e)
#define KBD_END_SYM             (-0x3f)
#define KBD_DOWN_SYM            (-0x40)
#define KBD_PGDN_SYM            (-0x41)
#define KBD_INS_SYM             (-0x42)
#define KBD_DEL_SYM             (-0x43)

/* -0x50..-0xXX are extra keys */
#define KBD_POWER_SYM           (-0x50)
#define KBD_SLEEP_SYM           (-0x51)
#define KBD_WAKE_SYM            (-0x52)
#define KBD_POWERUP_SYM         (-0x53)
#define KBD_SLEEPUP_SYM         (-0x54)
#define KBD_WAKEUP_SYM          (-0x55)

/* MOUSE */

#define MOUSE_INPORT    0x60

/* state. */
#define MOUSE_LEFTBTN   0x01 /* left button flag. */
#define MOUSE_RIGHTBTN  0x02 /* right button flag. */
#define MOUSE_MIDDLEBTN 0x04 /* middle button flag. */
#define MOUSE_3BTNMSK   0x07 /* 3-button mask. */
#define MOUSE_BTN4      0x08 /* button 4 flag. */
#define MOUSE_BTN5      0x10 /* button 5 flag. */
#define MOUSE_XBTNMSK   0x18 /* extra button (4 & 5) mask. */
/* other data. */
#define MOUSE_XSIGN     0x10 /* x-movement sign. */
#define MOUSE_YSIGN     0x20 /* y-movement sign. */
#define MOUSE_XOVERFLOW 0x40 /* x-movement overflow. */
#define MOUSE_YOVERFLOW 0x80 /* y-movement overflow. */
/* extra byte. */
#define MOUSE_ZSIGN     0x08 /* z-movement sign. */
#define MOUSE_ZMSK      0x07 /* z-movement mask. */
#define MOUSE_5BTNMSK   0x30 /* extra button (4 & 5) mask. */
/* flags. */
#define MOUSE_WHEEL      0x01 /* scroll wheel flag. */
#define MOUSE_WHEEL5BTN 0x02 /* 5-button flag. */
#define MOUSE_WHEELMSK  0x03 /* intellimouse mask. */
struct mousestat {
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

#endif /* __KERN_IO_DRV_PC_PS2_H__ */

