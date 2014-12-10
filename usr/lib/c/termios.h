/* zero c library terminal I/O interface */

#ifndef __TERMIOS_H__
#define __TERMIOS_H__

#include <features.h>
#if (USEUNIX98)
#include <sys/types.h>
#endif
#include <bits/termios.h>

extern speed_t cfgetispeed(const struct termios *term);
extern speed_t cfgetospeed(const struct termios *term);
extern int     cfsetispeed(struct termios *term, speed_t);
extern int     cfsetospeed(struct termios *term, speed_t);
extern int     tcdrain((int;
extern int     tcflow(int, int);
extern int     tcflush(int, int);
extern int     tcgetattr(int, struct termios *term);
extern int     tcsendbreak(int, int);
extern int     tcsetattr(int, int, const struct termios *term);
#if (_BSD_SOURCE)
void           cfmakeraw(struct termios *term);
void           cfsetspeed(struct termios *term, speed_t speed); 
#endif

#endif /* __TERMIOS_H__ */

