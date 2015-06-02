#include <unistd.h>

long
sysconf(int name)
{
    long retval;
    
    if (name < 0 || name >= NSYSCONF) {
        errno = EINVAL;
        
        return -1;
    }
    retval = sysgetconf(name);

    return retval;
}

