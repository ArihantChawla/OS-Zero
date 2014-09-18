#ifndef __ZDB_ZPU_H__
#define __ZDB_ZPU_H__

#include <zdb/zdb.h>
#include <zpu/zpu.h>

/* register access */
#define zdbget_zpu(zdb, r) (((struct zpu *)((zdb)->data))[(r)])
#define zdbgetfp_zpu(zdb)  (((struct zpu *)((zdb)->data))->fp)
#define zdbgetsp_zpu(zdb)  (((struct zpu *)((zdb)->data))->sp)
#define zdbgetpc_zpu(zdb)  (((struct zpu *)((zdb)->data))->pc)
#define zdbgetmsw_zpu(zdb) (((struct zpu *)((zdb)->data))->msw)
/* memory access */
#define zdbgetmemb_zpu(zdb, adr)                                        \
    (((uint8_t *)((struct zpu *)((zdb)->data))->core)[(adr)])
#define zdbgetmemw_zpu(zdb, adr)                                        \
    (((uint16_t *)((struct zpu *)((zdb)->data))->core)[(adr)])
#define zdbgetmem_zpu(zdb, adr)                                         \
    (((uint32_t *)((struct zpu *)((zdb)->data))->core)[(adr)])
#define zdbgetmemq_zpu(zdb, adr)                                        \
    (((uint64_t *)((struct zpu *)((zdb)->data))->core)[(adr)])

#endif /* __ZDB_ZPU_H__ */

