#ifndef __BITS_CTYPE_H__
#define __BITS_CTYPE_H__

#if (_ZERO_SOURCE)
#define CTYPE_ASCII      0 /* default C locale */
#define CTYPE_ISO8859_1  1 /* Finnish collation for starters */
#define CTYPE_UCS2       2 /* 16-bit Unicode */
#define CTYPE_UCS4       3 /* 31-bit Unicode */
#define CTYPE_ISO10646   CTYPE_UCS4
#if 0
#define CTYPE_ISO8859_2  2
#define CTYPE_ISO8859_3  3
#define CTYPE_ISO8859_4  4
#define CTYPE_ISO8859_5  5
#define CTYPE_ISO8859_6  6
#define CTYPE_ISO8859_7  7
#define CTYPE_ISO8859_8  8
#define CTYPE_ISO8859_9  9
#define CTYPE_ISO8859_10 10
#define CTYPE_ISO8859_11 11
#define CTYPE_ISO8859_12 12
#define CTYPE_ISO8859_13 13
#define CTYPE_ISO8859_14 14
#define CTYPE_ISO8859_15 15
#define CTYPE_ISO8859_16 16
#endif /* 0 */
#endif

#endif /* __BITS_CTYPE_H__ */

