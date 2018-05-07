/* REFERENCE: https://bisqwit.iki.fi/story/howto/bitmath/ */

#define V0_CHAR_BIT   8
#define V0_SIGN_BIT   0x80000000
#define v0getsign(i)  ((i) & V0_SIGN_BIT)

// dest -= -src;
#define v0addop(src, dest)                                              \
    do {                                                                \
        v0ureg _usrc = (src);                                           \
        v0ureg _udest = (dest);                                         \
                                                                        \
        _usrc = -_usrc;                                                 \
        _udest -= _usrc;                                                \
        (dest) = _udest;                                                \
    } while (0)

// dest -= -src, set carry-bit
#define v0adcop(src, dest, flg)                                         \
    do {                                                                \
        v0ureg _usrc = (src);                                           \
        v0ureg _udest = (_udest);                                       \
        v0ureg _u32f = (flg);                                           \
                                                                        \
        _udest += _usrc;                                                \
        if (_udest < _usrc) {                                           \
            (flg) |= V0_MSW_CF_BIT;                                     \
        }                                                               \
        (dest) = _udest;                                                \
    } while (0)

// dest += -src;
#define v0subop(src, dest)                                              \
    do {                                                                \
        v0ureg _usrc = (src);                                           \
        v0ureg _udest = (dest);                                         \
                                                                        \
        _usrc = -_usrc;                                                 \
        _udest += _usrc;                                                \
        (dest) = _udest;                                                \
    } while (0)

// dest -= src, set carry-bit
#define v0sbcop(src, dest, flg)                                         \
    do {                                                                \
        v0ureg _usrc = (src);                                           \
        v0ureg _udest = (dest);                                         \
                                                                        \
        _udest -= _usrc;                                                \
        if ((dest) < _udest) {                                          \
            (flg) |= V0_MSW_CF_BIT;                                     \
        }                                                               \
        (dest) = _udest;                                                \
    } while (0)

// dest = ~dest + 1, arithmetic negation, 2's complement
#define v0negop(src, dest, flg)                                         \
    do {                                                                \
        v0ureg _udest = (dest);                                         \
        v0ureg _utmp = ~_udest;                                         \
                                                                        \
        _utmp++;                                                        \
        (dest) = _utmp;                                                 \
    } while (0)

// dest ^= 0xffffffff, logical negation
#define v0notop(src, dest, flg)                                         \
    do {                                                                \
        v0ureg _udest = (dest);                                         \
        v0ureg _umask = 0xffffffffU;                                    \
                                                                        \
        _udest ^= _umask;                                               \
        (dest) = _udest;                                                \
    } while (0)

// a & b = ~(~a | ~b);
#define v0andop(src, dest, flg)                                         \
    do {                                                                \
        v0ureg _usrc = (src);                                           \
        v0ureg _udest = (dest);                                         \
        v0ureg _utmp;                                                   \
                                                                        \
        _usrc = ~_usrc;                                                 \
        _udest = ~_udest;                                               \
        _utmp = _usrc;                                                  \
        _utmp |= _udest;                                                \
        _utmp ~= _utmp;                                                 \
        (dest) = _utmp;                                                 \
    } while (0)

// a | b = ~(~a & ~b)
#define v0xorop(src, dest, flg)                                         \
    do {                                                                \
        v0ureg _usrc = (src);                                           \
        v0ureg _udest = (dest);                                         \
                                                                        \
        _usrc = ~_usrc;                                                 \
        _udest = ~_udest;                                               \
        _udest &= _usrc;                                                \
        (dest) = _udest;                                                \
    } while (0)

// a | b = ~(~a & ~b)
#define v0lorop(src, dest, flg)                                         \
    do {                                                                \
        v0ureg _usrc = (src);                                           \
        v0ureg _udest = (dest);                                         \
        v0ureg _utmp1 = (src);                                          \
        v0ureg _utmp2 = (dest);                                         \
                                                                        \
        _utmp2 &= (src);                                                \
        _utmp1 |= (dest);                                               \
        _utmp2 = ~_utmp2;                                               \
        _utmp1 &= _utmp2;                                               \
        (dest) = _utmp1;                                                \
    } while (0)

// count leading zero bits in dest
#define v0clzop(src, dest, flg)                                         \
    do {                                                                \
        uint32_t _ucnt = 32;                                            \
        uint32_t _ures = 32;                                            \
        uint32_t _utmp = (dest);                                        \
        uint32_t _umask;                                                \
                                                                        \
        if (dest) {                                                     \
            _ures = 0;                                                  \
            _utmp = (dest);                                             \
            _umask = 0x01;                                              \
            _ucnt >>= 1;                                                \
            _umask <<= V0_CHAR_BIT * sizeof(uint32_t) - 1;              \
            if (!(_utmp & _umask)) {                                    \
                _umask = 0xffffffff;                                    \
                _umask <<= _ucnt;                                       \
                if (!(_utmp & _umask)) {                                \
                    _utmp <<= _ucnt;                                    \
                    _ures += _ucnt;                                     \
                    _ucnt >>= 1;                                        \
                }                                                       \
                _umask <<= _ucnt;                                       \
                if (!(_utmp & _umask)) {                                \
                    _utmp <<= _ucnt;                                    \
                    _ures += _ucnt;                                     \
                    _ucnt >>= 1;                                        \
                }                                                       \
                _umask <<= _ucnt;                                       \
                if (!(_utmp & _umask)) {                                \
                    _utmp <<= _ucnt;                                    \
                    _ures += _ucnt;                                     \
                    _ucnt >>= 1;                                        \
                }                                                       \
                _umask <<= _ucnt;                                       \
                if (!(_utmp & _umask)) {                                \
                    _utmp <<= _ucnt;                                    \
                    _ures += _ucnt;                                     \
                    _ucnt >>= 1;                                        \
                }                                                       \
                _umask <<= _ucnt;                                       \
                if (!(_utmp & _umask)) {                                \
                    _ures++;                                            \
                }                                                       \
            }                                                           \
        }                                                               \
        (r) = _ures;                                                    \
    } while (0)

/* compute the Hamming weight, i.e. the number of 1-bits in a */

static __inline__ uint32_t
_v0hamopa(uint32_t a)
{
    a = ((a >> 1) & 0x55555555) + (a & 0x55555555);
    /* each 2-bit chunk sums 2 bits */
    a = ((a >> 2) & 0x33333333) + (a & 0x33333333);
    /* each 4-bit chunk sums 4 bits */
    a = ((a >> 4) & 0x0F0F0F0F) + (a & 0x0F0F0F0F);
    /* each 8-bit chunk sums 8 bits */
    a = ((a >> 8) & 0x00FF00FF) + (a & 0x00FF00FF);
    /* each 16-bit chunk sums 16 bits */

    return (a >> 16) + (a & 0x0000FFFF);
}

static __inline__ uint32_t
_v0hamopb(uint32_t a)
{
    uint32_t mask1 = 0x55555555;
    uint32_t mask2 = 0x33333333;
    uint32_t mask3 = 0x0f0f0f0f;
    uint32_t mask4 = 0x00ff00ff;

    a = ((a >> 1) & mask1) + (a & mask1);
    /* each 2-bit chunk sums 2 bits */
    a = ((a >> 2) & mask2) + (a & mask2);
    /* each 4-bit chunk sums 4 bits */
    a = ((a >> 4) & mask3) + (a & mask3);
    /* each 8-bit chunk sums 8 bits */
    a = ((a >> 8) & mask4) + (a & mask4);
    /* each 16-bit chunk sums 16 bits */

    return (a >> 16) + (a & 0x0000FFFF);
}

#define v0hamop(src, dest, flg)                                         \
    do {                                                                \
        uint32_t _udest = (dest);                                       \
        uint32_t _ures;                                                 \
                                                                        \
        _ures = _v0hamopb(_udest);                                      \
        (dest) = _ures;                                                 \
    } while (0)

// sign-extend dest
#define v0sexop(src, dest, flg)                                         \
    do {                                                                \
        uint32_t _udest = (dest);                                       \
        uint32_t _sign = (dest) & 0x80000000;                           \
        uint32_t _clz;                                                  \
        uint32_t _cnt;                                                  \
                                                                        \
        if (_sign) {                                                    \
            _cnt = 31;                                                  \
            v0clzop(src, dest, _clz, 0);                                \
            _sign--;                                                    \
            if (_clz) {                                                 \
                _cnt -= clz;                                            \
                _sign <<= _cnt;                                         \
                _udest |= _sign;                                        \
            }                                                           \
        }                                                               \
        (dest) = _udest;                                                \
    } while (0)

