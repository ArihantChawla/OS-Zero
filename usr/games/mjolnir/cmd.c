#include <zero/cdecl.h>
#include <zero/param.h>
#include <mjolnir/mjol.h>

extern void mjolbeep(void);

mjolcmdfunc *mjolcmdfunctab[256] ALIGNED(CLSIZE);
uint8_t      mjolcmdhasdirmap[32];
uint8_t      mjolcmdhasargmap[32];

long
mjolidentify(struct mjolchr *src, struct mjolobj *dest)
{
    return 0;
}

long
mjolmoveleft(struct mjolchr *src, struct mjolobj *dest)
{
    return 0;
}

long
mjolmovedown(struct mjolchr *src, struct mjolobj *dest)
{
    return 0;
}

long
mjolmoveup(struct mjolchr *src, struct mjolobj *dest)
{
    return 0;
}

long
mjolmoveright(struct mjolchr *src, struct mjolobj *dest)
{
    return 0;
}

long
mjolmoveupleft(struct mjolchr *src, struct mjolobj *dest)
{
    return 0;
}

long
mjolmoveupright(struct mjolchr *src, struct mjolobj *dest)
{
    return 0;
}

long
mjolmovedownleft(struct mjolchr *src, struct mjolobj *dest)
{
    return 0;
}

long
mjolmovedownright(struct mjolchr *src, struct mjolobj *dest)
{
    return 0;
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
    mjolsetdir(MJOL_CMD_FIGHT);
    mjolsetdir(MJOL_CMD_THROW);
    mjolsetdir(MJOL_CMD_MOVE);
    mjolsetdir(MJOL_CMD_ZAP);
    mjolsetdir(MJOL_CMD_IDENTIFY_TRAP);

    return;
}

