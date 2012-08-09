#if (ZPCIMLIB2)

#include <stdio.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <Imlib2.h>
#include "x11.h"

void
imlib2init(struct x11app *app)
{
    imlib_context_set_display(app->display);
    imlib_context_set_drawable(app->win);
    imlib_context_set_visual(app->visual);
    imlib_context_set_colormap(app->colormap);
    imlib_context_set_blend(0);
    imlib_context_set_mask(1);

    return;
}

Pixmap
imlib2loadimage(struct x11app *app, const char *filename, int w, int h)
{
    Pixmap       pmap;
    Pixmap       mask;
    Imlib_Image *img;

    img = imlib_load_image(filename);
    if (img) {
        imlib_context_set_drawable(app->win);
        imlib_context_set_image(img);
        imlib_render_pixmaps_for_whole_image_at_size(&pmap, &mask, w, h);
    }

    return pmap;
}

#endif /* ZPCIMLIB2 */

