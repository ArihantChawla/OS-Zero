#include <sys/time.h>

int getitimer(itimer_which_t, struct itimerval *val);
extern int setitimer(itimer_which_t, const struct itimerval *__restrict newval,
                     struct itimerval *__restrict oldval);
