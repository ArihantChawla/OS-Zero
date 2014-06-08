#include <vt/conf.h>
#if (VTTEST)
#include <stdio.h>
#endif
#include <stdlib.h>
#include <limits.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <vt/vt.h>
#include <vt/pty.h>
#include <vt/color.h>

int32_t vtxtermcolortab[256] ALIGNED(PAGESIZE) = {
    VT_XTERM_COLOR_0,
    VT_XTERM_COLOR_1,
    VT_XTERM_COLOR_2,
    VT_XTERM_COLOR_3,
    VT_XTERM_COLOR_4,
    VT_XTERM_COLOR_5,
    VT_XTERM_COLOR_6,
    VT_XTERM_COLOR_7,
    VT_XTERM_COLOR_8,
    VT_XTERM_COLOR_9,
    VT_XTERM_COLOR_10,
    VT_XTERM_COLOR_11,
    VT_XTERM_COLOR_12,
    VT_XTERM_COLOR_13,
    VT_XTERM_COLOR_14,
    VT_XTERM_COLOR_15,
    VT_XTERM_COLOR_16,
    VT_XTERM_COLOR_17,
    VT_XTERM_COLOR_18,
    VT_XTERM_COLOR_19,
    VT_XTERM_COLOR_20,
    VT_XTERM_COLOR_21,
    VT_XTERM_COLOR_22,
    VT_XTERM_COLOR_23,
    VT_XTERM_COLOR_24,
    VT_XTERM_COLOR_25,
    VT_XTERM_COLOR_26,
    VT_XTERM_COLOR_27,
    VT_XTERM_COLOR_28,
    VT_XTERM_COLOR_29,
    VT_XTERM_COLOR_30,
    VT_XTERM_COLOR_31,
    VT_XTERM_COLOR_32,
    VT_XTERM_COLOR_33,
    VT_XTERM_COLOR_34,
    VT_XTERM_COLOR_35,
    VT_XTERM_COLOR_36,
    VT_XTERM_COLOR_37,
    VT_XTERM_COLOR_38,
    VT_XTERM_COLOR_39,
    VT_XTERM_COLOR_40,
    VT_XTERM_COLOR_41,
    VT_XTERM_COLOR_42,
    VT_XTERM_COLOR_43,
    VT_XTERM_COLOR_44,
    VT_XTERM_COLOR_45,
    VT_XTERM_COLOR_46,
    VT_XTERM_COLOR_47,
    VT_XTERM_COLOR_48,
    VT_XTERM_COLOR_49,
    VT_XTERM_COLOR_50,
    VT_XTERM_COLOR_51,
    VT_XTERM_COLOR_52,
    VT_XTERM_COLOR_53,
    VT_XTERM_COLOR_54,
    VT_XTERM_COLOR_55,
    VT_XTERM_COLOR_56,
    VT_XTERM_COLOR_57,
    VT_XTERM_COLOR_58,
    VT_XTERM_COLOR_59,
    VT_XTERM_COLOR_60,
    VT_XTERM_COLOR_61,
    VT_XTERM_COLOR_62,
    VT_XTERM_COLOR_63,
    VT_XTERM_COLOR_64,
    VT_XTERM_COLOR_65,
    VT_XTERM_COLOR_66,
    VT_XTERM_COLOR_67,
    VT_XTERM_COLOR_68,
    VT_XTERM_COLOR_69,
    VT_XTERM_COLOR_70,
    VT_XTERM_COLOR_71,
    VT_XTERM_COLOR_72,
    VT_XTERM_COLOR_73,
    VT_XTERM_COLOR_74,
    VT_XTERM_COLOR_75,
    VT_XTERM_COLOR_76,
    VT_XTERM_COLOR_77,
    VT_XTERM_COLOR_78,
    VT_XTERM_COLOR_79,
    VT_XTERM_COLOR_80,
    VT_XTERM_COLOR_81,
    VT_XTERM_COLOR_82,
    VT_XTERM_COLOR_83,
    VT_XTERM_COLOR_84,
    VT_XTERM_COLOR_85,
    VT_XTERM_COLOR_86,
    VT_XTERM_COLOR_87,
    VT_XTERM_COLOR_88,
    VT_XTERM_COLOR_89,
    VT_XTERM_COLOR_90,
    VT_XTERM_COLOR_91,
    VT_XTERM_COLOR_92,
    VT_XTERM_COLOR_93,
    VT_XTERM_COLOR_94,
    VT_XTERM_COLOR_95,
    VT_XTERM_COLOR_96,
    VT_XTERM_COLOR_97,
    VT_XTERM_COLOR_98,
    VT_XTERM_COLOR_99,
    VT_XTERM_COLOR_100,
    VT_XTERM_COLOR_101,
    VT_XTERM_COLOR_102,
    VT_XTERM_COLOR_103,
    VT_XTERM_COLOR_104,
    VT_XTERM_COLOR_105,
    VT_XTERM_COLOR_106,
    VT_XTERM_COLOR_107,
    VT_XTERM_COLOR_108,
    VT_XTERM_COLOR_109,
    VT_XTERM_COLOR_110,
    VT_XTERM_COLOR_111,
    VT_XTERM_COLOR_112,
    VT_XTERM_COLOR_113,
    VT_XTERM_COLOR_114,
    VT_XTERM_COLOR_115,
    VT_XTERM_COLOR_116,
    VT_XTERM_COLOR_117,
    VT_XTERM_COLOR_118,
    VT_XTERM_COLOR_119,
    VT_XTERM_COLOR_120,
    VT_XTERM_COLOR_121,
    VT_XTERM_COLOR_122,
    VT_XTERM_COLOR_123,
    VT_XTERM_COLOR_124,
    VT_XTERM_COLOR_125,
    VT_XTERM_COLOR_126,
    VT_XTERM_COLOR_127,
    VT_XTERM_COLOR_128,
    VT_XTERM_COLOR_129,
    VT_XTERM_COLOR_130,
    VT_XTERM_COLOR_131,
    VT_XTERM_COLOR_132,
    VT_XTERM_COLOR_133,
    VT_XTERM_COLOR_134,
    VT_XTERM_COLOR_135,
    VT_XTERM_COLOR_136,
    VT_XTERM_COLOR_137,
    VT_XTERM_COLOR_138,
    VT_XTERM_COLOR_139,
    VT_XTERM_COLOR_140,
    VT_XTERM_COLOR_141,
    VT_XTERM_COLOR_142,
    VT_XTERM_COLOR_143,
    VT_XTERM_COLOR_144,
    VT_XTERM_COLOR_145,
    VT_XTERM_COLOR_146,
    VT_XTERM_COLOR_147,
    VT_XTERM_COLOR_148,
    VT_XTERM_COLOR_149,
    VT_XTERM_COLOR_150,
    VT_XTERM_COLOR_151,
    VT_XTERM_COLOR_152,
    VT_XTERM_COLOR_153,
    VT_XTERM_COLOR_154,
    VT_XTERM_COLOR_155,
    VT_XTERM_COLOR_156,
    VT_XTERM_COLOR_157,
    VT_XTERM_COLOR_158,
    VT_XTERM_COLOR_159,
    VT_XTERM_COLOR_160,
    VT_XTERM_COLOR_161,
    VT_XTERM_COLOR_162,
    VT_XTERM_COLOR_163,
    VT_XTERM_COLOR_164,
    VT_XTERM_COLOR_165,
    VT_XTERM_COLOR_166,
    VT_XTERM_COLOR_167,
    VT_XTERM_COLOR_168,
    VT_XTERM_COLOR_169,
    VT_XTERM_COLOR_170,
    VT_XTERM_COLOR_171,
    VT_XTERM_COLOR_172,
    VT_XTERM_COLOR_173,
    VT_XTERM_COLOR_174,
    VT_XTERM_COLOR_175,
    VT_XTERM_COLOR_176,
    VT_XTERM_COLOR_177,
    VT_XTERM_COLOR_178,
    VT_XTERM_COLOR_179,
    VT_XTERM_COLOR_180,
    VT_XTERM_COLOR_181,
    VT_XTERM_COLOR_182,
    VT_XTERM_COLOR_183,
    VT_XTERM_COLOR_184,
    VT_XTERM_COLOR_185,
    VT_XTERM_COLOR_186,
    VT_XTERM_COLOR_187,
    VT_XTERM_COLOR_188,
    VT_XTERM_COLOR_189,
    VT_XTERM_COLOR_190,
    VT_XTERM_COLOR_191,
    VT_XTERM_COLOR_192,
    VT_XTERM_COLOR_193,
    VT_XTERM_COLOR_194,
    VT_XTERM_COLOR_195,
    VT_XTERM_COLOR_196,
    VT_XTERM_COLOR_197,
    VT_XTERM_COLOR_198,
    VT_XTERM_COLOR_199,
    VT_XTERM_COLOR_200,
    VT_XTERM_COLOR_201,
    VT_XTERM_COLOR_202,
    VT_XTERM_COLOR_203,
    VT_XTERM_COLOR_204,
    VT_XTERM_COLOR_205,
    VT_XTERM_COLOR_206,
    VT_XTERM_COLOR_207,
    VT_XTERM_COLOR_208,
    VT_XTERM_COLOR_209,
    VT_XTERM_COLOR_210,
    VT_XTERM_COLOR_211,
    VT_XTERM_COLOR_212,
    VT_XTERM_COLOR_213,
    VT_XTERM_COLOR_214,
    VT_XTERM_COLOR_215,
    VT_XTERM_COLOR_216,
    VT_XTERM_COLOR_217,
    VT_XTERM_COLOR_218,
    VT_XTERM_COLOR_219,
    VT_XTERM_COLOR_220,
    VT_XTERM_COLOR_221,
    VT_XTERM_COLOR_222,
    VT_XTERM_COLOR_223,
    VT_XTERM_COLOR_224,
    VT_XTERM_COLOR_225,
    VT_XTERM_COLOR_226,
    VT_XTERM_COLOR_227,
    VT_XTERM_COLOR_228,
    VT_XTERM_COLOR_229,
    VT_XTERM_COLOR_230,
    VT_XTERM_COLOR_231,
    VT_XTERM_COLOR_232,
    VT_XTERM_COLOR_233,
    VT_XTERM_COLOR_234,
    VT_XTERM_COLOR_235,
    VT_XTERM_COLOR_236,
    VT_XTERM_COLOR_237,
    VT_XTERM_COLOR_238,
    VT_XTERM_COLOR_239,
    VT_XTERM_COLOR_240,
    VT_XTERM_COLOR_241,
    VT_XTERM_COLOR_242,
    VT_XTERM_COLOR_243,
    VT_XTERM_COLOR_244,
    VT_XTERM_COLOR_245,
    VT_XTERM_COLOR_246,
    VT_XTERM_COLOR_247,
    VT_XTERM_COLOR_248,
    VT_XTERM_COLOR_249,
    VT_XTERM_COLOR_250,
    VT_XTERM_COLOR_251,
    VT_XTERM_COLOR_252,
    VT_XTERM_COLOR_253,
    VT_XTERM_COLOR_254,
    VT_XTERM_COLOR_255
};

#define vtdefcolor(i)     (vtdefcolortab[(c)])
#define vtfgtodefcolor(i) ((c) - 30)
#define vtbgtodefcolor(i) ((c) - 40)
int32_t vtdefcolortab[16] ALIGNED(CLSIZE) = {
    VT_BLACK_COLOR,
    VT_RED_COLOR,
    VT_GREEN_COLOR,
    VT_YELLOW_COLOR,
    VT_BLUE_COLOR,
    VT_MAGENTA_COLOR,
    VT_CYAN_COLOR,
    VT_WHITE_COLOR,
    VT_BRIGHT_BLACK_COLOR,
    VT_BRIGHT_RED_COLOR,
    VT_BRIGHT_GREEN_COLOR,
    VT_BRIGHT_YELLOW_COLOR,
    VT_BRIGHT_BLUE_COLOR,
    VT_BRIGHT_MAGENTA_COLOR,
    VT_BRIGHT_CYAN_COLOR,
    VT_BRIGHT_WHITE_COLOR
};

void
vtfree(struct vt *vt)
{
    if (vt->devbuf.in.base) {
        free(vt->devbuf.in.base);
        vt->devbuf.in.base = NULL;
    }
    if (vt->devbuf.out.base) {
        free(vt->devbuf.out.base);
        vt->devbuf.out.base = NULL;
    }
    if (vt->atr.masterpath) {
        free(vt->atr.masterpath);
        vt->atr.masterpath = NULL;
    }
    if (vt->atr.slavepath) {
        free(vt->atr.slavepath);
        vt->atr.masterpath = NULL;
    }

    return;
}

void
vtfreetextbuf(struct vttextbuf *buf)
{
    int32_t       **data = buf->data;
    struct vtrend **rend = buf->rend;
    long            nrow = buf->nrow;
    long            n;

    if (data) {
        for (n = 0 ; n < nrow ; n++) {
            if (data[n]) {
                free(data[n]);
            }
        }
        free(data);
    }
    if (rend) {
        for (n = 0 ; n < nrow ; n++) {
            if (rend[n]) {
                free(rend[n]);
            }
        }
        free(rend);
    }
}

long
vtinittextbuf(struct vttextbuf *buf, long nrow, long ncol)
{
    long            ndx;
    int32_t       **data;
    struct vtrend **rend;
    int32_t        *dptr;
    struct vtrend  *rptr;

    fprintf(stderr, "TEXTBUF: %ld rows, %ld columns\n", nrow, ncol);
    data = malloc(nrow * sizeof(int32_t *));
    if (!data) {

        return 0;
    }
    rend = malloc(nrow * sizeof(struct vtrend *));
    if (!rend) {
        vtfreetextbuf(buf);
        
        return 0;
    }
    for (ndx = 0 ; ndx < nrow ; ndx++) {
        dptr = calloc(ncol, sizeof(int32_t));
        if (!dptr) {
            vtfreetextbuf(buf);

            return 0;
        }
        data[ndx] = dptr;
        rptr = calloc(ncol, sizeof(struct vtrend));
        if (!rptr) {
            vtfreetextbuf(buf);

            return 0;
        }
        rend[ndx] = rptr;
    }

    return 1;
}

struct vt *
vtinit(struct vt *vt)
{
    long  newvt = (vt) ? 0 : 1;

    if (!vt) {
        vt = malloc(sizeof(struct vt));
        if (!vt) {

            return vt;
        }
    }
    if (!ringinit(&vt->devbuf.in, NULL, VTBUFSIZE / sizeof(RING_ITEM))
        || !ringinit(&vt->devbuf.in, NULL, VTBUFSIZE / sizeof(RING_ITEM))
        || !(vt->atr.masterpath = malloc(PATH_MAX))
        || !(vt->atr.slavepath = malloc(PATH_MAX))) {
        vtfree(vt);
        if (newvt) {
            free(vt);
        }
        
        return NULL;
    }
    vt->atr.fd = vtopenpty(&vt->atr.masterpath, &vt->atr.slavepath);
    if (vt->atr.fd < 0) {
        vtfree(vt);
        if (newvt) {
            free(vt);
        }

        return NULL;
    }
    vt->state.mode = VTDEFMODE;
    vt->state.flags = 0;
    vt->state.fgcolor = VTDEFFGCOLOR;
    vt->state.bgcolor = VTDEFBGCOLOR;
    vt->state.textatr = VTDEFTEXTATR;
    if (!vtinittextbuf(&vt->textbuf, vt->textbuf.nrow, vt->state.ncol)) {
        vtfree(vt);
        if (newvt) {
            free(vt);
        }

        return NULL;
    }
    if (!vtinittextbuf(&vt->scrbuf, vt->state.nrow, vt->state.ncol)) {
        vtfree(vt);
        vtfreetextbuf(&vt->textbuf);
        if (newvt) {
            free(vt);
        }

        return NULL;
    }
    vt->colormap.deftab = vtdefcolortab;
    vt->colormap.xtermtab = vtxtermcolortab;

    return vt;
}

#if (VTTEST)
void
vtprintinfo(struct vt *vt)
{
    fprintf(stderr, "VT(%d): %s, %s\n",
            vt->atr.fd, vt->atr.masterpath, vt->atr.slavepath);

    return;
}

#define VTFONTWIDTH  7
#define VTFONTHEIGHT 13
int
main(int argc, char *argv[])
{
    struct vt vt;

    memset(&vt, 0, sizeof(struct vt));
    vt.state.nrow = 24;
    vt.state.ncol = 80;
    vt.state.w = vt.state.ncol * VTFONTWIDTH;
    vt.state.h = vt.state.nrow * VTFONTHEIGHT;
    vt.textbuf.nrow = VTDEFBUFNROW;
    vt.scrbuf.nrow = 24;
    if (vtinit(&vt)) {
        vtprintinfo(&vt);
    } else {
        fprintf(stderr, "failed to initialise VT\n");
    }

    exit(1);
}
#endif
