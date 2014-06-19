#ifndef __ZDB_DB_H__
#define __ZDB_DB_H__

#define ZDB_DEFUI   UI_SYS_XORG
#define ZDB_DEFAPI  ZDB_API_ZPU

#define ZDB_API_GDB 1
#define ZDB_API_WPM 2
#define ZDB_API_ZPU 3

#include <ui/ui.h>
#if (ZPU)
#include <zpu/zpu.h>
#endif
#if (WPM)
#include <wpm/wpm.h>
#endif

struct zdbapi;
typedef long zdbapiinitfunc(struct zdbapi *);
struct zdbapi {
    long            type;
    zdbapiinitfunc *init;
};

struct zdb {
    struct ui      ui;          // user interface
    struct uifont  font;        // font information
    struct zdbapi  api;
    void          *data;        // machine context such as struct zpu *
};

void zdbgetopt(struct zdb *zdb, int argc, char *argv[]);

#endif /* __ZDB_DB_H__ */

