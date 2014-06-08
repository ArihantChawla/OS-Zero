#ifndef __UI_UI_H__
#define __UI_UI_H__

#define UI_ENV_ZERO 0
#define UI_ENV_XORG 1

#include <ui/win.h>

struct ui {
    long  type;
    void *env;
};

#endif /* __UI_UI_H__ */

