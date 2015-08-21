/* zero assembler configuration and command-line options */

#include <zas/conf.h>
#include <zas/zas.h>
#include <zas/opt.h>
#if (ZVM)
#include <zvm/asm.h>
#endif

#if (ZASALIGN)
extern zasmemadr_t zastokalntab[ZASNTOKEN];
#endif

#if (ZASALIGN)
void
zasinitalign(void)
{
	zasmemadr_t aln = sizeof(zasword_t);

    zastokalntab[ZASTOKENVALUE] = aln;
    zastokalntab[ZASTOKENLABEL] = aln;
    zastokalntab[ZASTOKENINST] = aln;
    zastokalntab[ZASTOKENSYM] = aln;
    zastokalntab[ZASTOKENDATA] = aln;
    zastokalntab[ZASTOKENSPACE] = aln;

    return;
}
#endif

