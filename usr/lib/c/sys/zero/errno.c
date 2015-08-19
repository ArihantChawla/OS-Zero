#include <errno.h>

#if (PTHREAD)
__thread int errno;
#else
int          errno;
#endif

