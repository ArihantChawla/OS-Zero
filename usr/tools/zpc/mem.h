#ifndef __ZPC_MEM_H__
#define __ZPC_MEM_H__

#include <zpc/conf.h>

#if (ZPC32BIT)
#include <zpc/mem32.h>
#elif (ZPC64BIT)
#include <zpc/mem64.h>
#endif

#define MEMEXECBIT  (1 << 0)
#define MEMWRITEBIT (1 << 1)
#define MEMPRESBIT  (1 << 2)

#endif /* __ZPC_MEM_H__ */

