#include <SDL/SDL.h>
#include <SDL/SDL_main.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define OUT_WIDTH       640
#define OUT_HEIGHT      480

#define PALETTE_SIZE    256

#define SHIFT_MAG       64
#define FIRE_OVERDRAW   (SHIFT_MAG*3)
#define FIRE_BUFF_W     (OUT_WIDTH + FIRE_OVERDRAW)
#define FIRE_BUFF_H     (PALETTE_SIZE*2/3)
#define FIRE_SEEDROWS   1

#define TARGET_FPS 100

struct fpsctx {
    uint32_t prevtick;
    uint16_t tickInterval;
};

struct data {
    SDL_Surface *surface;
    uint32_t    *screenpixels;
    uint8_t     *firebuff;
    uint32_t    *palette;
};


static struct fpsctx fpstimer;

bool init(struct data *ctx);
void drawFire(struct data *ctx);
void cleanup(struct data *ctx);
bool processEvents(void);
void initfpstimer(struct fpsctx* t, int fpslimit);
void limitfps(struct fpsctx* t);

int main (void)
{
    struct data ctx = {0};

    if (init(&ctx)) {
        while(!processEvents()) {
            drawFire(&ctx);
            SDL_Flip(ctx.surface);
            limitfps(&fpstimer);
        }
    }

    cleanup(&ctx);

    return 0;
}

void drawFire(struct data *ctx)
{
    uint32_t *screenpos;
    uint8_t *firepos;
    int i, x, y;
    unsigned ypos;
    static unsigned shift, localshift;

    firepos = ctx->firebuff + (FIRE_BUFF_H - FIRE_SEEDROWS - 1) * FIRE_BUFF_W;
    for (i = 0; i < FIRE_BUFF_W * FIRE_SEEDROWS; i++) {
        uint8_t intensity = rand() % (PALETTE_SIZE+1);
        if (intensity < PALETTE_SIZE/4) intensity = 0;
        *firepos++ = intensity;
    }

    localshift = shift = (shift + 7) % SHIFT_MAG;

    for (ypos = 0, y = 0; y < FIRE_BUFF_H - FIRE_SEEDROWS; y++) {
        localshift = (localshift + 3) % SHIFT_MAG;
        for (x = 1 + SHIFT_MAG/2; x < FIRE_BUFF_W - 1 - SHIFT_MAG; x++) {
            firepos = ctx->firebuff + x + ypos;
            firepos = firepos + localshift - SHIFT_MAG/2;

            uint8_t c = (  *(firepos - 1)
                         + *(firepos + 1)
                         + *(firepos - 1 + FIRE_BUFF_W)
                         + *(firepos + 1 + FIRE_BUFF_W)) >> 2;
            *firepos = c;
        }
        ypos += FIRE_BUFF_W;
    }

    screenpos = ctx->screenpixels + (OUT_HEIGHT - FIRE_BUFF_H + FIRE_SEEDROWS) * OUT_WIDTH;

    for (ypos = 0, y = 0; y < FIRE_BUFF_H - FIRE_SEEDROWS; y++) {
        firepos = ctx->firebuff + (FIRE_OVERDRAW >> 1) + ypos;
        for (x = 0; x < OUT_WIDTH; x++) {
            *screenpos++ = ctx->palette[*(firepos+x)];
        }
        ypos += FIRE_BUFF_W;
    }
}

bool init(struct data *ctx)
{

    srand((unsigned)time(NULL));

    /* init sdl
     */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;
    atexit(SDL_Quit);
    ctx->surface = SDL_SetVideoMode(OUT_WIDTH, OUT_HEIGHT, 32,
                               SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!ctx->surface) return false;
    ctx->screenpixels = ctx->surface->pixels;SDL_LockSurface(ctx->surface);

    /* set target fps
     */
    initfpstimer(&fpstimer, TARGET_FPS);

    /* init palette
     */
    ctx->palette = malloc(sizeof(*ctx->palette) * PALETTE_SIZE);
    if (!ctx->palette) return false;

#if 0
    unsigned i;
    unsigned step = 256 / PALETTE_SIZE;
    unsigned c;
    for (c = 0, i = 0 ; i < PALETTE_SIZE; i++, c += step) {
        ctx->palette[i] = SDL_MapRGB(ctx->surface->format,
                                     c,
                                     c < 150 ? i/5 : 0,
                                     c < 100 ? i/7 : 0);
    }
    //ctx->palette[0] = SDL_MapRGB(ctx->surface->format, 255, 255, 255);
#else // xflame palette
    unsigned i, c;
    unsigned step = 256 / PALETTE_SIZE;
    int r, g, b;
    for (i = 0 ; i < PALETTE_SIZE; i++, c += step) {
        r=c*3;
        g=(c-80)*3;
        b=(c-160)*3;
        if (r<0) r=0;
        if (r>255) r=255;
        if (g<0) g=0;
        if (g>255) g=255;
        if (b<0) b=0;
        if (b>255) b=255;
        ctx->palette[i] = SDL_MapRGB(ctx->surface->format, r, g, b);
    }
#endif

    /* alloc fire buffer
     */
    ctx->firebuff = malloc(sizeof(*ctx->firebuff) * FIRE_BUFF_W * FIRE_BUFF_H);
    if (!ctx->firebuff) return false;
    memset(ctx->firebuff, 0, FIRE_BUFF_W * FIRE_BUFF_H);

    return true;
}

void cleanup(struct data *ctx)
{
    free(ctx->firebuff);
    free(ctx->palette);

    SDL_Quit();
}

bool processEvents(void)
{
    bool quit = false;
    SDL_Event event;

    while(SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            quit = true;
            break;
        case SDL_KEYDOWN:
            quit = event.key.keysym.sym == SDLK_ESCAPE;
            break;
        }
    }
    return quit;
}

void initfpstimer(struct fpsctx* t, int fpslimit)
{
    t->prevtick  = SDL_GetTicks();
    t->tickInterval = 1000/fpslimit;
}

void limitfps(struct fpsctx* t)
{
    uint32_t currtick, expected;

    currtick = SDL_GetTicks();
    expected = t->prevtick + t->tickInterval;

    if (currtick < expected) {
        SDL_Delay(expected - currtick);
        //printf("Delayed %d\n", expected - currtick);
    }

    t->prevtick = SDL_GetTicks();
}

