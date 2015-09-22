#ifndef __KERN_MEM_OBJ_H__
#define __KERN_MEM_OBJ_H__

/* flg-values */
#define MEM_SHARE    (1 << 0)
#define MEM_RESIZE   (1 << 1)
#define MEM_GROWDOWN (1 << 2)
#define MEM_UMAP     (1 << 3)
#define MEM_BUF      (1 << 4)
#define MEM_WIRE     (1 << 5)
#define MEM_DMA      (1 << 6)
#define MEM_MMIO     (1 << 7)
#define MEM_PHYS     (1 << 8)
#define MEM_NOCACHE  (1 << 9)
struct memreg {
    struct perm *perm;  // permission structure
    long         flg;   // flag-bits
    void        *adr;	// base address of region
    size_t       ofs;	// offset into region (for locks and such)
    size_t       len;	// length
};

#endif /* __KERN_MEM_OBJ_H__ */

