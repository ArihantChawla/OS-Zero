/* valhalla c compiler register allocation/management */

#define VCC_MAX_REGS 256

struct vccmemreg {
    void            *adr;
    union vccregval  val;
};

struct vccreg {
    zerofmtx_t       lk;   // mutual exclusion lock
    union vccregval  val;  // cached register contents
    struct vccreg   *prev; // pointer to previous in queue
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
    struct vccreg *used; // queue of registers in use
    struct vccreg *free; // list of registers deallocated explicitly
};

