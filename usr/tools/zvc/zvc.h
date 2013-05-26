/* CRAY-1 uses 24-bit address registers */
#define zvcgetareg(mp, id) ((mp)->a[(id)] & 0x00ffffff)
#define zvcgetbreg(mp, id) ((mp)->b[(id)] & 0x00ffffff)
#define zvcgetsreg(mp, id) ((mp)->s[(id)])
#define zvcgettreg(mp, id) ((mp)->t[(id)])
#define zvcgetv(mp, r, id) ((mp)->v[(r)][(id)])
struct zvcmach {
    /* vector registers */
    uint64_t v[8][64];
    /* intermediate registers */
    /* addresses */
    uint32_t b[64];
    /* scalars */
    uint64_t t[64];
    /* scalar registers */
    uint64_t s[8];
    /* address registers */
    uint32_t a[8];
};
