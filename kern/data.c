#include <zero/param.h>
#include <zero/fastudiv.h>
#include <kern/sched.h>

/* lookup table for fast division with multiplication and shift */
struct divu16 fastu16divu16tab[SCHEDDIVU16TABSIZE];

