AC_DEFUN([LIGHTNING_HEADERS_POSIX],
[
    AC_CHECK_HEADERS(cpio.h dirent.h fcntl.h grp.h pwd.h tar.h termios.h \
                     unistd.h utime.h sys/stat.h sys/times.h sys/types.h \
                     sys/utsname.h sys/wait.h)
])

