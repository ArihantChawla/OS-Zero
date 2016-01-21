#include <zero/fastidiv.h>
#include <kern/sched.h>

/* lookup table for fast division with multiplication and shift */
#if (FASTIDIVWORDSIZE == 64) && 0
struct divu64 fastudiv24tab[SCHEDHISTORYSIZE];
#elif (FASTIDIVWORDSIZE == 32)
struct divu32 fastudiv24tab[SCHEDHISTORYSIZE];
#endif

