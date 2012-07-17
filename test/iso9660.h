#ifndef __ZERO_ISO9660_H__
#define __ZERO_ISO9660_H__

#include <stdint.h>
#include <zero/cdecl.h>

/* descriptor types */
#define ISO9660BOOTREC 0
#define ISO9660PRIVOL  1
#define ISO9660SUPPVOL 2
#define ISO9660VOLPART 3
/* 4-254 are reserved */
#define ISO9660VOLTERM 255

/* extension flags */
#define ISO9660JOLIET1    0x01
#define ISO9660JOLIET2    0x02
#define ISO9660JOLIET3    0x04
#define ISO9660ROCKRIDGE  0x08
#define ISO9660HIGHSIERRA 0x10
#define ISO9660JOLIET     (ISO9660JOLIET1 | ISO9660JOLIET2 | ISO9660JOLIET3)

struct iso9660stime {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    int8_t  gmtofs;
} PACK();

struct iso9660ltime {
    int8_t year[4];
    int8_t month[2]
    int8_t day[2];
    int8_t hour[2];
    int8_t min[2];
    int8_t sec[2];
    int8_t hsec[2];
    int8_t gmtofs;
} PACK();

#define ISO9660IDLEN1     32
#define ISO9660IDLEN2     128
#define ISO9660IDLEN3     37
#define ISO9660APPDATALEN 512
/* primary volume descriptor */
struct iso9660privol {
    uint8_t             type;
    uint8_t             id[5];                          // "CD001"
    uint8_t             version;
    uint8_t             unused1;
    uint8_t             sysid[ISO9660IDLEN1];
    uint8_t             volid[ISO9660IDLEN1];
    uint8_t             unused2[8];
    uint64_t            spacesz;
    uint8_t             unused3[32];
    uint32_t            setsize;
    uint32_t            seqnum;
    uint32_t            logblksz;
    uint64_t            pathtabsz;
    uint32_t            lpathtab;
    uint32_t            optlpathtab;
    uint32_t            mpathtabtype;
    uint32_t            optmpathtab;
    struct iso9660dir   rootdir;
    uint8_t             setid[ISO9660IDLEN2];
    uint8_t             pubid[ISO9660IDLEN2];
    uint8_t             prepid[ISO9660IDLEN2];
    uint8_t             appid[ISO9660IDLEN2];
    uint8_t             copyrfileid[ISO9660IDLEN3];
    uint8_t             abstfileid[ISO9660IDLEN3];
    uint8_t             bibfileid[ISO966IDLEN3];
    struct iso9660ltime cdate;                          // creation date
    struct iso9660ltime mdate;                          // modification date
    struct iso9660ltime edate;                          // expiration date
    struct iso9660ltime fdate;                          // effective date
    uint8_t             fstructver;
    uint8_t             unused4;
    uint8_t             appdata[ISO9660APPDATALEN];
    uint8_t             unused5[653];
} PACK();

struct iso9660suppvol {
    uint8_t             type;
    uint8_t             id[5];                          // "CD001"
    uint8_t             version;
    uint8_t             unused1;
    uint8_t             sysid[ISO9660IDLEN1];
    uint8_t             volid[ISO9660IDLEN1];
    uint8_t             unused2[8];
    uint64_t            spacesz;
    uint8_t             escseq[32];
    uint32_t            setsize;
    uint32_t            seqnum;
    uint32_t            logblksz;
    uint64_t            pathtabsz;
    uint32_t            lpathtab;
    uint32_t            optlpathtab;
    uint32_t            mpathtabtype;
    uint32_t            optmpathtab;
    struct iso9660dir   rootdir;
    uint8_t             setid[ISO9660IDLEN2];
    uint8_t             pubid[ISO9660IDLEN2];
    uint8_t             prepid[ISO9660IDLEN2];
    uint8_t             appid[ISO9660IDLEN2];
    uint8_t             copyrfileid[ISO9660IDLEN3];
    uint8_t             abstfileid[ISO9660IDLEN3];
    uint8_t             bibfileid[ISO966IDLEN3];
    struct iso9660ltime cdate;                          // creation date
    struct iso9660ltime mdate;                          // modification date
    struct iso9660ltime edate;                          // expiration date
    struct iso9660ltime fdate;                          // effective date
    uint8_t             fstructver;
    uint8_t             unused4;
    uint8_t             appdata[ISO9660APPDATALEN];
    uint8_t             unused5[653];
} PACK();

#define ISO9660DIRNAMELEN 38
struct iso9660dir {
    uint8_t             len;
    uint8_t             extatrlen;
    uint64_t            extent;
    uint64_t            size;
    struct iso9660stime rectime;
    uint8_t             flags;
    uint8_t             unitsize;
    uint8_t             gap;
    uint32_t            seqnum;
    uint8_t             namelen;
    uint8_t             name[ISO9660DIRNAMELEN];
} PACK();

/* BIG-endian! */
#define ISO9660STATFILE 1
#define ISO9660STATDIR  2
struct iso9660stat {
    iso_rock_statbuf_t rockridge;
    struct tm          tm;
    lsn_t              lsn;
    uint32_t           size;
    uint32_t           secsize;
    iso9660_xa_t       extatr;
    int32_t            type;
    int32_t            hasextatr;
    uint8_t            filename[EMPTY];
};

#endif /* __ZERO_ISO9660_H__ */

