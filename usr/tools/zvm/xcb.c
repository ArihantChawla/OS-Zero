#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>
#include <zvm/zvm.h>

xcb_connection_t         *xcbconnection;
xcb_gcontext_t            xcbtextctx;
static xcb_screen_t;      xcbscreen;
static xcb_window_t       xcbuiwin;
static xcb_gcontext_t     xcbgctx;
static xcb_void_cookie_t  xcbcookie;
static xcb_void_cookie_t  xcbtextcookie;
static xcb_void_cookie_t  xcbfontcookie;
static xcb_font_t         xcbtextfont;

void
zvminitui(void)
{
    int                    scrnum;
    xcb_screen_t          *scr;
    xcb_screen_iterator_t  iter;
    xcb_void_cookie_t      cookie;
    xcb_generic_error_t   *error;
    uint32_t               vals[3];
    const char             label[] = "zero virtual machine";
    
    xcbconnection = xcb_connect(NULL, &scrnum);
    iter = xcb_setup_roots_iterator(xcb_get_setup(xcbconnection));
    for ( ; iter.rem ; (--scrnum), xcb_screen_next(&iter)) {
        if (!scrnum) {
            scr = iter.data;

            break;
        }
    }

    printf("\n");
    printf("SCREEN #%ld\n", (long)scr->root);
    printf("------\n");
    printf("\twidth: %d\n", scr->width_in_pixels);
    printf("\theight: %d\n", scr->height_in_pixels);
    xcbuiwin = xcb_generate_id(xcbconnection);
    vals[0] = scr->white_pixel;
    vals[1] = XCB_EVENT_MASK_KEY_RELEASE
        | XCB_EVENT_MASK_BUTTON_PRESS
        | XCB_EVENT_MASK_EXPOSURE
        | XCB_EVENT_MASK_POINTER_MOTION;
    xcb_create_window_checked(xcbconnection,
                              XCB_COPY_FROM_PARENT,
                              xcbuiwin,
                              scr->root,
                              0, 0,
                              640, 480,
                              2,
                              XCB_WINDOW_CLASS_INPUT_OUTPUT,
                              scr->root_visual,
                              0, vals);
    cookie = xcb_map_window_checked(xcbconnection, xcbuiwin);
    error = xcb_request_check(xcbconnection, cookie);
    if (error) {
        fprintf(stderr, "ERROR: cannot create window\n");
        xcb_disconnect(xcbconnection);

        abort();
    }
    xcb_flush(xcbconnection);
    xcbtextfont = xcb_generate_id(xcbconnection);
    xcbfontcookie = xcb_open_font(xcbconnection,
                                  xcbtextfont,
                                  strlen("7x13"),
                                  "7x13");
    vals[0] = scr->black_pixel;
    vals[1] = scr->white_pixel;
    vals[2] = xcbtextfont;
    xcbgctx = xcb_generate_id(xcbconnection);
    xcb_create_gc(xcbconnection, xcbgctx, xcbuiwin,
                  XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT,
                  vals);
    xcbtextcookie = xcb_image_text_8_checked(xcbconnection,
                                             strlen(label),
                                             xcbuiwin,
                                             xcbgctx,
                                             64, 64,
                                             label);
    error = xcb_request_check(xcbconnection, xcbtextcookie);
    if (error) {
        fprintf(stderr, "ERROR: cannot draw text\n");
        xcb_disconnect(xcbconnection);

        abort();
    }
//    xcb_close_font(xcbconnection, xcbtextfont);
    xcb_flush(xcbconnection);

    return;
}

void
xcbdoevent(void)
{
    xcb_generic_event_t *event;
    xcb_generic_error_t *error;
    
    event = xcb_poll_for_event(xcbconnection);
    switch (event->response_type & ~0x80) {
        case XCB_EXPOSE:
        {
            char text[] = "press ESC to exit...";
            
            cookie = xcb_image_text_8_checked(xcbconnection,
                                              strlen(text),
                                              xcbuiwin,
                                              xcbtextgc,
                                              64, 128,
                                              text);
            error = xcb_request_check(xcbconnection,
                                      cookie);
            if (error) {
                fprintf(stderr, "ERROR: can't draw text\n");
                xcb_disconnect(xcbconnection);
                
                exit(1);
            }
        }
    }

    return;
}
