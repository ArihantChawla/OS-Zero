#ifndef __KERN_IO_DRV_PC_PS2_KBD_US_H__
#define __KERN_IO_DRV_PC_PS2_KBD_US_H__

/* scancodes. */

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

#endif /* __KERN_IO_DRV_PC_PS2_KBD_US_H__ */

