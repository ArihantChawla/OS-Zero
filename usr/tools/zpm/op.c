__inline__ uint8_t *
zpmnot(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src = zpmget1arggenreg(vm, op, dptr);
    zpmreg        dest;

    dest = ~src;
    pc += sizeof(struct zpmop);
    *dptr = dest;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return NULL;
    }
    ptr = &vm->mem[pc];

    return ptr;
}

__inline__ uint8_t *
zpmand(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    pc += sizeof(struct zpmop);
    dest &= src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return NULL;
    }
    ptr = &vm->mem[pc];
    *dptr = dest;

    return ptr;
}

__inline__ uint8_t *
zpmor(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    pc += sizeof(struct zpmop);
    dest |= src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return NULL;
    }
    ptr = &vm->mem[pc];
    *dptr = dest;

    return ptr;
}

__inline__ uint8_t *
zpmxor(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    pc += sizeof(struct zpmop);
    dest ^= src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return NULL;
    }
    ptr = &vm->mem[pc];
    *dptr = dest;

    return ptr;
}

__inline__ uint8_t *
zpmshl(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg2];
    zpmreg        src = ((op->argt & ZPM_IMM_VAL)
                         ? op->imm8
                         : vm->genregs[op->reg1]);
    zpmreg  dest = *dptr;

    dest <<= src;
    pc += sizeof(struct zpmop);
    if (pc > vm->seglims[ZPM_TEXT]) {

        return NULL;
    }
    *dptr = dest;
    ptr = &vm->mem[pc];

    return ptr;
}

__inline__ uint8_t *
zpmshr(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg2];
    zpmreg        src = ((op->argt & ZPM_IMM_VAL)
                         ? op->imm8
                         : vm->genregs[op->reg1]);
    zpmreg        dest = *dptr;
    zpmreg        fill = ~((zpmreg)0) >> src;

    dest >>= src;
    pc += sizeof(struct zpmop);
    dest &= fill;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return NULL;
    }
    ptr = &vm->mem[pc];
    *dptr = dest;

    return ptr;
}

__inline__ uint8_t *
zpmsar(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg2];
    zpmreg        src = ((op->argt & ZPM_IMM_VAL)
                         ? op->imm8
                         : vm->genregs[op->reg1]);
    zpmreg        dest = *dptr;
    zpmreg        mask = ~(zpmreg)0;
#if defined(ZPM64BIT)
    zpmreg        fill = (((dest) & (INT64_C(1) << 63))
                          ? (mask >> (64 - src))
                          : 0);
#else
    zpmreg        fill = (((dest) & (INT32_C(1) << 31))
                          ? (mask >> (32 - src))
                          : 0);
#endif

    dest >>= src;
    fill = -fill << (CHAR_BIT * sizeof(zpmreg) - src);
    pc += sizeof(struct zpmop);
    dest &= fill;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return NULL;
    }
    ptr = &vm->mem[pc];
    *dptr = dest;

    return ptr;
}

__inline__ uint8_t *
zpmrol(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg2];
    zpmreg        src = ((op->argt & ZPM_IMM_VAL)
                         ? op->imm8
                         : vm->genregs[op->reg1]);
    zpmreg        dest = *dptr;
    zpmreg        mask = (~(zpmreg)0) << (CHAR_BIT * sizeof(zpmreg) - src);
    zpmreg        bits = dest & mask;
    zpmreg        cf = dest & ((zpmreg)1 << (src - 1));

    bits >>= CHAR_BIT * sizeof(zpmreg) - 1;
    dest <<= src;
    pc += sizeof(struct zpmop);
    dest |= bits;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return NULL;
    }
    ptr = &vm->mem[pc];
    *dptr = dest;

    return ptr;
}

__inline__ uint8_t *
zpmror(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg2];
    zpmreg        src = ((op->argt & ZPM_IMM_VAL)
                         ? op->imm8
                         : vm->genregs[op->reg1]);
    zpmreg        dest = *dptr;
    zpmreg        mask = (~(zpmreg)0) >> (CHAR_BIT * sizeof(zpmreg) - src);
    zpmreg        bits = dest & mask;
    zpmreg        cf = dest & ((zpmreg)1 << (src - 1));

    bits <<= CHAR_BIT * sizeof(zpmreg) - 1;
    dest >>= src;
    pc += sizeof(struct zpmop);
    dest |= bits;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return NULL;
    }
    ptr = &vm->mem[pc];
    *dptr = dest;

    return ptr;
}

__inline__ uint8_t *
zpminc(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg1];
    zpmreg        src = *dptr;

    src++;
    pc += sizeof(struct zpmop);
    dest = src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return NULL;
    }
    ptr = &vm->mem[pc];
    *dptr = dest;

    return ptr;
}

__inline__ uint8_t *
zpmdec(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr = &vm->genregs[op->reg1];
    zpmreg        src = *dptr;

    src--;
    pc += sizeof(struct zpmop);
    dest = src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return NULL;
    }
    ptr = &vm->mem[pc];
    *dptr = dest;

    return ptr;
}

__inline__ uint8_t *
zpmadd(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;
    zpmreg        res;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    pc += sizeof(struct zpmop);
    res = src + dest;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return NULL;
    }
    if (res < dest) {
        zpmsetof(vm);
    }
    ptr = &vm->mem[pc];
    *dptr = dest;

    return ptr;
}

__inline__ uint8_t *
zpmsub(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    pc += sizeof(struct zpmop);
    dest -= src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return NULL;
    }
    ptr = &vm->mem[pc];
    *dptr = dest;

    return ptr;
}

__inline__ uint8_t *
zpmcmp(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg       *dptr;
    zpmreg        src;
    zpmreg        dest;

    zpmget2argsgenreg(vm, op, dptr, src, dest);
    pc += sizeof(struct zpmop);
    dest -= src;
    if (pc > vm->seglims[ZPM_TEXT]) {

        return NULL;
    }
    zpmclrmsw(vm);
    if (!dest) {
        zpmsetzf(vm);
    } else if (dest < 0) {
        zpmsetcf(vm);
    }
    ptr = &vm->mem[pc];
    *dptr = dest;

    return ptr;
}

__inline__ uint8_t *
zpmmul(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmdiv(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmjmp(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmbz(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    pc += sizeof(struct zpmop);
    if (zpmzfset(vm)) {
        ptr = &vm->mem[dest];
    } else {
        ptr = &vm->mem[pc];
    }

    return ptr;
}

__inline__ uint8_t *
zpmbnz(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    pc += sizeof(struct zpmop);
    if (!zpmzfset(vm)) {
        ptr = &vm->mem[dest];
    } else {
        ptr = &vm->mem[pc];
    }

    return ptr;
}

__inline__ uint8_t *
zpmblt(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    pc += sizeof(struct zpmop);
    if (!zpmofset(vm)) {
        ptr = &vm->mem[dest];
    } else {
        ptr = &vm->mem[pc];
    }

    return ptr;
}

__inline__ uint8_t *
zpmble(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    pc += sizeof(struct zpmop);
    if (!zpmofset(vm) || zpmzfset(vm)) {
        ptr = &vm->mem[dest];
    } else {
        ptr = &vm->mem[pc];
    }

    return ptr;
}

__inline__ uint8_t *
zpmbgt(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    pc += sizeof(struct zpmop);
    if (zpmofset(vm)) {
        ptr = &vm->mem[dest];
    } else {
        ptr = &vm->mem[pc];
    }

    return ptr;
}

__inline__ uint8_t *
zpmbge(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    pc += sizeof(struct zpmop);
    if (zpmofset(vm) || zpmzfset(vm)) {
        ptr = &vm->mem[dest];
    } else {
        ptr = &vm->mem[pc];
    }

    return ptr;
}

__inline__ uint8_t *
zpmbo(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    pc += sizeof(struct zpmop);
    if (zpmofset(vm)) {
        ptr = &vm->mem[dest];
    } else {
        ptr = &vm->mem[pc];
    }

    return ptr;
}

__inline__ uint8_t *
zpmbno(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    pc += sizeof(struct zpmop);
    if (!zpmofset(vm)) {
        ptr = &vm->mem[dest];
    } else {
        ptr = &vm->mem[pc];
    }

    return ptr;
}

__inline__ uint8_t *
zpmbc(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    pc += sizeof(struct zpmop);
    if (zpmcfset(vm)) {
        ptr = &vm->mem[dest];
    } else {
        ptr = &vm->mem[pc];
    }

    return ptr;
}

__inline__ uint8_t *
zpmbnc(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    struct zpmop *op = (struct zpmop *)ptr;
    zpmreg        dest = vm->genregs[op->reg1];

    pc += sizeof(struct zpmop);
    if (!zpmcfset(vm)) {
        ptr = &vm->mem[dest];
    } else {
        ptr = &vm->mem[pc];
    }

    return ptr;
}

__inline__ uint8_t *
zpmpop(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmpush(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmpusha(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmlda(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmsta(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmcall(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmenter(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmleave(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmret(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmthr(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmltb(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmldr(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmstr(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmrst(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmhlt(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmin(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

__inline__ uint8_t *
zpmout(struct zpm *vm, uint8_t *ptr, zpmureg pc)
{
    ;
}

