#ifndef __ZERO_AUD_H__
#define __ZERO_AUD_H__

/* srate (sample rate) values */
#define AUDCDRATE  44100
#define AUDMEDRATE 96000
#define AUDHIRATE  192000
/* fmt (format) arguments */
#define AUD16LE    0
#define AUD24LE    1
#define AUD32LE    2
struct audbuf {
    long  desc;         // object descriptor
    long  srate;        // sample rate in Hz
    long  fmt;          // audio data format
    long  nchan;        // number of audio channels
    void *adr;          // buffer base address
    long  size;         // buffer size in bytes
};

#endif /* __ZERO_AUD_H__ */

