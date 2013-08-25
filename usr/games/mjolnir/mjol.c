#include <mjolnir/mjol.h>

typedef void (*mjolfunc_t)(struct dngobj *src, struct dngobj *dest);

struct mjolchar {
    long hp;
    long maxhp;
    long gold;
    long str;
    long maxstr;
    long arm;
    long exp;
};

#endif /* ASCIIGFX */
