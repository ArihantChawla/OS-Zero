#ifndef __VT_PTY_H__
#define __VT_PTY_H__

int vtopenpty(char **masterpath, char **slavepath);

struct vtpty {
    int   fd;           // teletype file descriptor
    char *master;       // master device name
    char *slave;        // slave device name
    void *esctab;       // escape sequence parsing structure
};

#endif /* __VT_PTY_H__ */

