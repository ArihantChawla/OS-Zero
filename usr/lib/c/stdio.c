/* flg values */
#define FILENORMAL  (1L << 0)
#define FILEDIR     (1L << 1)
#define FILESYM     (1L << 2)
#define FILECDEV    (1L << 3)
#define FILEBDEV    (1L << 4)
#define FILENFLGBIT 5
#define FILEFLGMASK ((1L << FILNFLGBIT) - 1)

/* type-check macros */
#define fileisnormal(fp) ((fp)->flg & FILEFLGMASK == FILENORMAL)
#define fileisdir(fp)    ((fp)->flg & FILEFLGMASK == FILEDIR)
#define fileissym(fp)    ((fp)->flg & FILEFLGMASK == FILESYM)
#define fileiscdev(fp)   ((fp)->flg & FILEFLGMASK == FILECDEV)
#define fileisbdev(fp)   ((fp)->flg & FILEFLGMASK == FILEBDEV)
struct file {
    long    desc;
    long    flg;
    size_t  bufsz;
    void   *buf;
};
