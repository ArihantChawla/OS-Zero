#include <v0/conf.h>
#include <vas/conf.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <zero/fastudiv.h>
#include <v0/types.h>
#include <v0/mach.h>
#include <v0/vm.h>
#include <v0/io.h>
#include <v0/op.h>
#include <vas/types.h>

extern void   vasinit(void);
extern long * vasgetinst(char *str);

extern char            *vaslinebuf;
struct v0              *v0vm;
#if defined(V0_GAME)
static long long        v0speedcnt;
#endif
#if defined(V0_DEBUG_TABS)
static struct v0opinfo  v0opinfotab[V0_NINST_MAX];
#endif
char                   *v0opnametab[V0_NINST_MAX];

void
v0printop(struct v0op *op)
{
  int val = op->code;

  fprintf(stderr, "code\t%x - unit == %x, inst == %x\n",
	  val, v0getunit(val), v0getinst(val));

  return;
}

void
v0initseg(struct v0 *vm, v0memadr base, size_t npage, v0memflg flg)
{
  v0memflg *bits = vm->membits;
  size_t    ndx = base / V0_PAGE_SIZE;

  /* text, rodata, data, bss, stk */
  for (ndx = 0 ; ndx < npage ; ndx++) {
    bits[ndx] = flg;
  }

  return;
}

void
v0initio(struct v0 *vm)
{
  struct v0iofuncs *vec = vm->iovec;
  char             *vtd = vm->vtdpath;
  FILE             *fp;

  if (!vtd) {
    vtd = strdup(V0_VTD_PATH);
    if (!vtd) {
      fprintf(stderr, "V0: failed to duplicate VTD-path\n");

      exit(1);
    }
  }
  fp = fopen(vtd, "a+");
  if (!fp) {
    perror("V0: failed to open VTD-file");

    exit(errno);
  }
  vec[V0_STDIN_PORT].rdfunc = v0readkbd;
  vec[V0_STDOUT_PORT].wrfunc = v0writetty;
  vec[V0_STDERR_PORT].wrfunc = v0writetty;
  vec[V0_RTC_PORT].rdfunc = v0readrtc;
  vec[V0_TMR_PORT].rdfunc = v0readtmr;

  return;
}

struct v0 *
v0init(struct v0 *vm)
{
  void   *mem = calloc(1, V0_MEM_SIZE);
  void   *ptr;
  long    newvm = 0;
  size_t  vmnpg = V0_MEM_SIZE / V0_PAGE_SIZE;

  if (!mem) {

    return NULL;
  }
  ptr = calloc(V0_NIOPORT_MAX, sizeof(struct v0iofuncs));
  if (ptr) {
    if (!vm) {
      vm = malloc(sizeof(struct v0));
      if (!vm) {
	free(mem);
	free(ptr);

	return NULL;
      }
      newvm = 1;
    }
    memset(vm, 0, sizeof(struct v0));
    vm->iovec = ptr;
    ptr = calloc(65536, sizeof(struct divuf16));
    if (!ptr) {
      free(mem);
      free(vm->iovec);
      if (newvm) {
	free(vm);
      }

      return NULL;
    }
    fastuf16divuf16gentab(ptr, 0xffff);
    vm->divu16tab = ptr;
    ptr = calloc(vmnpg, sizeof(v0memflg));
    if (!ptr) {
      free(mem);
      free(vm->iovec);
      free(vm->divu16tab);
      if (newvm) {
	free(vm);
      }
    }
    vm->mem = mem;
    vm->membits = ptr;
    v0initseg(vm, V0_PAGE_SIZE, vmnpg,
	      V0_MEM_PRESENT | V0_MEM_READ | V0_MEM_WRITE | V0_MEM_EXEC);
    v0initio(vm);
    vm->regs[V0_FP_REG] = 0x00000000;
    vm->regs[V0_SP_REG] = V0_MEM_SIZE;
  }
  v0vm = vm;

  return vm;
}

int
v0loop(struct v0 *vm)
{
  static _V0OPTAB_T  jmptab[V0_NINST_MAX];
  v0ureg             pc = vm->regs[V0_PC_REG];
  struct v0op       *op = (struct v0op *)&vm->mem[V0_TEXT_ADR];
  v0reg              code;
  v0reg              msw;

  v0initops(jmptab);

#if defined(__GNUC__)
  do {
      v0reg code = op->code;

      goto *jmptab[code];

      {
          v0opnop:
              pc = v0nop(vm, op);

              opjmp(vm, pc);
          v0opinc:
              pc = v0inc(vm, op);

              opjmp(vm, pc);
          v0opdec:
              pc = v0dec(vm, op);

              opjmp(vm, pc);
          v0opcmp:
              pc = v0cmp(vm, op);

              opjmp(vm, pc);
          v0opadd:
              pc = v0add(vm, op);

              opjmp(vm, pc);
          v0opadc:
              pc = v0adc(vm, op);

              opjmp(vm, pc);
          v0opsub:
              pc = v0sub(vm, op);

              opjmp(vm, pc);
          v0opsbb:
              pc = v0sbb(vm, op);

              opjmp(vm, pc);
          v0opshl:
              pc = v0shl(vm, op);

              opjmp(vm, pc);
          v0opsal:
              pc = v0sal(vm, op);

              opjmp(vm, pc);
          v0opshr:
              pc = v0shr(vm, op);

              opjmp(vm, pc);
          v0opsar:
              pc = v0sar(vm, op);

              opjmp(vm, pc);
          v0opnot:
              pc = v0not(vm, op);

              opjmp(vm, pc);
          v0opand:
              pc = v0and(vm, op);

              opjmp(vm, pc);
          v0opxor:
              pc = v0xor(vm, op);

              opjmp(vm, pc);
          v0oplor:
              pc = v0lor(vm, op);

              opjmp(vm, pc);
          v0opcrp:
              pc = v0crp(vm, op);

              opjmp(vm, pc);
          v0opmul:
              pc = v0mul(vm, op);

              opjmp(vm, pc);
          v0opmuh:
              pc = v0muh(vm, op);

              opjmp(vm, pc);
          v0opldr:
              pc = v0ldr(vm, op);

              opjmp(vm, pc);
          v0opstr:
              pc = v0str(vm, op);

              opjmp(vm, pc);
          v0oppsh:
              pc = v0psh(vm, op);

              opjmp(vm, pc);
          v0oppsm:
              pc = v0psm(vm, op);

              opjmp(vm, pc);
          v0oppop:
              pc = v0pop(vm, op);

              opjmp(vm, pc);
          v0oppom:
              pc = v0pom(vm, op);

              opjmp(vm, pc);
          v0opjmp:
              pc = v0jmp(vm, op);

              opjmp(vm, pc);
        v0opjmr:
              pc = v0jmr(vm, op);

              opjmp(vm, pc);
      }
  } while (1);

#else /* !defined(__GNUC__) */

  while (v0opisvalid(vm, op)) {
    struct v0op *op = v0adrtoptr(vm, pc);
    uint8_t      code = op->code;
    v0opfunc    *func = jmptab[code];

    pc = func(vm, pc);
  }

#endif

  return EXIT_SUCCESS;
}

void
v0getopt(struct v0 *vm, int argc, char *argv[])
{
  return;
}

int
main(int argc, char *argv[])
{
  struct v0 *vm = v0init(NULL);
  vasmemadr  adr = V0_TEXT_ADR;
  int        ret = EXIT_FAILURE;
  long       ndx;

  if (vm) {
    v0getopt(vm, argc, argv);
    vasinit();
    for (ndx = 1 ; ndx < argc ; ndx++) {
      vasreadfile(argv[1], adr);
      vastranslate(adr);
      vasresolve(adr);
      vasfreesyms();
      if (!vm->regs[V0_PC_REG]) {
	vm->regs[V0_PC_REG] = V0_TEXT_ADR;
      }
      ret = v0loop(vm);
    }

    exit(ret);
  }

  exit(0);
}

