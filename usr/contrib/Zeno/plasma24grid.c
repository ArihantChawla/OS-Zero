/* "Old school" demo/intro effect.
 *
 * A plasma-like effect.
 * By CDR - September 2013
 * Rot13 Email: xqr.cflpu ng tznvy.pbz
 *
 * gcc -O3 -lm -lSDL filename.c
 */

/*
 * Tuomo "vendu" Venäläinen
 * - hacks to make it run with zero kernel using graphics framebuffer
 */

#if (__KERNEL__)
#define PLASMADOUBLEBUF 0
#if (PLASMADOUBLEBUF)
#include <kern/util.h>
void plasmasync(void);
#endif
#endif

#if (!__KERNEL__)
#include <SDL/SDL.h>
#include <SDL/SDL_main.h>
#endif
#include <stdbool.h>
#if (!__KERNEL__)
#include <stdio.h>
#include <stdlib.h>
#endif
#include <stdint.h>
#if (!__KERNEL__)
#include <math.h>
#include <time.h>
#endif
#if (__KERNEL__)
#include <gfx/rgb.h>
#include <kern/malloc.h>
#include <kern/io/drv/pc/vbe.h>
#include <kern/unit/x86/asm.h>
#include <kern/unit/x86/pit.h>
#include <kern/unit/i387/math.h>
#endif

#if (__KERNEL__)
extern long vbefontw;
extern long vbefonth;
#define malloc(sz) kmalloc(sz)
#endif

#define PI_OVER_180 (0.01745329252)
#define DEG_TO_RAD(d) ((d)*PI_OVER_180)

#if (__KERNEL__)
#define OUT_WIDTH  1024
#define OUT_HEIGHT 768
#else
#define OUT_WIDTH  800
#define OUT_HEIGHT 600
#endif

#define MAX_SHIFT 256

#define OFFSET_MAG  (MAX_SHIFT/2)

#define PALETTE_SIZE (OUT_HEIGHT + MAX_SHIFT)

#define INTER_WIDTH  (OUT_WIDTH + MAX_SHIFT)
#define INTER_HEIGHT (OUT_HEIGHT)

#define TARGET_FPS 40

#define NOLOGO

#if (__KERNEL__)

#define PLASMAFPS  25

uint8_t *plasmafb;
#if (PLASMADOUBLEBUF)
uint8_t *plasmabuf;
#endif

#else

struct fpsctx {
    uint32_t prevtick;
    uint16_t tickInterval;
};

static SDL_Surface* surface;
static SDL_Surface* logo;

#endif

#if (__KERNEL__)
static uint8_t rtab[INTER_WIDTH * INTER_HEIGHT];
static uint8_t gtab[INTER_WIDTH * INTER_HEIGHT];
static uint8_t btab[INTER_WIDTH * INTER_HEIGHT];
#endif

static uint8_t palette1[PALETTE_SIZE];

static uint8_t *intermediateR;
static uint8_t *intermediateG;
static uint8_t *intermediateB;

static int16_t offsetTable[512];
#if (!__KERNEL__)
static struct fpsctx fpstimer;
#endif

bool init(void);
void cleanup(void);
#if (__KERNEL__)
void plasmainit(void);
void plasmadraw(void);
#else
bool processEvents(void);
void drawPlasma(SDL_Surface *surface);
void drawLogo(SDL_Surface *surface, const SDL_Surface *logo);
#endif
#if (!__KERNEL__)
void initfpstimer(struct fpsctx* t, int fpslimit);
void limitfps(struct fpsctx* t);
#endif


/*
 * TODO: Clean this mess up
 */

#if (__KERNEL__)

void
plasmainit(void)
{
    plasmafb = vbescreen.fbuf;
#if (PLASMADOUBLEBUF)
    plasmabuf = kcalloc(INTER_WIDTH * INTER_HEIGHT
                        * vbescreen.pixsize * sizeof(uint8_t));
#endif
    vbeclrscr(GFX_BLACK);
    __asm__ __volatile__ ("finit\n"
                          "fwait\n");
    init();

    return;
}

void
plasmaloop(long nsec)
{
    static long ndx = 0;
    long        nfrm = PLASMAFPS * nsec;

    plasmainit();
    if (nfrm < 0) {
        for ( ; ; ) {
            plasmadraw();
#if (PLASMADOUBLEBUF)
            plasmasync();
#endif
//        pitsleep((1000 / 2) / PLASMAFPS, NULL);
//        k_waitint();
        }
    } else {
        for (ndx = 0 ; ndx < nfrm ; ndx++) {
            plasmadraw();
#if (PLASMADOUBLEBUF)
            plasmasync();
#endif
//        pitsleep((1000 / 2) / PLASMAFPS, NULL);
//        k_waitint();
        }
    }
#if (__KERNEL__) && (PLASMADOUBLEBUF)
    kfree(plasmabuf);
#endif
}

#else

int main (void)
{
    if (init()) {
        while(!processEvents()) {
            drawPlasma(surface);
#ifndef NOLOGO
            drawLogo(surface, logo);
#endif
            SDL_Flip(surface);
            limitfps(&fpstimer);
        }
    }

    cleanup();

    return 0;
}

#endif /* __KERNEL__ */

bool init(void)
{
#if (!__KERNEL__)
    SDL_Surface *tmpSurf;
#endif
    unsigned i;
#if 0
    const int max_colour = 255;
    const int min_colour = 0;
#endif
    const long max_colour = 255;
    const long min_colour = 0;
    double step = (double)(max_colour - min_colour) / (PALETTE_SIZE / 2);
    double tmp;
    unsigned len = sizeof offsetTable / sizeof offsetTable[0];

    if (OUT_HEIGHT < OFFSET_MAG) {
#if (!__KERNEL__)
        puts("Output screen/window size is too small.");
#endif
        return false;
    }

    // init sdl
#if (!__KERNEL__)

    if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;
    atexit(SDL_Quit);
    surface = SDL_SetVideoMode(OUT_WIDTH, OUT_HEIGHT, 32,
                               SDL_HWSURFACE | SDL_DOUBLEBUF);
                               //| SDL_FULLSCREEN);
    if (!surface) return false;
    SDL_LockSurface(surface);

#ifndef NOLOGO
    // load logo
    tmpSurf = SDL_LoadBMP("./oszero.bmp");
    if (!tmpSurf) return false;
    if (tmpSurf->w > surface->w || tmpSurf->h > surface->h) {
#if (!__KERNEL__)
        puts("Logo too large. Aborting.");
#endif
        return 0;
    }
    logo = SDL_ConvertSurface(tmpSurf, surface->format, 0);
    SDL_FreeSurface(tmpSurf);
    SDL_LockSurface(logo);
#endif

#endif /* !__KERNEL__ */

#if (__KERNEL__)
    intermediateR = rtab;
    intermediateG = gtab;
    intermediateB = btab;
#else
    // Intermediate pixel destinations
    intermediateR = malloc(INTER_WIDTH * INTER_HEIGHT * sizeof(*intermediateR));
    if (!intermediateR) return false;

    intermediateG = malloc(INTER_WIDTH * INTER_HEIGHT * sizeof(*intermediateG));
    if (!intermediateG) return false;
    
    intermediateB = malloc(INTER_WIDTH * INTER_HEIGHT * sizeof(*intermediateB));
    if (!intermediateR) return false;
#endif

    // init palettes

    for (i = 0; i < PALETTE_SIZE / 2; i++) {
        int b;

        tmp = ceil(i * step);
        b = min_colour + tmp;
        if (b > max_colour) b = max_colour;

        palette1[i] = b;
        palette1[PALETTE_SIZE - i - 1] = b;

    }

    // void init_offsetTable(void)
    for (i = 0; i < len; i++) {
        offsetTable[i] = sin(DEG_TO_RAD((double)i / len * 360.0)) * OFFSET_MAG;
    }

#if (!__KERNEL__)
    // set target fps
    initfpstimer(&fpstimer, TARGET_FPS);
#endif

    return true;
}

#if (__KERNEL__)
void
cleanup(void)
{
#if 0
    kfree(intermediateR);
    kfree(intermediateG);
    kfree(intermediateB);
#endif
    vbeclrscr(GFX_BLACK);
}
#else
void cleanup(void)
{
    free(intermediateR);
    free(intermediateG);
    free(intermediateB);
    
    SDL_Quit();
}
#endif

#if (!__KERNEL__)

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

#endif

#if (__KERNEL__)
void plasmadraw(void)
#else
void drawPlasma(SDL_Surface *surface)
#endif
{
    int x, y;

    static uint16_t p1_xoff = 0xf000,
                    p1_yoff = 0xe000,
                    p2_xoff = 0x0001,
                    p2_yoff = 0x0003,
                    p3_xoff = 0x0000,
                    p3_yoff = 0x0000;

#if 0                    
    static uint16_t p1_fade = 0x0000,
                    p2_fade = 0x0000,
                    p3_fade = 0x0000;
#endif
    static uint16_t p1_fade = 0x0000;
    
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
    
    uint16_t    p3_sinpos_start_x = p3_xoff;
    uint16_t    p3_sinpos_start_y = p3_yoff;
    uint16_t    p3_sinposy;
    uint16_t    p3_sinposx;
    int         p3_palettePos = OFFSET_MAG;
        
    unsigned ypos = 0;

#if (__KERNEL__)
#if (PLASMADOUBLEBUF)
    uint8_t *dest = plasmabuf;
#else
    uint8_t *dest = plasmafb;
#endif
#else
    uint32_t *dest = surface->pixels;
#endif

    p1_sinposx = p1_sinpos_start_x;
    p2_sinposx = p2_sinpos_start_x;
    p3_sinposx = p3_sinpos_start_x;

    for (y = 0; y < INTER_HEIGHT; y++) {

        p1_sinposy = p1_sinpos_start_y;
        p2_sinposy = p2_sinpos_start_y;
        p3_sinposy = p3_sinpos_start_y;

        for (x = 0; x < INTER_WIDTH; x++) {
            uint32_t colour;

            p1_sinposy += 61;
            colour = palette1[p1_palettePos - offsetTable[p1_sinposy>>7]];
            *(intermediateR + x + ypos) = colour;

            p2_sinposy += 47;
            colour = palette1[p2_palettePos - offsetTable[p2_sinposy>>7]];
            //colour >>= 1;
            *(intermediateG + x + ypos) = colour;
            
            p3_sinposy += 67;
            colour = 255-palette1[p3_palettePos - offsetTable[p3_sinposy>>7]];
            
            *(intermediateB + x + ypos) = colour;

        }
        if (y % vbefonth) {
            for (x = 0; x < OUT_WIDTH; x++) {
                int srcpos;
                uint32_t colour;
#if (__KERNEL__)
                gfxargb32_t r;
                gfxargb32_t g;
                gfxargb32_t b;
#endif
                

                if (x % vbefontw) {
                    // copy to row y
                    srcpos = OFFSET_MAG + x
                                + ypos - (offsetTable[p1_sinposx>>7]>>1);
                    
                    /* FIXME: SDL_MapRGB() is very likely not a very fast way to do this
                    */
#if (__KERNEL__)
                    r = intermediateR[srcpos];
                    g = intermediateG[srcpos];
                    b = intermediateB[srcpos];
                    colour = gfxmkpix(0, r, g, b);
                    gfxsetrgb888(colour, dest + x * 3);
#else
                    colour = SDL_MapRGB(surface->format,
                                        intermediateG[srcpos],
                                        intermediateR[srcpos],
                                        intermediateB[srcpos]);
                    *(dest + x) = colour;
#endif
                }
#if (__KERNEL__)
#endif
            }
        }

        p1_palettePos++;
        p1_sinposx += 263;
        p1_fade++;
        
        p2_palettePos++;
        p2_sinposx += 907;
                
        p3_palettePos++;
        p3_sinposx += 397;

        // advance to next output row
#if (__KERNEL__)
        dest += vbescreen.w * 3;
#else
        dest += surface->w;
#endif
        ypos += INTER_WIDTH;

    }

    p1_xoff += 1559;//1087;    // Lots of magic values. They're all prime numbers
    p1_yoff += 307;     // because I figure that will make them more magical.
    
    p2_xoff += 521;//503;
    p2_yoff += 179;
    
    p3_xoff += 131;
    p3_yoff += 89;
    
}

#if (__KERNEL__) && (PLASMADOUBLEBUF)
void
plasmasync(void)
{
    kmemcpy(plasmafb, plasmabuf,
            INTER_WIDTH * INTER_HEIGHT * 3 * sizeof(uint8_t));

    return;
}
#endif

#ifndef NOLOGO
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
    
#if 0
    dest += (surface->w - logo->w) / 3;                   // start dest xpos
    dest += (surface->h - logo->h) * 2/3 * surface->w;    // start dest ypos
#else
    dest += (surface->w - logo->w) / 2;                   // start dest xpos
    dest += (surface->h - logo->h) / 2 * surface->w;      // start dest ypos
#endif

    src = logo->pixels;

    for (srcy = 0; srcy < logo->h; srcy++) {
        for (srcx = 0; srcx < logo->w; srcx++) {
            if (*src) {
                *dest = *src;
            }
            src++;
            dest++;
        }
        dest += destxmod;
    }
}
#endif

#if (!__KERNEL__)

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

#endif /* !__KERNEL__ */

