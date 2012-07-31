#define WPM_WINDOW_WIDTH  1280
#define WPM_WINDOW_HEIGHT 720

#include <pthread.h>
#include "SDL.h"

struct sdl {
    SDL_Surface *screen;
    int          w;
    int          h;
    int          bpp;
};

__thread struct sdl *sdl;

void
sdlinit(int w, int h, int bpp)
{
    SDL_Thread *thread;
    struct sdl  param;

    param.w = w;
    param.h = h;
    param.bpp = bpp;
    thread = SDL_CreateThread(sdlmain, &param);

    return;
}

void
sdldrawpixel(uint32_t argb, int x, int y)
{
    Uint32  color = SDL_MapRGB(sdl->screen->format,
                               redval(argb),
                               greenval(argb),
                               blueval(argb));

    if (SDL_MUSTLOCK(screen)) {
        if (SDL_LockSurface(screen) < 0) {

            return;
        }
    }
    switch (sdl->screen->format->BytesPerPixel) {
        case 1:
            Uint8 *buf = (Uint8 *)screen->pixels + y * sdl->screen->pitch + x;
            *buf = color;

            break;
        case 2:
            Uint16 *buf = (Uint8 *)screen->pixels
                + ((y * sdl->screen->pitch) >> 1)
                + x;
            *buf = color;

            break;
        case 3:
            Uint8 *buf = (Uint8 *)screen->pixels + y * sdl->screen->pitch + x;
            buf[screen->format->RShift >> 3] = r;
            buf[screen->format->GShift >> 3] = g;
            buf[screen->format->BShift >> 3]  = b;

            break;
        case 4:
            Uint32 *buf = (Uint8 *)screen->pixels
                + ((y * sdl->screen->pitch) >> 2)
                + x;
            *buf = color;

            break;
    }
    if (SDL_MUSTLOCK(screen)) {
        SDL_UnlockSurface(screen);
    }
    SDL_UpdateRect(screen, x, y, 1, 1);
}

int
sdlmain(void *param)
{
    SDL_Event event;

    sdl = malloc(sizeof(struct sdl));
    if (param) {
        memcpy(sdl, param, sizeof(struct sdl));
    } else {
        sdl->w = SDLDEFW;
        sdl->h = SDLDEFH;
        sdl->bpp = SDLDEFBPP;
    }
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
        fprintf(stderr, "failed to initialise SDL: %s\n", SDL_GetError());

        exit(1);
    }
    atexit(SDL_Quit);
    sdl->screen = SDL_SetVideoMode(sdl->w, sdl->h, sdl->bpp, SDL_SWSURFACE);
    if (!sdl->screen) {
        fprintf(stderr, "unable to set %dx%d@%d video: %s\n",
                w, h, bpp, SDL_GetError());

        exit(1);
    }
    while (1) {
        SDL_WaitEvent(&event);
        switch (event.type) {
            case SDL_KEYDOWN:
                fprintf(stderr, "you pressed %s\n",
                        SDL_GetKeyName(event.key.keysym.sym));

                break;
            case SDL_MOUSEMOTION:
                fprintf(stderr, "mouse moved by %d, %d to (%d, %d)\n",
                        event.motion.xrel, event.motion.yrel,
                        event.motion.x, event.motion.y);

                break;
            case SDL_MOUSEBUTTONDOWN:
                fprintf(stderr, "mouse button %d pressed at (%d, %d)\n",
                        event.button.button, event.button.x, event.button.y);

                break;
            case SDL_QUIT:

                exit(0);
            default:

                break;
        }
    }

    /* NOTREACHED */
    return;
}

