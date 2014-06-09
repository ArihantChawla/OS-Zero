#ifndef __UI_UI_H__
#define __UI_UI_H__

#include <ui/conf.h>
#include <ui/win.h>

#define UI_SYS_ZERO 0
#define UI_SYS_XORG 1

struct ui {
    long  type;
    void *env;
    void *api;
};

#if (UI_ENV_XORG)
#include <ui/sys/xorg.h>
#endif

void uisetsys(struct ui *ui, long type);
void uiinit(struct ui *ui, int argc, char *argv[]);

typedef void   uiinit_t(struct ui *, int, char **);
typedef void * uiinitcolors_t(void *, int32_t *, size_t);
struct uiapi {
    uiinit_t       *init;
    uiinitcolors_t *initcolors;
};

#endif /* __UI_UI_H__ */

