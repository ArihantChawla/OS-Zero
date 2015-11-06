#ifndef __VT_TERM_H__
#define __VT_TERM_H__

#include <vt/conf.h>
#include <stdint.h>
#include <unistd.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <sys/select.h>

#define TERMINPUTFD    STDIN_FILENO
#define TERMOUTPUTFD   STDOUT_FILENO
#define TERMERRORFD    STDERR_FILENO
#define TERMNSTDFD     3

#define TERMINBUFSIZE  (8 * PAGESIZE)
#define TERMOUTBUFSIZE (8 * PAGESIZE)

//#include <vt/vt.h>
//#include <vt/io.h>

#endif /* __VT_TERM_H__ */

