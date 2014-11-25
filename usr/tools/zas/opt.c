#include <zas/zas.h>
#include <zas/opt.h>

#if (ZASALIGN)
extern zasmemadr_t zastokalntab[ZASNTOKEN];
#endif

#if (ZASALIGN)
void
zasinitalign(size_t opsize)
{
    zastokalntab[ZASTOKENVALUE] = sizeof(zasword_t);
    zastokalntab[ZASTOKENLABEL] = sizeof(zasword_t);
    zastokalntab[ZASTOKENINST] = opsize;
    zastokalntab[ZASTOKENSYM] = sizeof(zasword_t);
    zastokalntab[ZASTOKENDATA] = sizeof(zasword_t);
    zastokalntab[ZASTOKENSPACE] = sizeof(zasword_t);

    return;
}
#endif

