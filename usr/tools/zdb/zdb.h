#ifndef __ZDB_DB_H__
#define __ZDB_DB_H__

#define ZDB_DEFUI UI_SYS_XORG

#include <ui/ui.h>
#if (ZPU)
#include <zpu/zpu.h>
#endif
#if (WPM)
#include <wpm/wpm.h>
#endif

struct zdb {
    struct ui      ui;          // user interface
    struct uifont  font;        // font information
    void          *data;        // machine context such as struct zpu *
};

void zdbgetopt(struct zdb *zdb, int argc, char *argv[]);

#endif /* __ZDB_DB_H__ */

