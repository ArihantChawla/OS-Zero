#include <zero/param.h>

/*
 * scan bitmap for first zero-bit past ofs
 * return -1 if not found, offset otherwise
 */
long
bfindzerol(unsigned long *bmap, long ofs, long nbit)
{
    long cnt = ofs & ((1UL << (LONGSIZELOG2 + 3)) - 1);
    long ndx = ofs >> (LONGSIZELOG2 + 3);
    long val;
    
    nbit -= ofs;
    if (nbit > 0) {
        val = bmap[ndx];
        val >>= cnt;
        if (val) {
            if (~val) {
                while (val & 0x01) {
                    val >>= 1;
                    ofs++;
                }
            } else {
                ndx++;
                nbit -= ofs + cnt;
                ofs += (1L << (LONGSIZELOG2 + 3)) - cnt;
                while (nbit) {
                    val = bmap[ndx];
                    if (!val) {
                        
                        break;
                    } else if (~val) {
                        while (val & 0x01) {
                            val >>= 1;
                            ofs++;
                        }

                        break;
                    } else {
                        val = 1L << (LONGSIZELOG2 + 3);
                        ndx++;
                        nbit -= val;
                        ofs += val;
                    }
                }
            }
        }
        if (nbit <= 0 || ofs >= nbit) {
            ofs = -1;
        }
    } else {
        ofs = -1;
    }
    
    return ofs;
}

