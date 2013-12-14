#ifndef __ZERO_FILE_H__
#define __ZERO_FILE_H__

void * zreadfile(char *filename, size_t *sizeret);
size_t zwritefile(char *filename, void *buf, size_t nb);

#endif /* __ZERO_FILE_H__ */

