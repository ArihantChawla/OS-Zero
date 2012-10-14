/* initialise assembly environment */
struct zvm    *zasinit(int argc, char *argv[]);
/* read code line */
struct token  *zasreadline(int fd, void *buf, long bufsz);
/* parse code line into list of tokens */
struct token  *zasparseline(void *buf, long bufsz);
/* translate token list to byte/machine code */
struct zvmobj *zasxlate(struct token *tokptr);
/* evaluate C-like expression */
struct token  *zaseval(struct token *tokptr);
/* run virtual machine; if nstep == 0, run continuously */
sig_t          zasrun(struct zvm *vmptr, long nstep);

#if defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__)
typedef uint32_t zasmemadr_t;
typedef uint32_t zasmachword_t;
#endif
