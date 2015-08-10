/* zero virtual machine Enlightenment Foundation Libraries interface */

#include <zvm/conf.h>

#if (ZVMEFL)

#include <unistd.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>
#include <zvm/efl.h>

static struct zvmui zvmui;

static void
zvminiteflobjs(void)
{
    /* TODO: initialise objects such as windows here */

    return;
}

static void
zvmcloseefl(void)
{
    ecore_evas_shutdown();

    return;
}

static void
zvmonscrsync(void)
{
    /* TODO: draw the main Evas canvas */

    return;
}

static void
zvminiteflevas(Ecore_Evas *core)
{
    Evas        *evas;
    
    ecore_evas_title_set(core, "Zero Virtual Machine");
    evas = ecore_evas_get(core);
    zvmui.evas = evas;
    ecore_evas_shaped_set(core, 0);
    ecore_evas_show(core);

    return;
}

static void
zvminitefledje(void)
{
    Evas_Object *edje;
    Evas_Coord   w;
    Evas_Coord   h;

    if (!edje_init()) {
        fprintf(stderr, "failed to initialise edje\n");

        exit(1);
    }
    edje = edje_object_add(zvmui.evas);
    zvmui.edje = edje;
    evas_object_move(edje, 0, 0);
    evas_object_resize(edje, ZVMWIDTH, ZVMHEIGHT);
    evas_object_show(edje);

    return;
}

static
Eina_Bool
zvmreadstdin(void *data EINA_UNUSED, Ecore_Fd_Handler *handler EINA_UNUSED)
{
    Eina_List  *lst;
    Ecore_Evas *core;
    char        ch;

    scanf("%c", &ch);
    if (ch == 'h') {
        EINA_LIST_FOREACH(ecore_evas_ecore_evas_list_get(), lst, core)
            ecore_evas_hide(core);
    } else if (ch == 's') {
        EINA_LIST_FOREACH(ecore_evas_ecore_evas_list_get(), lst, core)
            ecore_evas_show(core);
    }
    
    return ECORE_CALLBACK_RENEW;
}

void
zvminitecore(void)
{
    ecore_main_fd_handler_add(STDIN_FILENO, ECORE_FD_READ, zvmreadstdin,
                              NULL, NULL, NULL);
}

void
zvminitui(void)
{
    Ecore_Evas *core;
    Evas       *evas;

    if (!ecore_evas_init()) {
        fprintf(stderr, "failed to initialise EFL\n");

        exit(1);
    }
    core = ecore_evas_new(NULL, 0, 0, ZVMWIDTH, ZVMHEIGHT, NULL);
    zvmui.core = core;
    zvminiteflevas(core);
    zvminitefledje();
    zvminitecore();

    return;
}

static void
zvmstopui(void)
{
    ecore_main_loop_quit();
    ecore_evas_free(zvmui.core);
    ecore_evas_shutdown();
//    ecore_shutdown();

    return;
}

#endif /* ZVMEFL */

