/* valhalla c compiler register allocation/management */

struct vccreg {
    zerofmtx_t       lk;  // mutual exclusion lock
    unsigned long    id;  // register ID
    union vccregval  val; // cached register contents
    struct vccreg   *next; // pointer to next in queue;
};

/* ops
 * ---
 * - vccrefreg(reg)
 *   - move in front of usedq
 * - vccgetreg()
 *   - check tail of freeq (back up), queue after tail of usedq
 * - vccfreereg(reg)
 *   - dequeue from usedq, add in front of freeq
 * - vccfreereg(reg, ptr)
 *   - set register backup location to ptr, then call vccfreereg()
 */
struct vccregmgr {
    struct vccreg      usedq; // queue of registers in use
    struct vccreg      freeq; // list of registers deallocated explicitly
};

#define VCC_MAX_REGS 256

struct vccreg vccgenregs[VCC_MAX_REGS];
#if 0
struct vccfreg vccfltregs[VCC_MAX_REGS];
#endif

