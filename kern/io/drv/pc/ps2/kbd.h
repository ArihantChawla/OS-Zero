#ifndef __KERN_IO_DRV_PC_PS2_KBD_H__
#define __KERN_IO_DRV_PC_PS2_KBD_H__

#include "kbd_us.h"

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

#endif /* __KERN_IO_DRV_PC_PS2_KBD_H__ */

