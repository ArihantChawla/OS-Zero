#ifndef __ZERO_FASTIDIV_H__
#define __ZERO_FASTIDIV_H__

struct divul {
    unsigned long long magic;
    unsigned long long info;
};

void fastuldiv32gentab(struct divul *duptr, unsigned long lim32);
unsigned long fastuldiv32(unsigned long long num, uint32_t div32,
                          const struct divul *tab);

#endif /* __ZERO_FASTIDIV_H__ */

