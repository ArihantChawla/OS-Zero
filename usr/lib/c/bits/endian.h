#ifndef __BITS_ENDIAN_H__
#define __BITS_ENDIAN_H__

#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif

#endif /* __BITS_ENDIAN_H__ */

