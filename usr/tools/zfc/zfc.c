#include <zfc/zfc.h>

#define ZFCOUTPUT 1
#if (ANTIALIAS)
#include <stdint.h>
#endif

static struct x11app  *zfcapp;
static struct zfcfont  zfcfont;
static const char     *zfcinfile = "ProggyCleanSZ.ttf";
static int             zfcfontsize = 16;

Pixmap zfcrendft(struct x11app *app, GC gc, unsigned char *data,
                 int boxw, int boxh, int w, int h,
                 int top, int descent,
                 int pitch,
                 struct zfcfont *font,
                 int i);

void
zfcloadft(struct zfcfont *font, const char *filename, int size)
{
    unsigned char *data;
    GC gc;
    Pixmap pixmap, bitmap;
    FT_Error error;
    FT_UInt glyphndx;
    FT_Glyph glyph;
    FT_BitmapGlyph bitmapglyph;
    FT_BBox bbox;
    int i, j;
    int charw, charh;
    int w, h;
    int top, descent;
    int pitch;
    XGCValues gcvalues;

    error = FT_New_Face(font->lib, filename, 0, &font->face);
    if (error) {

        exit(1);
    }
    error = FT_Set_Char_Size(font->face,
			     size * 64, size * 64, 72, 72);
    if (error) {
	error = FT_Set_Pixel_Sizes(font->face, size, size);
    }
    if (error) {

        exit(1);
    }
    charw = font->face->size->metrics.max_advance / 64;
    charh = font->face->size->metrics.height / 64;
    descent = -font->face->size->metrics.descender / 64;
    bitmap = XCreatePixmap(zfcapp->display,
			   zfcapp->win,
			   1, 1,
			   1);
    memset(&gcvalues, 0, sizeof(gcvalues));
    gcvalues.foreground = 1;
    gcvalues.function = GXcopy;
    gcvalues.graphics_exposures = False;
    gc = XCreateGC(zfcapp->display,
		   bitmap,
		   GCForeground | GCFunction | GCGraphicsExposures,
		   &gcvalues);
    XFreePixmap(zfcapp->display, bitmap);
#if (ZFCOUTPUT)
    printf("static uint16_t proggycleansztab[256 * %d] = {\n", charh);
#endif
    for (i = 0 ; i < charh ; i++) {
        printf("0x00, ");
    }
    for (i = 1 ; i < 256 ; i++) {
	glyphndx = FT_Get_Char_Index(font->face, i);
	error = FT_Load_Glyph(font->face, glyphndx, FT_LOAD_MONOCHROME);
	if (!error) {
	    if (font->face->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
		error = FT_Render_Glyph(font->face->glyph,
					FT_RENDER_MODE_MONO);
	    }
	    if (!error) {
		error = FT_Get_Glyph(font->face->glyph, &glyph);
		if (!error) {
		    bitmapglyph = (FT_BitmapGlyph)glyph;
		    w = bitmapglyph->bitmap.width;
		    h = bitmapglyph->bitmap.rows;
		    top = bitmapglyph->top;
		    data = bitmapglyph->bitmap.buffer;
		    pitch = bitmapglyph->bitmap.pitch;
		    if ((w) && (h)) {
			bitmap = zfcrendft(zfcapp, gc, data,
                                           charw, charh,
                                           w, h,
                                           top, descent,
                                           pitch,
                                           font,
                                           i);
			font->bitmaps[i] = bitmap;
#if 0
			font->widths[i] = w;
			font->heights[i] = h;
#endif
#if (ZFCOUTPUT)
                    } else {
                        fprintf(stderr, "FILL: %x\n", charh);
                        for (j = 0 ; j < charh - 1; j++) {
                            printf("    0x00, ");
                        }
                        printf("    0x00,\n");
#endif
                    }
		    FT_Done_Glyph(glyph);
#if (ZFCOUTPUT)
		} else {
                    fprintf(stderr, "FILL: %x\n", charh);
                    for (j = 0 ; j < charh - 1; j++) {
                        printf("    0x00, ");
                    }
                    printf("    0x00,\n");
#endif
                }
	    }
        } else {

            exit(1);
        }
    }
#if (ZFCOUTPUT)
    printf("};\n");
#endif
    XFreeGC(zfcapp->display, gc);
#if 0
    font->ascent = ascent;
#endif
    font->asc = 0;
    font->w = charw;
    font->h = charh;
    fprintf(stderr, "loaded %s (%dx%d)\n", filename, charw, charh);

    return;
}

void
zfcinit(int argc, char *argv[])
{
    zfcapp = x11initapp(NULL);
    zfcapp->win = x11initwin(zfcapp,
                             RootWindow(zfcapp->display,
                                        DefaultScreen(zfcapp->display)),
                             0, 0,
                             1, 1,
                             0);
    if (FT_Init_FreeType(&zfcfont.lib)) {

        exit(1);
    }
    zfcloadft(&zfcfont, zfcinfile, zfcfontsize);
}

Pixmap
zfcrendft(struct x11app *app, GC gc, unsigned char *data,
          int boxw, int boxh, int w, int h,
          int top, int descent,
          int pitch,
          struct zfcfont *font,
          int i)
{
#if (ZFCOUTPUT)
    long val;
    long mask;
#endif
    unsigned char *dataptr;
    Pixmap bitmap;
    int x, y;
    int xoffset, yoffset;
    int bit;
    unsigned char uc;

    bitmap = XCreatePixmap(zfcapp->display,
			   zfcapp->win,
			   boxw, boxh,
			   1);
    XSetForeground(zfcapp->display, gc, 0);
    XFillRectangle(zfcapp->display,
		   bitmap,
		   gc,
		   0, 0,
		   boxw, boxh);
    XSetForeground(zfcapp->display, gc, 1);
    dataptr = data;
    xoffset = (boxw - w) / 2;
    yoffset = boxh - top - descent;
#if (ZFCOUTPUT)
    printf("    ");
#endif
    fprintf(stderr, "PLOT: %d\n", boxh);
    for (y = 0 ; y < yoffset ; y++) {
        printf("0x00, ");
    }
    if (i == 'i' || i == 'l') {
        fprintf(stderr, "%c: xofs == %d, yofs == %d, w == %d, h == %d\n",
                i, xoffset, yoffset, w, h);
    }
    for (; y < yoffset + h ; y++) {
#if (ZFCOUTPUT)
        mask = 1UL << (boxw - xoffset);
        val = 0;
//        val = 0;
//        mask = 1;
#endif
	for (x = 0 ; x < boxw - xoffset ; x++) {
#if (ZFCOUTPUT)
            mask >>= 1;
//            mask <<= 1;
#endif
	    bit = dataptr[(x >> 3)] & (0x80 >> (x & 0x07));
	    if (bit) {
#if (ZFCOUTPUT)
                val |= mask;
#endif
		XDrawPoint(zfcapp->display, bitmap, gc, x, y);
	    }
	}
	dataptr += pitch;
#if (ZFCOUTPUT)
        if (i == 255 && y == boxh - 1) {
            printf("0x%02lx\n", val);
        } else if (y == h - 1) {
            printf("0x%02lx,\n", val);
        } else {
            printf("0x%02lx, ", val);
        }
#endif
    }
    for (; y < boxh ; y++) {
        printf("0x00, ");
    }

    return bitmap;
}

int
main(int argc, char *argv[])
{
    zfcinit(argc, argv);

    exit(0);
}

