#ifndef __VT_TERM_H__
#define __VT_TERM_H__

#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <sys/select.h>

#define TERMXORG       1
#define TERMNSCREEN    2
#define TERMNSCREENVT  4

#define TERMINPUTFD    0
#define TERMOUTPUTFD   1
#define TERMERRORFD    2
#if (TERMXORG)
#define TERMCONNFD     3
#define TERMNSCREENFD  4
#else
#define TERMNSCREENFD  3
#endif

#define TERMINBUFSIZE  (8 * PAGESIZE)
#define TERMOUTBUFSIZE (8 * PAGESIZE)

//#include <vt/vt.h>
//#include <vt/io.h>

#endif /* __VT_TERM_H__ */

