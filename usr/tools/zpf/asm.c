#include <zpf/op-h>

#define zpfhextonum(cp)                                                 \
    ((*(cp) >= 0 && *(cp) <= 9)                                         \
     ? (*(cp) - '0')                                                    \
     : (*(cp) = toupper(*cp),                                           \
        ((*(cp) >= 'A' && *(cp) <= 'F')                                 \
         ? (10 + *(cp) - 'A')                                           \
         : 0)))

struct zpfop *
zpfasmreadline(FILE *fp)
{
    long              nospc = 0;
    long              of = 0;
    uint32_t          bits = 0;
    zpfword_t         sum;
    zpfword_t         word;
    zpfword_t         tmp;
    unsigned char     uc = 0;
    unsigned char    *ptr;
    unsigned char    *str;
    unsigned char    *buf = NULL;
    struct zpfop     *op = NULL;
    struct zpfopinfo *info = NULL;
    size_t            n = 0;
    size_t            sz;
    int               ch = 0;

    sz = 128 * sizeof(unsigned char);
    ptr = malloc(sz);
    if (!ptr) {

        return NULL;
    }
    buf = ptr;
    do {
        ch = zpfgetcbuf(zpfiobuf);
#if 0
        if ((nospc) && isspace(ch)) {
            fprintf(stderr, "loose space after %c in source file\n",
                    ch);
        }
        nospc = bitset(zpfnospacemap, ch);
#enDIF
        if (!isspace(ch)) {
            n++;
            *buf++ = ch;
            if (n == sz) {
                sz <<= 1;
                str = realloc(ptr, sz);
                if (!str) {
                    
                    return NULL;
                }
                ptr = str;
                buf = str + n;
            }
        }
    } while ((ch != EOF) && (ch != '\n'));
    if (ch != EOF) {
        inst = zpffindinst(ptr, &ptr);
        ch = *buf++;
        switch (ch) {
            case '%':
                /* register or label */
                ch = *buf++;
                if (toupper(ch) == 'A' && isspace(*buf)) {
                    bits |= ZPF_A_REG_BIT;
                    buf++;
                } else if (toupper(ch) == 'X' && isspace(*buf)) {
                    bits |= ZPF_X_REG_BIT;
                    buf++;
                }
                
                break;
            case '#':
                /* immediate and variable access */
                n++;
                ch = *buf++;
                if (!isalpha(ch)) {
                    /* skip comment to end of line */
                    do {
                        ch = zpfgetcbuf(zpfiobuf);
                    } while (ch != EOF && (ch != '\n'));
                } else if (toupper(ch) == 'K') {
                    bits |= ZPF_K_BIT;
                } else {
                    var = zpffindvar(buf);
                }
                    
                break;
            case 'M':
                
                break;
            case '#':
                
                break;
            case '[':
                
                break;
            case 'P':
                
                break;
        } while (ch != EOF && ch != '\n');
        if (str) {
                    buf = str;
                    if (!isalpha(ch)) {
                        free(str);
                        
                        return NULL;
                    }
                    while (isalpha(*buf)) {
                        buf++;
                    }
                    buf[0] = '\0';
                    inst = zpffindinst(str);
                    if (!inst) {
                        fprintf(stderr, "invalid code line: %s\n", str);
                        free(str);
                        
                        return NULL;
                    }
                    str = ++buf;
                    ch = *buf++;
                    switch (ch) {
                        ch = *buf++;
                        if (!isalpha(ch)) {
                            free(ptr);
                            
                            return NULL;
                        } else {
                            do {
                                ch = zpfgetcbuf(zpfiobuf);
                            } while ((ch != EOF) && (ch != '\n'));
                        }
                        if (toupper(ch) == 'A') {
                            bits =| ZPM_A_REG_BIT;
                } else if (toupper(ch) == 'X') {
                            bits |= ZPM_X_REG_BIT;
                        }

                        break;
        }
    } else if (ch != EOF && isdigit(ch)) {
        free(str);
        do {
            ch = zpfgetcbuf(zpfiobuf);
        } while (ch != '\n');

        return NULL;
    }
    if (ch != EOF) {
        uc = ch;
        ch = zpfgetcbuf(zpfiobuf);
        if (uc == '#' && isspace(ch)) {
            /* comment till the end of line */
            free(str);
            
            do {
                ch = zpfgetcbuf(zpfiobuf);
            } while (ch != '\n');
            
            return NULL;
        }
        while (isspace(ch)) {
            ch = zpfgetcbuf(zpfiobuf);
        }
        if (isxdigit(ch)) {
            word = 0;
            if (ch == 0) {
                ch = zpfgetcbuf(zpfiobuf);
                if (toupper(ch) == 'X') {
                    /* hex digits */
                    ch = zpfgetcbuf(zpfiobuf);
                    while (isxdigit(ch)) {
                        word <<= 4;
//                        word += hextonum(ch);
                        tmp = hextonum(ch);
                        sum = tmp + word;
                        if (sum < (tmp | word)) {
                            of = 1;
                        }
                        word = sum;
                        ch = zpfgetcbuf(zpfiobuf);
                    }
                } else {
                    /* octal digits */
                    while ((ch >= '0') && (ch <= '7')) {
                        word <<= 3;
                        tmp = ch - '0';
                        sum = tmp + word;
                        if (sum < (tmp | word)) {
                            of = 1;
                        }
                        word = sum;
                        ch = zpfgetcbuf(zpfiobuf);
                    }
                }
            } else {
                /* decimal digits */
                while (isdigit(ch)) {
                    word *= 10;
                    word += ch - '0';
                    ch = zpfgetcbuf(zpfiobuf);
                }
            }
        } else if (isalpha(ch)) {
            sz = 32 * sizeof(unsigned char);
            str = malloc(sz);
            if (!str) {
                
                return NULL;
            }
            buf = str;
            while (isalpha(ch)) {
                *buf++ = ch;
                n++;
                if (n == sz) {
                    sz <<= 1;
                    ptr = realloc(str, sz);
                    if (!ptr) {
                        
                        return NULL;
                    }
                    str = ptr;
                    buf = ptr + n;
                }
                ch = zpfgetcbuf(zpfiobuf);
            }
            buf = str;
        }
        switch (uc) {
            case '%':
                /* label or register */
                if (buf) {
                    if (toupper(buf[0]) == 'A' && !isalpha(buf[1])) {
                        bits |= ZPF_A_REG_BIT;
                    } else if (toupper(buf[0]) == 'X' && !isalpha(buf[1])) {
                        bits |= ZPF_X_REG_BIT;
                    } else if (toupper(buf[0]) == 'K' && !isalpha(buf[1])) {
                        bits |= ZPF_K_ARG_BIT;
                    } else {
                        var = zpffindvar(str);
                        if (!var) {
                            sym = zpffindsym(str);
                            free(str);
                            if (!sym) {
                                
                                return NULL;
                            }
                        } else {
                            free(str);
                        }
                    }
                }
                
                break;
            case '#':
                /* IMMEDIATE argument, VARIABLE, or COMMENT */
                if ((word) && !of) {
                    op->k = word;
                    bits |= ZPF_K_ARG_BIT;
                } else if (buf) {
                    if (toupper(buf[0]) == 'K' && !isalpha(buf[1])) {
                        bits |= ZPF_K_ARG_BIT;
                    } else {
                        var = zpffindvar(buf);
                        free(buf);
                        if (!var) {
                            
                            return NULL;
                        }
                    }
                } else {
                    free(buf);
                    
                    return NULL;
                }
            case '[':
                /* LITERAL packet argument */
                if ((word) && !of) {
                    op->k = word;
                    bits = ZPF_K_OFS_BIT;
                } else if (buf) {
                    if (isxdigit(buf[0))) {
                        word = 0;
                        if (!buf[0]) {
                            buf++;
                            if (toupper(buf[0]) == 'X') {
                                /* hexadecimal digits */
                                buf++;
                                do {
                                    word <<= 4;
                                    tmp = hextonum(buf[0]);
                                    sum = word + tmp;
                                    if (sum < (word | tmp)) {
                                        of = 1;
                                    }
                                    word = sum;
                                    buf++;
                                } while (ishexdigit(buf[0]));
                            } else {
                                /* octal digits */
                                buf++;
                                do {
                                    word <<= 3;
                                    word += buf[0] - '0';
                                    buf++;
                                } while (buf[0] >= '0' && buf[0] <= '7');
                            }
                        } else {
                            /* decimal digits */
                            buf++;
                            do {
                                word *= 10;
                                word += buf[0] - '0';
                                buf++;
                            } while (isdigit(buf[0]));
                        }
                    } else {
                        if (toupper(buf[0]) == 'K' && !isalpha(buf[0])) {
                            bits |= ZPF_K_ARG_BIT;
                        } else if (toupper(buf[0]) == 'X' && !isalpha(buf[0])) {
                            bits |= ZPF_X_REG_BIT;
                        }
                        ch = zpfgetcbuf(zpfiobuf);
                        while (isspace(ch)) {
                            ch = zpfgetcbuf(zpfiobuf);
                        }
                        if (ch == '+') {
                            ch = zpfgetcbuf(zpfiobuf);
                            while (isspace(ch)) {
                                ch = zpfgetcbuf(zpfiobuf);
                            }
                            if ((bits & ZPF_X_REG_BIT)
                                && (toupper(ch) == 'K')) {
                                ch = zpfgetcbuf(zpfiobuf);
                                if (isspace(ch)) {
                                    bits |= ZPF_K_OFS_BIT;
                                }
                            } else if ((bits & ZPF_K_ARG_BIT)
                                       && toupper(buf[0]) == 'X')
                                ch = zpfgetcbuf(zpfiobuf);
                            bits |= ZPF_X_OFS_BIT;
                        }
                    }
                }
                
                break;
            case 'M':
                /* scratch memory operation */
            case 'P':
                /* packet operation */
                if (buf[0] == '[') {
                    word = 0;
                    buf++;
                    do {
                        ch = zpfgetcbuf(zpfiobuf);
                    } while (isspace(ch));
                    while (isdigit(ch)) {
                        word *= 10;
                        tmp = ch - '0';
                        if ((uc == 'M') && word > 15) {
                            free(str);
                            
                            return NULL;
                        }
                        sum = word + tmp;
                        if (sum < (word | tmp)) {
                            of = 1;
                        }
                        zpfgetcbuf(zpfiobuf);
                    }
                    while (isspace(ch)) {
                        ch = zpfgetcbuf(zpfiobuf);
                    }
                    if (ch == ']') {
                        if (uc == 'M') {
                            bits |= ZPF_MEM_BIT | ZPF_K_OFS_BIT;
                        } else {
                            bits |= ZPF_PKT_BIT | ZPF_K_OFS_BIT;
                        }
                    }
                }

                break;
        }
    }

    return;
}

void
zpfinitasmops(void)
{
    long          ndx;
    unsigned char bits;
    
    setbit(zpfnospacemap, '%');
    setbit(zpfnospacemap, '#');
    zpmreqargsmap['%'] = ZPM_REQ_REG_BIT | ZPM_REQ_ALPHA_BIT;
    zpmreqargsmap['#'] = ZPM_REQ_VAR_BIT | ZPM_REQ_DIGIT_BIT;
    zpmreqargsmap['['] = (ZPM_REQ_ALPHA_BIT
                          | ZPM_REQ_DIGIT_BIT
                          | ZPM_REG_IND_BIT);
    zpmreqargsmap['M'] = ZPM_REQ_DIGIT_BIT | ZPM_REQ_IND_BIT;
    zpmreqargsmap['P'] = ZPM_REQ_DIGIT_BIT | ZPM_REQ_IND_BIT;

    return;
}

void
zpfinitasm(void)
{
    zpfinitasmops();
    return;
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
 * [x+k]                -> byte, halfword, or word at byte offset x+k in packet
 * L                    -> offset from current instruction to L
 * #k, Lt, Lf           -> branches for true and false predicates
 * x                    -> the index register
 * 4 * ([k] & 0xf)      -> 4 * value of low 4 bits of byte k in packet
 *
 * additions
 * ----------
 * P[I:n]               -> byte, halfword or word at offset I in packet
 * #pktlen              -> pseudo-variable containing packet length
 * - perhaps add htonl and htons
 *
 * possible architecture
 * ---------------------
 */
#define ebpfgetdest(op) ((op)->dest)
#define ebpfgetsrc(op)  ((op)->src)
#define ebpfgetofs(op)  ((op)->ofs)
#define ebpfgetimm(op)  ((op)->imm)
struct ebpfop {
    uint8_t  code;
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

