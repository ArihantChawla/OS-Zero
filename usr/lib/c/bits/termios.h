/* zero c library terminal I/O interface */

#ifndef __BITS_TERMIOS_H__
#define __BITS_TERMIOS_H__

#include <features.h>

#if (_BSD_SOURCE)
#define CCEQ(val, c) ((c) == (val) && (val) != _POSIX_VDISABLE)
#endif

/* special control characters */

#define _POSIX_VDISABLE ((unsigned char)'\377')

/* TODO: VSWTC, VEOL2? */
/* indices for c_cc in struct termios */
#define VEOF      0
#define VEOL      1
#if (_BSD_SOURCE)
#define VEOL      2
#endif
#define VERASE    3
#if (_BSD_SOURCE)
#define VWERASE   4
#endif
#define VKILL     5
#if (_BSD_SOURCE)
#define VREPRINT  6
#endif
#define VINTR     7
#define VQUIT     8
#define VSUSP     9
#if (_BSD_SOURCE)
#define VDSUSP    10
#endif
#define VSTART    11
#define VSTOP     12
#if (_BSD_SOURCE)
#define VLNEXT    13
#define VLDISCARD 14
#endif
#define VMIN      15
#define VTIME     16
#if (_BSD_SOURCE)
#define VSTATUS   17
#endif

#define VSWTC     18
#define VSWTCH    VSWTC

#define NCCS      32    // room for binary-compatible extensions

#if (_BSD_SOURCE)
#define CCEQ(val, c) (c == val ? val != _POSIX_VDISABLE : 0)
#endif

/* software input processing */

/* flags for c_iflag */
#define	IGNBRK  0x00000001    /* ignore BREAK condition */
#define	BRKINT  0x00000002    /* map BREAK to SIGINTR */
#define	IGNPAR  0x00000004    /* ignore (discard) parity errors */
#define	PARMRK  0x00000008    /* mark parity and framing errors */
#define	INPCK   0x00000010    /* enable checking of parity errors */
#define	ISTRIP  0x00000020    /* strip 8th bit off chars */
#define	INLCR   0x00000040    /* map NL into CR */
#define	IGNCR   0x00000080    /* ignore CR */
#define	ICRNL   0x00000100    /* map CR to NL (ala CRMOD) */
#define	IXON    0x00000200    /* enable output flow control */
#define	IXOFF   0x00000400    /* enable input flow control */
#if (_BSD_SOURCE)
#define	IXANY   0x00000800    /* any char will restart after stop */
#define IMAXBEL	0x00002000    /* ring bell on input queue full */
#endif
/* TODO: IUCLC, IUTF8 */

/* software output processing */

/* c_oflag field */
#define OPOST  0x00000001U
#if (_BSD_SOURCE)
#define ONLCR  0x00000002U
#endif
#define OCRNL  0x00000004U
#define ONOCR  0x00000008U
#define ONLRET 0x00000010U
#define OFDEL  0x00000020U
#define OFILL  0x00000040U
#if (_XOPEN_SOURCE)
#define NLDLY  0x00000080U
#define NL0    0x00000000U
#define NL1    0x00000080U
#define CRDLY  0x00000300U
#define CR0    0x00000000U
#define CR1    0x00000100U
#define CR2    0x00000200U
#define TABDLY 0x00000c00U
#define TAB0   0x00000000U
#define TAB1   0x00000400U
#define TAB2   0x00000800U
#define TAB3   0x00000c00U
#define BSDLY  0x00001000U
#define BS0    0x00000000U
#define BS1    0x00001000U
#define FFDLY  0x00002000U
#define FF0    0x00000000U
#define FF1    0x00002000U
#endif
#define VTDLY  0x00004000U
#define VT0    0x00000000U
#define VT1    0x00004000U
#if (_BSD_SOURCE)
#define OXTABS TAB3           /* expand tabs to spaces */
#define ONOEOT 0x00010000U    /* discard EOT's (^D) on output) */
#endif
#define XTABS  TAB3

/* terminal hardware control */

/* c_cflag */
#if (_BSD_SOURCE)
#define	CIGNORE    0x00000001    /* ignore control flags */
#endif
#define CSIZE      0x00000300    /* character size mask */
#define   CS5      0x00000000        /* 5 bits (pseudo) */
#define   CS6      0x00000100        /* 6 bits */
#define   CS7      0x00000200        /* 7 bits */
#define   CS8      0x00000300        /* 8 bits */
#define CSTOPB     0x00000400    /* send 2 stop bits */
#define CREAD      0x00000800    /* enable receiver */
#define PARENB     0x00001000    /* parity enable */
#define PARODD     0x00002000    /* odd parity, else even */
#define HUPCL      0x00004000    /* hang up on last close */
#define CLOCAL     0x00008000    /* ignore modem status lines */
#if (_BSD_SOURCE)
#define CCTS_OFLOW 0x00010000    /* CTS flow control of output */
#define CRTSCTS    CCTS_OFLOW    /* ??? */
#define CRTS_IFLOW 0x00020000    /* RTS flow control of input */
#define MDMBUF     0x00100000    /* flow control output via Carrier */
#endif

/* local flags */

#if (_BSD_SOURCE)
#define	ECHOKE      0x00000001    /* visual erase for line kill */
#endif
#define	ECHOE       0x00000002    /* visually erase chars */
#define	ECHOK       0x00000004    /* echo NL after line kill */
#define ECHO        0x00000008    /* enable echoing */
#define	ECHONL      0x00000010    /* echo NL even if ECHO is off */
#if (_BSD_SOURCE)
#define	ECHOPRT     0x00000020    /* visual erase mode for hardcopy */
#define ECHOCTL     0x00000040    /* echo control chars as ^(Char) */
#endif
#define	ISIG        0x00000080    /* enable signals INTR, QUIT, [D]SUSP */
#define	ICANON      0x00000100    /* canonicalize input lines */
#if (_BSD_SOURCE)
#define ALTWERASE  0x00000200    /* use alternate WERASE algorithm */
#endif
#define	IEXTEN     0x00000400    /* enable DISCARD and LNEXT */
#define EXTPROC    0x00000800  /* external processing */
#define TOSTOP     0x00400000    /* stop background jobs from output */
#if (_BSD_SOURCE)
#define FLUSHO     0x00800000    /* output being flushed (state) */
#define	NOKERNINFO 0x02000000    /* no kernel output from VSTATUS */
#define PENDIN     0x20000000    /* XXX retype pending input (state) */
#endif
#define	NOFLSH     0x80000000    /* don't flush after interrupt */

/* commands passed to tcsetattr() */
#define TCSANOW   0       // make change immediate
#define TCSADRAIN 1       // drain output, then change
#define TCSAFLUSH 2       // drain output, flush input
#if (_BSD_SOURCE)
#define TCSASOFT  0x10    // flag - do not alter hardware state
#endif

/* speed values */
#define B0     0
#define B50    50
#define B75    75
#define B110   110
#define B134   134
#define B150   150
#define B200   200
#define B300   300
#define B600   600
#define B1200  1200
#define B1800  1800
#define B2400  2400
#define B4800  4800
#define B9600  9600
#define B19200 19200
#define B38400 38400
#if (_BSD_SOURCE)
#define EXTA   19200
#define EXTB   38400
#endif

#define TCIFLUSH    1
#define	TCOFLUSH    2
#define TCIOFLUSH    3
#define	TCOOFF        1
#define	TCOON        2
#define TCIOFF        3
#define TCION        4

typedef unsigned long tcflag_t;
typedef unsigned char cc_t;
typedef long          speed_t;

struct termios {
    tcflag_t c_iflag;        // input flags
    tcflag_t c_oflag;        // output flags
    tcflag_t c_cflag;        // control flags
    tcflag_t c_lflag;        // local/other flags
    cc_t     c_cc[NCCS];    // control characters
#if (_BSD_SOURCE)
    long     c_ispeed;      // input speed
    long     c_ospeed;      // output speed
#endif
};

#endif /* __BITS_TERMIOS_H__ */

