#include <SDL/SDL.h>
#include <SDL/SDL_main.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#if (PROF)
#include <stdio.h>
#include <zero/prof.h>
#endif

#define PI_OVER_180 (0.01745329252)
#define DEG_TO_RAD(d) ((d)*PI_OVER_180)

#define OUT_WIDTH  640
#define OUT_HEIGHT 480

#define MAX_SHIFT 256

#define OFFSET_MAG  (MAX_SHIFT/2)

#define PALETTE_SIZE (OUT_HEIGHT + MAX_SHIFT)

#define INTER_WIDTH  (OUT_WIDTH + MAX_SHIFT)
#define INTER_HEIGHT (OUT_HEIGHT)

//#define TARGET_FPS 30
#define TARGET_FPS   20

struct fpsctx {
    uint32_t prevtick;
    uint16_t tickInterval;
};

static SDL_Surface* surface;
static SDL_Surface* logo;
static uint32_t palette1[PALETTE_SIZE];
static uint32_t palette2[PALETTE_SIZE];
static uint32_t *intermediate;
static int16_t offsetTable[512];
static struct fpsctx fpstimer;

bool init(void);
void cleanup(void);
bool processEvents(void);
void drawPlasma(SDL_Surface *surface);
void drawLogo(SDL_Surface *surface, const SDL_Surface *logo);
void initfpstimer(struct fpsctx* t, int fpslimit);
void limitfps(struct fpsctx* t);

int main (void)
{
#if (PROF)
    PROFDECLCLK(clk);
#endif

    if (init()) {
        while(!processEvents()) {
#if (PROF)
            profstartclk(clk);
#endif
            drawPlasma(surface);
#if (PROF)
            profstopclk(clk);
            fprintf(stderr, "%ld\n", profclkdiff(clk));
#endif
            drawLogo(surface, logo);
            SDL_Flip(surface);
            limitfps(&fpstimer);
        }
    }

    cleanup();

    return 0;
}

bool init(void)
{
#if (PROF)
    PROFDECLCLK(clk);
#endif

    if (OUT_HEIGHT < OFFSET_MAG) {
        puts("Output screen/window size is too small.");
        return false;
    }

#if (PROF)
    profstartclk(clk);
#endif

    // init sdl
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;
    atexit(SDL_Quit);
    surface = SDL_SetVideoMode(OUT_WIDTH, OUT_HEIGHT, 32,
                               SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!surface) return false;
    SDL_LockSurface(surface);

    // load logo
    SDL_Surface *tmpSurf;
    tmpSurf = SDL_LoadBMP("./zero.bmp");
    if (!tmpSurf) return false;
    if (tmpSurf->w > surface->w || tmpSurf->h > surface->h) {
        puts("Logo too large. Aborting.");
        return 0;
    }
    logo = SDL_ConvertSurface(tmpSurf, surface->format, 0);
    SDL_FreeSurface(tmpSurf);
    SDL_LockSurface(logo);

    // Intermediate pixel destination
    intermediate = malloc(INTER_WIDTH * INTER_HEIGHT * sizeof(*intermediate));
    if (!intermediate) return false;

    unsigned i;

    // init palette
    uint32_t colour;

    const int max_colour = 255;
    const int min_colour = 0;
    double step = (double)(max_colour - min_colour) / (PALETTE_SIZE / 2);

    for (i = 0; i < PALETTE_SIZE / 2; i++) {

        int b = min_colour + ceil(i * step);

        if (b > max_colour) b = max_colour;

        colour = SDL_MapRGB(surface->format, 0, 0, b);
        palette1[i] = colour;
        palette1[PALETTE_SIZE - i - 1] = colour;

        colour = SDL_MapRGB(surface->format, (max_colour-b)/2, b/2, b/4);
        palette2[i] = colour;
        palette2[PALETTE_SIZE - i - 1] = colour;
    }

    // void init_offsetTable(void)
    unsigned len = sizeof offsetTable / sizeof offsetTable[0];
//    double factor = (double)len / 360.0;
    double factor = 360.0 / (double)len;
    for (i = 0; i < len; i++) {
//        offsetTable[i] = sin(DEG_TO_RAD((double)i / len * 360.0)) * OFFSET_MAG;
        offsetTable[i] = sin(DEG_TO_RAD((double)i * factor)) * OFFSET_MAG;
    }

    // set target fps
    initfpstimer(&fpstimer, TARGET_FPS);

#if (PROF)
    profstopclk(clk);
    fprintf(stderr, "INIT: %ld\n", profclkdiff(clk));
#endif

    return true;
}

void cleanup(void)
{
    free(intermediate);
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
            quit = true;
            break;
        }
    }
    return quit;
}

void drawPlasma(SDL_Surface *surface)
{
    int x, y;

    static uint16_t p1_xoff,
                    p1_yoff,
                    p2_xoff,
                    p2_yoff;

    uint16_t    p1_sinpos_start_x = p1_xoff;
    uint16_t    p1_sinpos_start_y = p1_yoff;
    uint16_t    p1_sinposy;
    uint16_t    p1_sinposx;
    int         p1_palettePos = OFFSET_MAG;

    uint16_t    p2_sinpos_start_x = p2_xoff;
    uint16_t    p2_sinpos_start_y = p2_yoff;
    uint16_t    p2_sinposy;
    uint16_t    p2_sinposx;
    int         p2_palettePos = OFFSET_MAG;

    uint32_t *dest = surface->pixels;

    p1_sinposx = p1_sinpos_start_x;
    p2_sinposx = p2_sinpos_start_x;

    unsigned ypos = 0;

    for (y = 0; y < INTER_HEIGHT; y+=2) {

        p1_sinposy = p1_sinpos_start_y;
        p2_sinposy = p2_sinpos_start_y;

        for (x = 0; x < INTER_WIDTH; x++) {
            uint32_t colour;

            // copy to row y    (plasma #1)
            p1_sinposy += 61;
            colour = palette1[p1_palettePos - offsetTable[p1_sinposy>>7]];
            *(intermediate + x + ypos) = colour;

            // copy to row y+1  (plasma #2)
            p2_sinposy += 43;
            colour = palette2[p1_palettePos - offsetTable[p2_sinposy>>7]];
            *(intermediate + x + ypos + INTER_WIDTH) = colour;

        }
        for (x = 0; x < OUT_WIDTH; x++) {
            int srcpos;

            // copy to row y
            srcpos = OFFSET_MAG + x
                        + ypos - (offsetTable[p1_sinposx>>7]>>1);
            *(dest + x) = *(intermediate + srcpos);

            // copy to row y+1
            srcpos = OFFSET_MAG + x
                        + ypos + INTER_WIDTH - (offsetTable[p2_sinposx>>7]);
            *(dest + x + surface->w) = *(intermediate + srcpos);
        }

        p1_palettePos++;
        p1_sinposx += 263;

        p2_palettePos++;
        p2_sinposx += 191;

        // advance 2 rows (lines) ahead because there are 2 interlace plasmas
        dest += surface->w*2;
        ypos += INTER_WIDTH*2;

    }

    p1_xoff += 1087;    // Lots of magic values. They're all prime numbers
    p1_yoff += 307;     // because I figure that will make them more magical.
    p2_xoff += 901;
    p2_yoff += 215;
}

void drawLogo(SDL_Surface *surface, const SDL_Surface *logo)
{
    unsigned srcx, srcy;
    uint32_t *src;
    uint32_t *dest;
    unsigned destxmod;

    /* There is no error checking here... the width and the height of the logo
     * are important. If the logo is too large things will explode.
     */

    destxmod = surface->w - logo->w;

    dest = surface->pixels;
//    dest += (surface->w - logo->w)/3;                     // start dest xpos
//    dest += (surface->h - logo->h) * 2/3 * surface->w;    // start dest ypos
    dest += (surface->w - logo->w) >> 1;
    dest += ((surface->h - logo->h) >> 1) * surface->w;

    src = logo->pixels;

    for (srcy = 0; srcy < logo->h; srcy++) {
        for (srcx = 0; srcx < logo->w; srcx++) {
            if (*src) {
                *(dest) = *src;
            }
            src++;
            dest++;
        }
        dest += destxmod;
    }
}

void initfpstimer(struct fpsctx* t, int fpslimit)
{
    t->prevtick  = SDL_GetTicks();
    t->tickInterval = 1000/fpslimit;
}

void limitfps(struct fpsctx* t)
{
    uint32_t currtick, expected;

    /* Have to be careful with calcs here because everything is unsigned;
     * i.e. don't use subtraction unless certain it won't cause an underflow ;-)
     */
    currtick = SDL_GetTicks();
    expected = t->prevtick + t->tickInterval;

    if (currtick < expected) {
        SDL_Delay(expected - currtick);
        //printf("Delayed %d\n", expected - currtick);
    }

    t->prevtick = SDL_GetTicks();
}
