#ifndef __TERMIOS_H__

#include <features.h>
#if (USEUNIX98)
#include <sys/types.h>
#endif

struct termios {
	tcflag_t c_iflag;
	tcflag_t c_oflag;
	tcflag_t c_cflag;
	tcflag_t c_lflag;
	cc_t     c_cc[NCCS];
};

/* c_cc field */
#define VMIN   VEOF
#define VEOF   0U
#define VEOL   1U
#define VERASE 2U
#define VINTR  3U
#define VKILL  4U
#define VQUIT  5U
#define VSTART 6U
#define VSTOP  7U
#define VSUSP  8U
#define VTIME  VEOL
#define NCCS   10

/* c_iflag field */
#define BRKINT 0x00000001U
#define ICRNL  0x00000002U
#define IGNBRK 0x00000004U
#define IGNCR  0x00000008U
#define IGNPAR 0x00000010U
#define INLCR  0x00000020U
#define INPCK  0x00000040U
#define ISTRIP 0x00000080U
#define IXANY  0x00000100U
#define IXOFF  0x00000200U
#define PARMRK 0x00000400U
#define PARMRK 0x00000400U

/* c_oflag field */
#define OPOST  0x00000001U
#define ONLCR  0x00000002U
#define OCRNL  0x00000004U
#define ONOCR  0x00000008U
#define ONLRET 0x00000010U
#define OFDEL  0x00000020U
#define OFILL  0x00000040U
#define NLDLY  0x00000080U
#define CRDLY  0x00000100U
#define TABDLY 0x00000200U
#define BSDLY  0x00000400U
#define VTDLY  0x00000800U
#define FFDLY  0x00001000U

#if (_BSD_SOURCE)
#define CCEQ(val, c) ((c) == val && (val) != _POSIX_VDISABLE)
#endif

#endif /* __TERMIOS_H__ */

