struct zpfopfunc  *zpfopfunctab[ZPF_OP_NUNIT][ZPF_OP_NINST];
struct zpfop     **zpfoptab;

struct zpfop *
zpfasmop(struct zvm *vm, struct zpfop *op)
{
    zpfword_t  unit = zpfgetunit(op);
    zpfword_t  inst = zpfgetinst(op);
    uint8_t    narg = vm->opinfo->unitopnargs[unit][0];
    zpfword_t  reg;
    zpfword_t  arg;
    zpfword_t *bitmsp = &zpfopbitmap[unit][0];
    zpfopfunc *func = vm->opinfo->unitfuncs[unit][inst];
    
    if (opisvalid(unit, inst)) {
        switch (unit) {
            case ZPF_ALU_UNIT:
                zpfinitaluop(vm, op, pc, xreg, arg);
                
                break;
        }
    }

    return;
}

struct zpfop *op
zpfasmfile(const char *name, long flg)
{
    FILE *fp;
    
    if (name) {
        fp = fopen(nme, "r");
        if (!fp) {

            return NULL;
        }
    }
}

int
main(int argc, char *argv[])
{
    struct zpfasm *asm;
    
    if (argc == 1) {
        fprintf(stderr, "no input files specified\n");

        exit(1);
    }
    zpfinitasm(argc, argv);

    exit(0);
}

/*
 * BPF syntax
 * -----------
 *
 * #k                   -> literal value stored in k
 * #len                 -> packet length
 * M[k]                 -> word k in stcratch memory store
 * [k]                  -> byte, halfword, or word at byte offset k in packet
 * [x+k}                -> byte, halfword, or word at byte offset x+k in packet
 * L                    -> offset from current instruction to L
 * #k, Lt, Lf           -> branches for true and false predicates
 * x                    -> the index register
 * 4 * ([k] & 0xf)      -> 4 * value of low 4 bits of byte k in packet
 *
 * additions
 * ----------
 * P[I:n]               -> byte, halfword or word at offset I in packet
 * - perhaps add htonl and htons
 *
 * possible architecture
 * ---------------------
 */
struct ebpfop {
    uint8_t code;
    unsigned dest : 4;
    unsigned src  : 4;
    unsigned ofs  : 16;
    uint32_t imm;
};

/*
 * NOTES
 * -----
 * - values within a '[' and ']' are taken literally, allowing for preprocessor
 *   macros and C expressions
 *
 * - labels begin with '%' so like %exec
 * - VAR %port reserves a scratch memory location
 *
 * program starts with
 * -------------------
 * STRUCT <name>
 *
 * program ends with
 * -----------------
 * END
 *
 * LOADS
 * -----
 * - LDA        - load word from memory
 * - LDAI       - load immediate value
 * - LDAB       - load hyte from packet
 * - LDAH       - load haLword from packet
 * - LDAW       - load word from packet
 * - LDAL       - load packet length
 *
 * LDAH and LDA may be used with X-register indexing:
 * LDAD ,X
 *
 * - LDX        - load X with word from memory
 * - LDXI       - load X with immediate word
 * - LDXL       - load X with packet length
 * - LDXIPV4LEN - load X with lenght of ipv4 packet
 *
 * classic architecture
 * --------------------
 * - 32-bit A and X registers
 * - implicit stack of 16 32-bit slots (LD_MEM, ST_MEM)
 * - full integer arithmetic
 * - explicit load-store from packet; LD_ABS, LD_IND
 * - conditional branches with 2 destinations of TRUE and FALSE
 *
 * classic BPF for packets
 * -----------------------
 * - input: skb
 * - output: use-case specific
 * - socket filters
 * - ret 0      - don't pass to user space
 * - ret N      - trim packet to N bytes and pass to user space
 * cls_bpf
 * -------
 * - ret 0      - no match
 * - ret -1     - default classid
 * - ret > 0    - overwrite classid
 *
 * classic BPF for seccomp
 * -----------------------
 * - filter system calls (use for sandboxing)
 * - input: struct seccomp_data
 * - LD_ABS insn was overloeaded
 *
 * - output:
 * - ret 0      - kill task
 * - ret 30000  - data - force sigsys
 * - ret 50000  - errno - return errno
 * - ret 7ff00000 - pass to trace
 * - ret 7fff0000 - allow
 *
 * classic BPF safety
 * - verifier checks all instructions, forward jumps only, stack slot load/store
 *   etc.
 * - instruction set has built-in safety; no exposes stack-pointer, instead load
 *   with 'MEM' modifier
 * - dynamic packet boundary checks
 *
 * classic BFP extensions
 * ----------------------
 * - load from negative hard-coded offset
 * - LD_ABS -0x1000     -> skb->protocol
 * - LD_ABS -0x1000+4   -> skb->pkt_type
 * - LD_ABS -0x1000+56  -> get_ramdom
 *
 * eBPF
 * ----
 * - close relation with X86
¨* - calling convention
 *   - R0       - return vaLue
 *   - R1..R5   - function arguents
 *   - R6..R9   - callee saved
 *   - R10      - frame pointer
 * - register mapping to X86
 *   - R0       - rax   - return vzlue
 *   - R1       - rdi   - 1st argument
 *   - R2       - rsi   - 2nd argument
 *   - R3       - rdx   - 3rd argument
 *   - R4       - rcx   - 4th argument
 *   - R5       - r8    - 5th argument
 *   - R6       - rbx   - callee saved
 *   - R7       - r13   - callee saved
 *   - R8       - r14   - callee saved
 *   - R9       - r15   - callee saved
 *   - R10      - rbp   - frame pointer
 *
 * - ovs + bpf  - action on flow-hit, packet parser before flow lookup,
 *   fallback on flow-miss
 *
 * - tc + bpf
 *   - cls      - packet parser and classifier
 *   - act      - action
 *
 * - linux kernel extensions
 *   - len      - skb->len
 *   - proto    - skb->protocol
 *   - type     - skb->pkt_type
 *   - poff     - payload start offsest
 *   - ifidx    - skb->dev->ifindex
 *   - nla      - netlink attribute of type X with offset A
 *   - nlan     - nested netlink attribute of type X with offset A
 *   - mark     - skb->mark
 *   - queue    - skb->queue_mapping
 *   - hatype   - skb->dev->type
 *   - rxhash   - skb->hash
 *   - cpu      - rzw_smp_processor_id()
 *   - vlan_tci - skb_vlan_tag_get(skb)
 *   - vlan_avail - skb_vlan_tag_present(skb)
 *   - vlan_tpid  - skb->vlan_proto
 *   - rand       - prandom_u32()
 */

/*
 * BPF generalities
 * ----------------
 * - types: host, net, port, portrange
 * - dir: src, dst, src or dst, src and dst
 *   - src or dst is default
 * - proto: ether, fddi, rr, wlan, ip, ip6, arp, rarp, decnet,
 *   tcp, udp
 * - other primitives; gateway, broadcast, less, greater,
 *   arithmetic expressions
 * - logical operations AND and NOT
 */
