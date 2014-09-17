#include <stdint.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#define NOP(x)        ((void)0)
#define NOP3(x, y, z) ((void)0)
#define MALLOC(x)     ((void *)0)
#define FREE          NOP
#define MEMCPY        NOP3
#define RING_ITEM     uint64_t
#define RING_INVAL    UINT64_C(0)
#include <zero/ring.h>
#include <sys/zero/ps2.h>

#include <kern/syscall.h>
#include <kern/syscallnum.h>

extern ASMLINK long _syscall(long num, long arg1, long arg2, long arg3);

#if 0
#define ismodkey(val)                                                   \
    (((val) & 0x80000000) && ((val) & 0xfffffff0) == 0xfffffff0)
#endif
#define kbdsetcode(name)                                                \
    (((((name) >> 8) & 0xff) == PS2KBD_UP_BYTE)                         \
     ? (ps2keymap.tabup[(name >> 16) & 0xff] = name##_SYM | PS2KBD_UP_BIT) \
     : ((((name) & 0xff) == PS2KBD_PREFIX_BYTE)                         \
        ? (ps2keymap.tabmb[(name >> 8) & 0xff] = name##_SYM,            \
           ps2keymap.tabup[(name >> 8) & 0xff] = name##_SYM | PS2KBD_UP_BIT) \
        : (ps2keymap.tab1b[name & 0xff] = name##_SYM,                   \
           ps2keymap.tabup[name & 0xff] = name##_SYM | PS2KBD_UP_BIT)))

static struct ps2keymap ps2keymap ALIGNED(PAGESIZE);
static struct ringbuf   kbdbuf ALIGNED(PAGESIZE);

void
kbdinitmap_us(void)
{
    /* modifiers. */
    kbdsetcode(PS2KBD_LEFTCTRL);
    kbdsetcode(PS2KBD_LEFTSHIFT);
    kbdsetcode(PS2KBD_RIGHTSHIFT);
    kbdsetcode(PS2KBD_LEFTALT);
    kbdsetcode(PS2KBD_RIGHTALT);
    kbdsetcode(PS2KBD_CAPSLOCK);
    kbdsetcode(PS2KBD_NUMLOCK);
    kbdsetcode(PS2KBD_SCROLLLOCK);

    /* single-byte keys. */

    kbdsetcode(PS2KBD_ESC);
    kbdsetcode(PS2KBD_1);
    kbdsetcode(PS2KBD_2);
    kbdsetcode(PS2KBD_3);
    kbdsetcode(PS2KBD_4);
    kbdsetcode(PS2KBD_5);
    kbdsetcode(PS2KBD_6);
    kbdsetcode(PS2KBD_7);
    kbdsetcode(PS2KBD_8);
    kbdsetcode(PS2KBD_9);
    kbdsetcode(PS2KBD_0);
    kbdsetcode(PS2KBD_MINUS);
    kbdsetcode(PS2KBD_PLUS);
    kbdsetcode(PS2KBD_BACKSPACE);

    kbdsetcode(PS2KBD_TAB);
    kbdsetcode(PS2KBD_q);
    kbdsetcode(PS2KBD_w);
    kbdsetcode(PS2KBD_e);
    kbdsetcode(PS2KBD_r);
    kbdsetcode(PS2KBD_t);
    kbdsetcode(PS2KBD_y);
    kbdsetcode(PS2KBD_u);
    kbdsetcode(PS2KBD_i);
    kbdsetcode(PS2KBD_o);
    kbdsetcode(PS2KBD_p);
    kbdsetcode(PS2KBD_OPENBRACKET);
    kbdsetcode(PS2KBD_CLOSEBRACKET);

    kbdsetcode(PS2KBD_ENTER);

    kbdsetcode(PS2KBD_LEFTCTRL);

    kbdsetcode(PS2KBD_a);
    kbdsetcode(PS2KBD_s);
    kbdsetcode(PS2KBD_d);
    kbdsetcode(PS2KBD_f);
    kbdsetcode(PS2KBD_g);
    kbdsetcode(PS2KBD_h);
    kbdsetcode(PS2KBD_i);
    kbdsetcode(PS2KBD_j);
    kbdsetcode(PS2KBD_k);
    kbdsetcode(PS2KBD_l);
    kbdsetcode(PS2KBD_SEMICOLON);
    kbdsetcode(PS2KBD_QUOTE);

    kbdsetcode(PS2KBD_BACKQUOTE);

    kbdsetcode(PS2KBD_LEFTSHIFT);

    kbdsetcode(PS2KBD_BACKSLASH);

    kbdsetcode(PS2KBD_z);
    kbdsetcode(PS2KBD_x);
    kbdsetcode(PS2KBD_c);
    kbdsetcode(PS2KBD_v);
    kbdsetcode(PS2KBD_b);
    kbdsetcode(PS2KBD_n);
    kbdsetcode(PS2KBD_m);
    kbdsetcode(PS2KBD_COMMA);
    kbdsetcode(PS2KBD_DOT);
    kbdsetcode(PS2KBD_SLASH);

    kbdsetcode(PS2KBD_RIGHTSHIFT);

    kbdsetcode(PS2KBD_KEYPADASTERISK);

    kbdsetcode(PS2KBD_SPACE);

    kbdsetcode(PS2KBD_CAPSLOCK);

    kbdsetcode(PS2KBD_F1);
    kbdsetcode(PS2KBD_F2);
    kbdsetcode(PS2KBD_F3);
    kbdsetcode(PS2KBD_F4);
    kbdsetcode(PS2KBD_F5);
    kbdsetcode(PS2KBD_F6);
    kbdsetcode(PS2KBD_F7);
    kbdsetcode(PS2KBD_F8);
    kbdsetcode(PS2KBD_F9);
    kbdsetcode(PS2KBD_F10);

    kbdsetcode(PS2KBD_NUMLOCK);
    kbdsetcode(PS2KBD_SCROLLLOCK);

    kbdsetcode(PS2KBD_F11);
    kbdsetcode(PS2KBD_F12);

    kbdsetcode(PS2KBD_KEYPAD7);
    kbdsetcode(PS2KBD_KEYPAD8);
    kbdsetcode(PS2KBD_KEYPAD9);

    kbdsetcode(PS2KBD_KEYPADMINUS2);

    kbdsetcode(PS2KBD_KEYPAD4);
    kbdsetcode(PS2KBD_KEYPAD5);
    kbdsetcode(PS2KBD_KEYPAD6);

    kbdsetcode(PS2KBD_KEYPADPLUS);

    kbdsetcode(PS2KBD_KEYPADEND);
    kbdsetcode(PS2KBD_KEYPADDOWN);
    kbdsetcode(PS2KBD_KEYPADPGDN);

    kbdsetcode(PS2KBD_KEYPADINS);
    kbdsetcode(PS2KBD_KEYPADDEL);

    kbdsetcode(PS2KBD_SYSRQ);

    /* dual-byte sequences. */

    kbdsetcode(PS2KBD_KEYPADENTER);
    kbdsetcode(PS2KBD_RIGHTCTRL);
    kbdsetcode(PS2KBD_FAKELEFTSHIFT);
    kbdsetcode(PS2KBD_KEYPADMINUS3);
    kbdsetcode(PS2KBD_FAKERIGHTSHIFT);
    kbdsetcode(PS2KBD_CTRLPRINTSCREEN);
    kbdsetcode(PS2KBD_RIGHTALT);
    kbdsetcode(PS2KBD_CTRLBREAK);
    kbdsetcode(PS2KBD_HOME);
    kbdsetcode(PS2KBD_UP);
    kbdsetcode(PS2KBD_PGUP);
    kbdsetcode(PS2KBD_LEFT);
    kbdsetcode(PS2KBD_RIGHT);
    kbdsetcode(PS2KBD_END);
    kbdsetcode(PS2KBD_DOWN);
    kbdsetcode(PS2KBD_PGDN);
    kbdsetcode(PS2KBD_INS);
    kbdsetcode(PS2KBD_DEL);

    /* acpi codes. */
    kbdsetcode(PS2KBD_POWER);
    kbdsetcode(PS2KBD_SLEEP);
    kbdsetcode(PS2KBD_WAKE);
    kbdsetcode(PS2KBD_POWERUP);
    kbdsetcode(PS2KBD_SLEEPUP);
    kbdsetcode(PS2KBD_WAKEUP);

    return;
}

void
kbdinit(void)
{
    /* TODO: initialise ring buffer */
    /* pctl(PROC_MAPBUF, PROC_KBDBUF, &kbdbuf); set proc->kbdbuf */
    _syscall(SYS_PCTL, PROC_MAPBUF, PROC_KBDBUF, (long)&kbdbuf);
    /* initialise US keymap */
    kbdinitmap_us();
}

