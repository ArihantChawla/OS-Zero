#ifndef __ENV_H__
#define __ENV_H__

#include <features.h>
#include <unistd.h>

extern char *getenv(const char *name);
/* return NULL if environment untrusted, i.e. setuig or setgid */
extern char *secure_getenv(const char *name);
#if (_XOPEN_SOURCE)
extern int   putenv(char *string);
#endif
extern int   setenv(const char *name, const char *value, int replace);
#if (USEOLDBSD)
#else
extern void  unsetenv(const char *name);
#endif
extern int   clearenv(void);

#endif /* __ENV_H__ */

