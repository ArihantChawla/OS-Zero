#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <features.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#if (_POSIX_SOURCE)
#define _POSIX_VERSION  200112L
#endif
#if (USEXOPEN2K)
#define _XOPEN_VERSION  600
#elif (USEUNIX98)
#define _XOPEN_VERSION  500
#else
#define _XOPEN_VERSION  4
#endif

#if 0
#define _XOPEN_UNIX     1
#define _XOPEN_CRYPT    1
#define _XOPEN_ENH_I18N 1
#define _XOPEN_LEGACY   1
#endif

extern char **__environ;
#if (_GNU_SOURCE)
extern char **environ;
#endif

/* types (hopefully) not declared yet... :) */
#if (_BSD_SOURCE) || (_XOPEN_SOURCE)
/* TODO: check what this type is for and what it should be :) */
typedef long socklen_t;
#endif
/* system types - TODO: figure out which headers to drop some of these into */
//typedef int32_t dev_t;
//typedef int32_t ino_t;
//typedef int32_t mode_t;
//typedef int32_t nlink_t;
typedef int32_t uid_t;
//typedef int32_t gid_t;
//typedef int64_t off_t;
//typedef int64_t time_t;
//typedef int32_t blksize_t;
//typedef int64_t blkcnt_t;
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0	// check for existence
extern int access(const char *name, int type);
#if (_GNU_SOURCE)
extern int euidaccess(const char *name, int type);
extern int eaccess(const char *name, int type);
extern int faccessat(int fd, const char *name, int type, int flg);
#endif
#if (_BSD_SOURCE)
#define L_SET  SEEK_SET
#define L_INCR SEEK_CUR
#define L_XTND SEEK_END
#endif
#if (_FILE_OFFSET_BITS == 64)
#define llseek(fd, ofs, whence) lseek(fd, ofs, whence)
#endif
extern off_t   lseek(int fd, off_t ofs, int whence);
extern ssize_t read(int fd, void *buf, size_t count);
extern ssize_t write(int fd, void *buf, size_t count);
extern int     close(int fd);
#if (USEUNIX98)
extern ssize_t pread(int fd, void *buf, size_t len, off_t ofs);
extern ssize_t pwrite(int fd, const void *buf, size_t len, off_t ofs);
#endif
extern int          pipe(int fd[2]);
extern unsigned int alarm(unsigned int nsec);
extern unsigned int sleep(unsigned int nsec);
#if (_BSD_SOURCE) || (USEXOPENEXT)
extern useconds_t ualarm(useconds_t value, useconds_t interval);
extern int usleep(useconds_t nusec);
#endif
extern int pause(void);
extern int chown(const char *name, uid_t uid, gid_t gid);
#if (_BSD_SOURCE) || (USEXOPENEXT)
extern int fchown(int fd, uid_t uid, gid_t gid);
extern int lchown(const char *name, uid_t uid, gid_t gid);
#endif
#if (_GNU_SOURCE)
extern int fchownat(int fd, const char *name, uid_t uid, gid_t gid);
#endif
#if (_BSD_SOURCE) || (USEXOPENEXT)
extern int fchdir(int fd);
#endif
extern char *getcwd(char *buf, size_t len);
#if (_BSD_SOURCE) || (USEXOPENEXT)
extern char *getwd(char *buf);
#endif
#if (_GNU_SOURCE)
extern char * get_current_dir_name(void);
#endif
int dup(int fd);
int dup2(int fd, int newfd);
extern int execve(const char *path, char *const argv[], char *const envp);
#if (_GNU_SOURCE)
extern int fexecve(int fd, char *const argv, char *const envp[]);
#endif
extern int execv(const char *path, char *const argv[]);
extern int execle(const char *path, const char *arg, ...);
extern int execvp(const char *file, char *const argv[]);
extern int execlp(const char *file, const char *arg, ...);
#if (_XOPEN_SOURCE)
extern int nice(int incr);
#endif
extern void _exit(int status);
extern long int pathconf(const char *path, int name);
extern long int fpathconf(int fd, int name);
extern long int sysconf(int name);
#if (USEPOSIX2)
extern size_t confstr(int name, char *buf, size_t len);
#endif
extern pid_t getpid();
extern pid_t getppid();
#if !(USEOLDBSD)
extern pid_t getpgrp();
#else
#define getpgrp getpgid
#endif
extern pid_t getpgid(pid_t pid);
#if (USEXOPENEXT)
pid_t getpgid(pid_t pid);
#endif
#if (USESVID) || (_BSD_SOURCE) || (USEXOPENEXT)
#if !(USEOLDBSD)
extern int setpgrp(void);
#else
#define setpgrp setpgid
#endif
#endif
extern pid_t setsid(void);
#if (USEXOPENEXT)
extern pid_t getsid(pid_t pid);
#endif
extern uid_t getuid(void);
extern uid_t geteuid(void);
extern gid_t getgid(void);
extern gid_t getegid(void);
extern int getgroups(int len, gid_t list[]);
#if (_GNU_SOURCE)
extern int group_member(gid_t gid);
#endif
extern int setuid(uid_t uid);
#if (_BSD_SOURCE) || (USEXOPENEXT)
extern int setreuid(uid_t ruid, uid_t euid);
#endif
#if (_BSD_SOURCE) || (USEXOPEN2K)
extern int seteuid(uid_t uid);
#endif
extern int setgid(gid_t gid);
#if (_BSD_SOURCE) || (USEXOPENEXT)
extern int setregid(gid_t rgid, gid_t egid);
#endif
#if (_BSD_SOURCE) || (USEXOPEN2K)
extern int setegid(gid_t gid);
#endif
#if (_GNU_SOURCE)
extern int getresuid(uid_t *ruid, uid_t *euid, uid_t *suid);
extern int getresgid(gid_t *rgid, gid_t *egid, gid_t *sgid);
extern int setresuid(uid_t ruid, uid_t euid, uid_t suid);
extern int setresgid(gid_t gid, gid_t egid, gid_t sgid);
#endif
extern pid_t fork(void);
#if (_BSD_SOURCE) || (USEXOPENEXT)
/*
 * - don't clone the whole address space
 * - suspend calling process until exit() or execve()
 */
extern pid_t vfork(void);
#endif
extern char *ttyname(int fd);
extern int   ttyname_r(int fd, char *buf, size_t len);
#if (_BSD_SOURCE) || ((USEXOPENEXT) && !(USEUNIX98))
extern int ttyslot(void);
#endif
extern int link(const char *from, const char *to);
#if (_GNU_SOURCE)
extern int linkat(int fd, const char *from, int newfd, const char *to, int flg);
#endif
#if (_BSD_SOURCE) || defined(USEXOPENEXT) || (USEXOPEN2K)
extern int symlink(const char *from, const char *to);
extern ssize_t readlink(const char *__restrict path, char *__restrict buf,
                        size_t len);
#endif
#if (_GNU_SOURCE)
extern int symlinkat(const char *from, int fd, const char *to);
extern ssize_t readlinkat(int fd, const char *__restrict path,
                          char *__restrict buf, size_t len);
#endif
extern int unlink(const char *name);
#if (_GNU_SOURCE)
extern int unlinkat(int fd, const char *name, int flg);
#endif
extern int rmdir(const char *path);
extern pid_t tcgetpgrp(int fd);
extern int tcsetpgrp(int fd, pid_t pgrp);
extern char *getlogin(void);
#if (_REENTRANT) || (USEPOSIX199506)
extern int getlogin_r(char *name, size_t len);
#endif
#if (_BSD_SOURCE)
extern int setlogin(const char *name);
#endif
#if (USEPOSIX2)
#include <getopt.h>
#endif
#if (_BSD_SOURCE) || (USEUNIX98)
extern int gethostname(char *name, size_t len);
#endif
#if (_BSD_SOURCE) || ((_XOPEN_SOURCE) && !defined(USEUNIX98))
extern int sethostnmae(const char *name, size_t len);
extern int sethostid(long id);
extern int getdomainname(const char *name, size_t len);
extern int setdomainname(const char *name, size_t len);
extern int vhangup(void);
extern int revoke(const char *file);
extern int profil(unsigned short *buf, size_t size, size_t ofs,
                  unsigned int scale);
extern int acct(const char *name);
extern char *getusershell(void);
extern void endusershell(void);
extern void setusershell(void);
extern int daemon(int nochdir, int noclose);
#endif
#if (_BSD_SOURCE) || ((_XOPEN_SOURCE) && !(USEXOPEN2K))
extern int chroot(const char *path);
extern char *getpass(const char *prompt);
#endif
#if (_BSD_SOURCE) || (_XOPEN_SOURCE)
extern int fsync(int fd);
#endif
#if (_BSD_SOURCE) || (USEXOPENEXT)
extern long gethostid(void);
extern void sync(void);
extern int getpagesize(void);
#endif
extern int truncate(const char *file, off_t len);
#if (_BSD_SOURCE) || (USEXOPENEXT) || (USEXOPEN2K)
extern int ftruncate(int fd, off_t len);
#endif
#if (USEXOPENEXT)
extern int brk(void *adr);
extern void *sbrk(intptr_t delta);
#endif
extern long int syscall(long int num, ...);
#if (USEXOPENEXT) && !defined(F_LOCK)
/* these macros also appear in <fcntl.h> - keep the files consistent */
#define F_ULOCK 0
#define F_LOCK  1
#define F_TLOCK 2
#define F_TEST  3
#endif
extern int lockf(int fd, int cmd, off_t len);
#if (_GNU_SOURCE)
#define TEMP_FAILURE_RETRY(expr)                                        \
    (__extension__                                                      \
     ((long _res;                                                       \
       do {                                                             \
           _res = (long)(expr);                                         \
       } while (_res == -1L && errno == EINTR);                         \
       _res;                                                            \
         )))
#endif
#if (USEPOSIX199309) || (USEUNIX98)
extern int fdatasync(int fd);
#endif

#if (_XOPEN_SOURCE)
extern char *crypt(const char *key, const char *salt);
extern void encrypt(char *blk, int flg);
extern void swab(const void *__restrict from, void *__restrict to, ssize_t n);
#endif

#if (_XOPEN_SOURCE)
extern char *ctermid(char *str);
#endif

#if (_XOPEN_SOURCE_EXTENDED)
extern int getdtablesize(void);
#endif

#endif /* __UNISTD_H__ */

