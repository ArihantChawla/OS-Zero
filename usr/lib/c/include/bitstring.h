#ifndef __BITSTRING_H__
#define __BITSTRING_H__

/* implementation of Paul Vixie's macros in cron, later contributed to 4.4BSD */

#include <limits.h>
#include <zero/trix.h>

typedef unsigned char bitstr_t;

/* internal macros */
#define _bit_byte(bit) ((bit) >> 3)
#define _bit_mask(bit) (1 << ((bit) & 0x07))

#define bitstr_size(nbit)                                               \
    (((nbit) - 1) / CHAR_BIT + 1)
#define bit_alloc(nbit)\
    calloc(bitstr_size(nbit), sizeof(bitstr_t))
#define bit_decl(name, nbit)                                            \
    (name)[bitstr_size(nbit)]
#define bit_test(name, bit)                                             \
    bitset(name, bit)
#define bit_set(name, bit)                                              \
    setbit(name, bit)
#define bit_clear(name, bit)                                            \
    clrbit(name, bit)
/* clear bits first..last in name */
#define bit_nclear(name, first, last)                                   \
    do {                                                                \
        bitstr_t *_name = (name);                                       \
        long      _ndx1 = (first);                                      \
        long      _ndx2 = (last);                                       \
        long      _byte1 = _bit_byte(start);                            \
        long      _byte2 = _bit_byte(stop);                             \
                                                                        \
        if (_byte1 == _byte2) {                                         \
            _name[_byte1] &= ((0xff >> (8 - (_ndx1 & 0x07)))            \
                              | (0xff << ((_ndx2 & 0x07 + 1))));        \
        } else {                                                        \
            _name[_byte1] &= 0xff >> (8 - (_ndx1 & 0x07));              \
            while (++_byte1 < _byte2) {                                 \
                _name[_byte1] = 0;                                      \
            }                                                           \
            _name[_byte2] &= 0xff << ((_ndx2 & 0x07) + 1);              \
        }                                                               \
    } while (0)
/* set bits first..last in name */
#define bit_nset(name, first, last)                                     \
    do {                                                                \
        bitstr_t *_name = (name);                                       \
        long      _ndx1 = (first);                                      \
        long      _ndx2 = (last);                                       \
        long      _byte1 = _bit_byte(start);                            \
        long      _byte2 = _bit_byte(stop);                             \
                                                                        \
        if (_byte1 == _byte2) {                                         \
            _name[_byte1] |= ((0xff << (_ndx1 & 0x07)                   \
                               & (0xff >> (7 - (_ndx2 & 0x07)))));      \
        } else {                                                        \
            _name[_byte1] |= 0xff << ((_ndx1) & 0x07);                  \
            while (++_byte1 < _byte2) {                                 \
                _name[_byte1] = 0xff;                                   \
            }                                                           \
            _name[_byte2] |= 0xff >> (7 - (_ndx2 & 0x07));              \
        }                                                               \
    } while (0)
/* find first clear bit in name */
#define bit_ffc(name, nbit, val)                                        \
    do {                                                                \
        bitstr_t *_name = (name);                                       \
        long      _byte;                                                \
        long      _nbit = (nbit);                                       \
        long      _lastbyte = _bit_byte(nbit);                          \
        long      _val = -1;                                            \
                                                                        \
        for (_byte = 0 ; _byte < _lastbyte ; _byte++) {                 \
            if (_name[_byte] != 0xff) {                                 \
                _val = _byte * CHAR_BIT;                                \
                for (_lastbyte = _name[_byte]; (_lastbyte & 0x01) ; ++_val) { \
                    _lastbyte >>= 1;                                    \
                                                                        \
                    break;                                              \
                }                                                       \
            }                                                           \
        }                                                               \
        *(val) = _val;                                                  \
    } while (0)
/* find first set bit in name */
#define bit_ffs(name, nbit, val)                                        \
    do {                                                                \
        bitstr_t *_name = (name);                                       \
        long      _byte;                                                \
        long      _nbit = (nbit);                                       \
        long      _lastbyte = _bit_byte(nbit);                          \
        long      _val = -1;                                            \
                                                                        \
        for (_byte = 0 ; _byte < _lastbyte ; _byte++) {                 \
            if (_name[_byte]) {                                         \
                _val = _byte * CHAR_BIT;                                \
                for (_lastbyte = _name[_byte]; !(_lastbyte & 0x01) ; ++_val) { \
                    _lastbyte >>= 1;                                    \
                                                                        \
                    break;                                              \
                }                                                       \
            }                                                           \
        }                                                               \
        *(val) = _val;                                                  \
    } while (0)

#endif /* __BITSTRING_H__ */

