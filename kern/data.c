#include <zero/param.h>
#include <zero/fastidiv.h>
#include <kern/sched.h>

/* lookup table for fast division with multiplication and shift */
struct divu16 fastu32div16tab[rounduppow2(SCHEDHISTORYSIZE, PAGESIZE)];

