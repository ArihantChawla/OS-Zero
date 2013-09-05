#include <zero/cdecl.h>
#include <zero/param.h>
#include <mjolnir/mjol.h>

extern void mjolbeep(void);

mjolcmdfunc *mjolcmdfunctab[256] ALIGNED(CLSIZE);
uint8_t      mjolcmdhasdirmap[32];
uint8_t      mjolcmdhasargmap[32];

void
mjolidentify(struct mjolchar *src, struct mjolobj *dest)
{
    ;
}

void
mjolmoveleft(struct mjolchar *src, struct mjolobj *dest)
{
    ;
}

void
mjolmovedown(struct mjolchar *src, struct mjolobj *dest)
{
    ;
}

void
mjolmoveup(struct mjolchar *src, struct mjolobj *dest)
{
    ;
}

void
mjolmoveright(struct mjolchar *src, struct mjolobj *dest)
{
    ;
}

void
mjolmoveupleft(struct mjolchar *src, struct mjolobj *dest)
{
    ;
}

void
mjolmoveupright(struct mjolchar *src, struct mjolobj *dest)
{
    ;
}

void
mjolmovedownleft(struct mjolchar *src, struct mjolobj *dest)
{
    ;
}

void
mjolmovedownright(struct mjolchar *src, struct mjolobj *dest)
{
    ;
}

void
mjolinitcmd(void)
{
    mjolcmdfunctab[MJOL_CMD_IDENTIFY] = mjolidentify;
    mjolcmdfunctab[MJOL_CMD_MOVE_LEFT] = mjolmoveleft;
    mjolcmdfunctab[MJOL_CMD_MOVE_DOWN] = mjolmovedown;
    mjolcmdfunctab[MJOL_CMD_MOVE_UP] = mjolmoveup;
    mjolcmdfunctab[MJOL_CMD_MOVE_RIGHT] = mjolmoveright;
    mjolcmdfunctab[MJOL_CMD_MOVE_UPLEFT] = mjolmoveupleft;
    mjolcmdfunctab[MJOL_CMD_MOVE_UPRIGHT] = mjolmoveupright;
    mjolcmdfunctab[MJOL_CMD_MOVE_DOWNLEFT] = mjolmovedownleft;
    mjolcmdfunctab[MJOL_CMD_MOVE_DOWNRIGHT] = mjolmovedownright;

    return;
}

