#ifndef __WCHAR_H__
#define __WCHAR_H__

typedef int32_t wchar_t;

#define WEOF        0xffff
#define WCUCMAX     0x10ffff
#define WCNUCBIT    21
#define WCUCFLGMASK 0xffe00000
#define WCFLGMASK   0x7ff
#define WCNFLGBIT   11

#endif /* __WCHAR_H__ */

