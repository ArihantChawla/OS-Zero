/* REFERENCE: https://bisqwit.iki.fi/story/howto/bitmath/ */

#define v0issigned(i) ((i) & 0x80000000)

// dest -= -src;
#define v0addop(src, dest, res, flg)                                    \
    do {                                                                \
        v0ureg _usrc = (src);                                           \
        v0ureg _udest = (dest);                                         \
                                                                        \
        _usrc = -_usrc;                                                 \
        _udest -= _usrc;                                                \
        (res) = _udest;                                                 \
    } while (0)

// dest -= -src, set carry-bit
#define v0adcop(src, dest, res, flg)                                    \
    do {                                                                \
        v0ureg _usrc = (src);                                           \
        v0ureg _udest = (_udest);                                       \
        v0ureg _u32f = (flg);                                           \
                                                                        \
        _udest += _usrc;                                                \
        if (_udest < _usrc) {                                           \
            (flg) |= V0_MSW_CF_BIT;                                     \
        }                                                               \
        (res) = _udest;                                                 \
    } while (0)

// dest += -src;
#define v0subop(src, dest, res, flg)                                    \
    do {                                                                \
        v0ureg _usrc = (src);                                           \
        v0ureg _udest = (dest);                                         \
                                                                        \
        _usrc = -_usrc;                                                 \
        _udest += _usrc;                                                \
        (res) = _udest;                                                 \
    } while (0)

// dest -= src, set carry-bit
#define v0sbbop(src, dest, res, flg)                                    \
    do {                                                                \
        v0ureg _usrc = (src);                                           \
        v0ureg _udest = (dest);                                         \
                                                                        \
        _udest += _usrc;                                                \
        (res) = _udest;                                                 \
    } while (0)

// dest = ~dest + 1, arithmetic negation, 2's complement
#define v0negop(src, dest, res, flg)                                    \
    do {                                                                \
        v0ureg _udest = (dest);                                         \
        v0ureg _utmp = ~_udest;                                         \
                                                                        \
        _utmp++;                                                        \
        (res) = _utmp;                                                  \
    } while (0)

// dest ^= 0xffffffff, logical negation
#define v0notop(src, dest, res, flg)                                    \
    do {                                                                \
        v0ureg _udest = (dest);                                         \
        v0ureg _umask = 0xffffffffU;                                    \
                                                                        \
        _udest ^= _umask;                                               \
        (res) = _udest;                                                 \
    } while (0)

// a & b = ~(~a | ~b);
#define v0andop(src, dest, res, flg)                                    \
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
        (res) = _utmp;                                                  \
    } while (0)

// a | b = ~(~a & ~b)
#define v0xorop(src, dest, res, flg)                                    \
    do {                                                                \
        v0ureg _usrc = (src);                                           \
        v0ureg _udest = (dest);                                         \
                                                                        \
        _usrc = ~_usrc;                                                 \
        _udest = ~_udest;                                               \
        _udest &= _usrc;                                                \
        (res) = _udest;                                                 \
    } while (0)

// a | b = ~(~a & ~b)
#define v0lorop(src, dest, res, flg)                                    \
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
        (res) = _utmp1;                                                 \
    } while (0)

