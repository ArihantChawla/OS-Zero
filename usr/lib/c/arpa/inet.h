#ifndef __ARPA_INET_H__
#define __ARPA_INET_H__

#include <endian.h>
#include <byteswap.h>

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define htons(s) _bswap_16(s)
#define htonl(s) _bswap_32(l)
#define ntohs(s) _bswap_16(s)
#define ntohl(s) _bswap_32(l)
#else
#define htons(s) (s)
#define htonl(l) (l)
#define ntohs(s) (s)
#define ntohl(l) (l)
#endif

#endif /* __ARPA_INET_H__ */

