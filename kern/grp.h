#ifndef __GRP_H__
#define __GRP_H__

#include <stddef.h>
#include <sys/types.h>

/* group structure */
struct group {
    char   *gr_name;
    char   *gr_passwd;
    gid_t   gr_id;
    char  **gr_mem;
};

/* function prototypes */
extern struct group *getgrgid (__gid_t __gid);
extern struct group *getgrnam (__const char *__name);

#define NSS_BUFLEN_GROUP 1024

#if (_BSD_SOURCE) || (USESVID) || (USEXOPENEXT)
extern void          setgrent(void);
extern void          endgrent(void);
extern struct group *getgrent(void);
#endif
#if	(USESVID)
extern struct group *fgetgrent (FILE *__stream);
#endif
#if (_GNU_SOURCE)
extern int putgrent (__const struct group *__restrict __p,
                               FILE *__restrict __f);
#endif
#if (_POSIX_SOURCE)

#if (_GNU_SOURCE)
extern int getgrent_r (struct group *__restrict __resultbuf,
                       char *__restrict __buffer, size_t __buflen,
                       struct group **__restrict __result);
#endif

/* Search for an entry with a matching group ID.  */
extern int getgrgid_r (__gid_t __gid, struct group *__restrict __resultbuf,
                       char *__restrict __buffer, size_t __buflen,
                       struct group **__restrict __result);

/* Search for an entry with a matching group name.  */
extern int getgrnam_r (__const char *__restrict __name,
                       struct group *__restrict __resultbuf,
                       char *__restrict __buffer, size_t __buflen,
                       struct group **__restrict __result);

#if (USESVID)
extern int fgetgrent_r (FILE *__restrict __stream,
                        struct group *__restrict __resultbuf,
                        char *__restrict __buffer, size_t __buflen,
                        struct group **__restrict __result);
#endif

#endif

#if (_BSD_SOURCE)
extern int setgroups (size_t __n, __const __gid_t *__groups);
extern int getgrouplist (__const char *__user, __gid_t __group,
                         __gid_t *__groups, int *__ngroups);
extern int initgroups (__const char *__user, __gid_t __group);

#endif /* _BSD_SOURCE */

#endif /* __GRP_H__ */

