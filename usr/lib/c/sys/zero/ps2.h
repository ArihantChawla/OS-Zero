#ifndef __SYS_ZERO_PS2_H__
#define __SYS_ZERO_PS2_H__

/* REFERENCE: http://www.vetra.com/scancodes.html */

#include <stdint.h>
#include <sys/zero/conf.h>

/* KEYBOARD */

//#define PS2KBD_NBUFKEY              (PAGESIZE >> 3)

#define PS2KBD_NOCODE               0x0000000
#define PS2KBD_NOSYM                0x0000000

#if 0
#define PS2KBD_US                   0x00
#endif

#define KBDMAXVAL                   0xff
#define KBDNVAL                     256

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
#define PS2KBD_KEYPADDELETE_SYM    (-0x34)
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
#define PS2KBD_DELETE_SYM          (-0x43)

/* -0x50..-0xXX are extra keys */
#define PS2KBD_POWER_SYM           (-0x50)
#define PS2KBD_SLEEP_SYM           (-0x51)
#define PS2KBD_WAKE_SYM            (-0x52)
#define PS2KBD_POWERUP_SYM         (-0x53)
#define PS2KBD_SLEEPUP_SYM         (-0x54)
#define PS2KBD_WAKEUP_SYM          (-0x55)

struct ps2keymap {
#if 0
/* modifier keys. */
    int32_t mod[KBDNVAL];
#endif
/* single-code values. */
    int32_t tab1b[KBDNVAL];
/* 0xe0-prefixed values. */
    int32_t tabmb[KBDNVAL];
/* release values. */
    int32_t tabup[KBDNVAL];
};

#endif /* __SYS_ZERO_PS2_H__ */

