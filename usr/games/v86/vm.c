#include <stddef.h>
#include <v86/v86.h>
#include <v86/vm.h>

static __inline__ long
vmdov86op(struct v86vm *vm, struct v86op *op)
{
    long    code = 0;
    long    retval = -1L;
    v86reg *dptr = NULL;
    v86reg  dest;
    v86reg  src;
    v86reg  pc;

    
    do {
        if (code) {
          __v86not:
            _v86opnot(vm, op, retval);
            
            return retval;
          __v86and:
            _v86opand(vm, op, retval);
            
            return retval;
          __v86or:
            _v86opor(vm, op, retval);
            
            return retval;
          __v86xor:
            _v86opxor(vm, op, retval);
            
            return retval;
          __v86shl:
            _v86opshl(vm, op, retval);

            return retval;
          __v86shr:
            _v86opshr(vm, op, retval);
            
            return retval;
          __v86sar:
            _v86opsar(vm, op, retval);
            
            return retval;
          __v86add:
            _v86opadd(vm, op, retval);
            
            return retval;
          __v86sub:
            _v86opsub(vm, op, retval);
            
            return retval;
          __v86cmp:
            _v86opcmp(vm, op, retval);
            
            return retval;
          __v86mul:
            _v86opmul(vm, op, retval);
            
            return retval;
          __v86div:
            _v86opdiv(vm, op, retval);
            
            return retval;
          __v86jmp:
            _v86opjmp(vm, op, retval);
            
            return retval;
          __v86ljmp:
            _v86opljmp(vm, op, retval);

            return retval;
          __v86jz:
            _v86opjz(vm, op, retval);
            
            return retval;
          __v86jnz:
            _v86opjnz(vm, op, retval);
            
            return retval;
          __v86jc:
            _v86opjc(vm, op, retval);
            
            return retval;
          __v86jnc:
            _v86opjnc(vm, op, retval);

            return retval;
          __v86jo:
            _v86opjo(vm, op, retval);
            
            return retval;
          __v86jno:
            _v86opjno(vm, op, retval);
            
            return retval;
          __v86jle:
            _v86opjle(vm, op, retval);
            
            return retval;
          __v86jgt:
            _v86opjgt(vm, op, retval);
            
            return retval;
          __v86jge:
            _v86opjge(vm, op, retval);
            
            return retval;
          __v86call:
            _v86opcall(vm, op, retval);

            return retval;
          __v86ret:
            _v86opret(vm, op, retval);

            return retval;
          __v86ldr:
            _v86opldr(vm, op, retval);

            return retval;
          __v86str:
            _v86opstr(vm, op, retval);

            return retval;
          __v86push:
            _v86oppush(vm, op, retval);

            return retval;
          __v86pusha:
            _v86oppusha(vm, op, retval);

            return retval;
          __v86pop:
            _v86oppop(vm, op, retval);

            return retval;
          __v86popa:
            _v86oppopa(vm, op, retval);

            return retval;
          __v86in:
            _v86opin(vm, op, retval);

            return retval;
          __v86out:
            _v86opnout(vm, op, retval);

            return retval;
          __v86hlt:
            _v86ophlt(vm, op, retval);

            return retval;
        }
        do {
            void *jmptab[V86_OPERATIONS] =
            {
                NULL,
                &&__v86not,
                &&__v86and,
                &&__v86or,
                &&__v86xor,
                &&__v86shl,
                &&__v86shr,
                &&__v86sar,
                &&__v86add,
                &&__v86sub,
                &&__v86cmp,
                &&__v86mul,
                &&__v86div,
                &&__v86jmp,
                &&__v86ljmp,
                &&__v86jz,
                &&__v86jnz,
                &&__v86jc,
                &&__v86jnc,
                &&__v86jo,
                &&__v86jno,
                &&__v86jle,
                &&__v86jgt,
                &&__v86jge,
                &&__v86call,
                &&__v86ret,
                &&__v86ldr,
                &&__v86str,
                &&__v86push,
                &&__v86pusha,
                &&__v86pop,
                &&__v86popa,
                &&__v86in,
                &&__v86out,
                &&__v86hlt
            };
            code = op->code;

            goto *jmptab[code];
        } while (code);
    } while (0);
    
    return retval;
}

int
main(int argc, char *argv[])
{
    
}

