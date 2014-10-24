#ifndef __ZFC_ZFC_H__
#define __ZFC_ZFC_H__

#include <x11/x11.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

struct zfcfont {
    FT_Library lib;
    FT_Face    face;
    int        asc;
    int        w;
    int        h;
    Pixmap     bitmaps[256];
    int        wtab[256];
    int        htab[256];
};

#endif

