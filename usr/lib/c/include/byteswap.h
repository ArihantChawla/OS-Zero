#ifndef __BYTESWAP_H__
#define __BYTESWAP_H__

/*
 * NOTE: on the ia-32 architecture (P3), optimizing these with assembly didn't
 * seem worth it...
 */
#define bswap_16(w)   _bswap_16(w)
#define bswap_32(l)   _bswap_32(l)
#define bswap_64(ll)  _bswap_64(ll)

#define _bswap_16(w)  ((((w) >> 8) & 0xff) | (((w) & 0xff) << 8))
#define _bswap_32(l)  ((((l) >> 24) & 0xff)                             \
                       | (((l) >> 8) & 0xff00)                          \
                       | (((l) & 0xff00) << 8)                          \
                       | (((l) & 0xff) << 24))
#define _bswap_64(ll) ((((ll) >> 56) & 0xff)   \
                       | (((ll) >> 40) & 0xff00)                        \
                       | (((ll) >> 24) & 0x00ff0000)                    \
                       | (((ll) >> 8) & 0xff000000)                     \
                       | (((ll) & 0xff000000) << 8)                     \
                       | (((ll) & 0x00ff0000) << 24)                    \
                       | (((ll) & 0xff00) << 40)   \
                       | (((ll) & 0xff) << 56))

#endif /* __BYTESWAP_H__ */

