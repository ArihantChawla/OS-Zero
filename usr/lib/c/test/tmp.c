#define maggetptr(mag, ptr)                                             \
    ((uint8_t *)(mag)->base                                             \
     + ((mag)->ptrtab[magptrid(mag, ptr)] << (mag)->bktid))
#if (MALLOCPTRNDX) && 0
#define magputptr(mag, ptr1, ptr2)                                      \
    ((mag)->ptrtab[magptrid(mag, ptr1)] = magptrid(mag, ptr2))
#define magputptr(mag, ptr1, ptr2)                                      \
    (((MAGPTRNDX *)(mag)->ptrtab)[magptrid(mag, ptr1)] = magptrid(mag, ptr2))
#elif 0
#define magputptr(mag, ptr1, ptr2)                                      \
    (((void **)(mag)->ptrtab)[magptrid(mag, ptr1)] = (ptr2))
#endif
#if 0
#define maggetptr(mag, ptr)                                             \
    ((void *)(((uintptr_t)(mag)->base                                   \
               + ptrdiff(mag->base, rounddownpow2((uintptr_t)(ptr),     \
                                                  1UL << ((mag)->bktid))))))
#endif
#if 0
#if (MALLOCHDRPREFIX)
#if (MALLOCNEWHDR)
#endif
#endif
#define maggetptr(mag, ptr)                                             \
    ((getaln(ptr))                                                      \
     ? ((uint8_t *)(ptr) - getaln(ptr))                                 \
     : (void *)((uintptr_t)(mag)->base                                  \
                + ptrdiff(mag->base,                                    \
                          rounddownpow2((uintptr_t)(ptr) - MEMHDRSIZE,  \
                                        1UL << (mag)->bktid))))
#if 0
        if (mag->cur == 1) {
            tab = mag->tab;
            if (tab) {
                __malloclkmtx(&tab->lk);
                if (mag->cur == 1) {
                    if ((mag->prev) && (mag->next)) {
                        mag->next->prev = mag->prev;
                        mag->prev->next = mag->next;
                    } else if (mag->prev) {
                        mag->prev->next = NULL;
                    } else if (mag->next) {
                        mag->next->prev = NULL;
                        tab->ptr = mag->next;
                    } else {
                        tab->ptr = NULL;
                    }
                    mag->tab = NULL;
                }
                __mallocunlkmtx(&tab->lk);
            }
            if (gtpow2(lim, 1)) {
                mag->next = arn->magbkt[bktid].ptr;
                if (mag->next) {
                    mag->next->prev = mag;
                }
                arn->magbkt[bktid].ptr = mag;
            }
        } else if (!mag->cur) {
            tab = mag->tab;
            if (tab) {
                __malloclkmtx(&tab->lk);
                if ((mag->prev) && (mag->next)) {
                    mag->next->prev = mag->prev;
                    mag->prev->next = mag->next;
                } else if (mag->prev) {
                    mag->prev->next = NULL;
                } else if (mag->next) {
                    mag->next->prev = NULL;
                    tab->ptr = mag->next;
                } else {
                    tab->ptr = NULL;
                }
                mag->tab = NULL;
                __mallocunlkmtx(&tab->lk);
            }
        } else {
            mag->next = arn->magbkt[bktid].ptr;
            if (mag->next) {
                mag->next->prev = mag;
            }
            arn->magbkt[bktid].ptr = mag;
        }
    }
    ptr = clrptr(ptrval);
#endif
