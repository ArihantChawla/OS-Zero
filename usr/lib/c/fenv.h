#ifndef __FENV_H__
#define __FENV_H__

#define FE_TOWARDZERO 0
#define FE_TONEAREST  1
#define FE_UPWARD     2
#define FE_DOWNWARD   3

int fesetround(int mode);
int fegetround(int mode);

#endif /* __FENV_H__ */

