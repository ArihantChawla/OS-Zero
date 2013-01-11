#ifndef __BYTESWAP_H__
#define __BYTESWAP_H__

/*
 * NOTE: on the ia-32 architecture (P3), optimizing these with assembly didn't
 * seem worth it...
 */
#define bswap_16(x)  _bswap_16(x)
#define bswap_32(x)  _bswap_32(x)
#define bswap_64(x)  _bswap_64(x)

#define _bswap_16(x) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
#define _bswap_32(x) ((((x) >> 24) & 0xff) \
                     | (((x) >> 8) & 0xff00) \
                     | (((x) & 0xff00) << 8) \
                     | (((x) & 0xff) << 24))
#define _bswap_64(x) ((((x) >> 56) & 0xff) \
                     | (((x) >> 40) & 0xff00) \
                     | (((x) >> 24) & 0x00ff0000) \
                     | (((x) >> 8) & 0xff000000) \
                     | (((x) & 0xff000000) << 8) \
                     | (((x) & 0x00ff0000) << 24) \
                     | (((x) & 0xff00) << 40) \
                     | (((x) & 0xff) << 56))

#endif /* __BYTESWAP_H__ */

