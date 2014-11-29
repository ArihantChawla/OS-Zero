/* zero virtual machine Enlightenment Foundation Libraries interface */

#ifndef __ZVM_EFL_H__
#define __ZVM_EFL_H__

#include <zvm/conf.h>

#if (ZVMEFL)

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#define ZVMGFXWIDTH  800
#define ZVMGFXHEIGHT 600

/* user interface toplevel windows */
#define ZVMGFXWIN    0
#define ZVMCONSWIN   1
#define ZVMDBGWIN    2
#define ZVMDISASMWIN 3

struct zvmui {
    Ecore_Evas   *core;
    Evas         *evas;
    Evas_Object  *edje;
    Evas_Object **wintab;
};

#endif /* ZVMEFL */

#endif /* __ZVM_EFL_H__ */

