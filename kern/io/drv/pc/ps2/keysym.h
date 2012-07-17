#ifndef __KERN_IO_DRV_PC_PS2_KEYSYM_H__
#define __KERN_IO_DRV_PC_PS2_KEYSYM_H__

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

#endif /* __KERN_IO_DRV_PC_PS2_KEYSYM_H__ */

