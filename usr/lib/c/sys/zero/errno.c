#if defined(__STDC_VERSION__) && (__STDC_VERSION >= 201112L)            \
    && !defined(__STDC_NO_THREADS__)
thread_local int errno;
#elif (PTHREAD) && defined(__GNUC__)
__thread int     errno;
#else
int              errno;
#endif

