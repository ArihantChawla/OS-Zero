/*
 * iopt.c - internal option management for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#define FALSE 0
#define TRUE  1

#include <stdio.h>

#include <main.h>

static void __Eprtusage(void);
#if (__EARTHQUAKE_DEBUG_OPTS)
static void __Eprtopts(void);
#   define __EARTHQUAKE_PRINT_OPTS() __Eprtopts();
#else
#   define __EARTHQUAKE_PRINT_OPTS()
#endif

/*
 * <NOTE>
 * - when you add new options, remember to add them to this table as well.
 * </NOTE>
 */
static struct __Eoptdesc __Eoptdescs[] =
{
    { "lecc <options> <input files>", "- Execute LECC." },
    { "-E", "                          - Preprocess source files." },
    { "-S", "                          - Assemble source files." },
    { "-O", "                          - Optimize." },
    { "-ansi", "                       - Exclude non-ANSI features." },
    { "-c", "                          - Do not link." },
    { "-dump", "                       - Dump core after parsing input files." },
    { "-g", "                          - Compile with debugging information." },
    { "-nostdinc", "                   - Do not search default system header directories." },
    { "-nostdlib", "                   - Do not link to default libraries." },
    { "-o <outfile>", "                - Output file. REQUIRED." },
    { "-pg", "                         - Compile with profiling information." },
    { "-profile", "                    - Profile compiler." },
    { "-stats", "                      - Print compilation statistics." },
    { "-falign-functions=<value>", "   - Set function alignment. Must be power of two." },
    { "-falign-jumps", "               - Set jump alignment. Must be power of two."},
    { "-falign-labels", "              - Set label alignment. Must be power of two." },
    { "-falign-loops", "               - Set loop alignment. Must be power of two." },
    { "-falign-structs", "             - Set structure alignment. Must be power of two" },
    { "-falign-unions", "              - Set union alignment. Must be power of two" },
    { "-fbounds-check", "              - Enable bound checking." },
    { "-ffast-math", "                 - Enable mathematical optimizations." },
    { "-fforce-addr-reg", "            - Force memory addresses into registers." },
    { "-fforce-mem", "                 - Force memory operands into registers." },
    { "-fno-builtin", "                - Disable built-in functions" },
    { "-fno-inline", "                 - Disable function inlining." },
    { "-fomit-frame-pointer", "        - Omit frame pointer." },
    { "-foptimize-register-move", "    - reassign registers in move instructions." },
    { "-help", "                       - Show this information." },
    { "-m128bit-long-double", "        - Enable 128-bit long doubles." },
    { "-version", "                    - Display compiler version." },
    { NULL, NULL }
};

int
__Eparseopts(int argc, char *argv[])
{
    unsigned long ndx;
    unsigned long valnext;
    unsigned long *optptr;
    uint8_t *optstr;
    struct __Efile *newfile;
    int retval;
    uint8_t opt;

    ndx = 0;
    valnext = FALSE;
    retval = 0;
    opt = '\0';
    while (++ndx < argc) {
	optstr = argv[ndx];
	if (valnext) {
	    switch (opt) {
		case 'o':
		    if (__Erun.files.outfile) {
			fprintf(stderr,
				"%s: cannot specify several output files\n",
				argv[0]);

			return -1;
		    }
		    newfile = __Eallocfile(optstr, FALSE);
		    if (newfile == NULL) {

			return -1;
		    }
		    __Erun.files.outfile = newfile;

		    break;
		default:

		    break;
	    }
	    opt = '\0';
	    valnext = FALSE;
	} else if (!strcmp(optstr, "-E")) {
	    optptr = &__Erun.opts.output.preproc;
	    *optptr = TRUE;
	} else if (!strcmp(optstr, "-S")) {
	    optptr = &__Erun.opts.output.assemble;
	    *optptr = TRUE;
	} else if (!strcmp(optstr, "-O")) {
	    optptr = &__Erun.opts.optim.optimize;
	    *optptr = TRUE;
	} else if (!strcmp(optstr, "-ansi")) {
	    optptr = &__Erun.opts.input.ansi;
	    *optptr = TRUE;
	} else if (!strcmp(optstr, "-c")) {
	    optptr = &__Erun.opts.output.nolink;
	    *optptr = TRUE;
	} else if (!strcmp(optstr, "-dump")) {
	    optptr = &__Erun.opts.debug.dump;
	    *optptr = TRUE;
	} else if (!strcmp(optstr, "-g")) {
	    optptr = &__Erun.opts.output.debug;
	    *optptr = TRUE;
	} else if (!strcmp(optstr, "-nostdinc")) {
	    optptr = &__Erun.opts.input.nostdinc;
	    *optptr = TRUE;
	} else if (!strcmp(optstr, "-nostdlib")) {
	    optptr = &__Erun.opts.input.nostdlib;
	    *optptr = TRUE;
	} else if (!strcmp(optstr, "-o")) {
	    opt = 'o';
	    valnext = TRUE;
	} else if (!strcmp(optstr, "-pg")) {
	    optptr = &__Erun.opts.output.profile;
	    *optptr = TRUE;
	} else if (!strcmp(optstr, "-profile")) {
	    optptr = &__Erun.opts.debug.profile;
	    *optptr = TRUE;
	} else if (!strcmp(optstr, "-stats")) {
	    optptr = &__Erun.opts.debug.stats;
	    *optptr = TRUE;
	} else if (!strncmp(optstr, "-f", 2)) {
	    retval = __Eparsefopt(optstr, argv[0]);
	    if (retval < 0) {
		fprintf(stderr,	"%s: invalid option %s\n", argv[0], optstr);

		return -1;
	    }
	} else if (!strcmp(optstr, "-help")) {
	    __Eprtusage();
	    
	    return -1;
	} else if (!strncmp(optstr, "-m", 2)) {
	    retval = __Eparsemopt(optstr, argv[0]);
	    if (retval < 0) {
		fprintf(stderr, "%s: invalid option %s\n", argv[0], optstr);
	    }
	} else if (!strcmp(optstr, "-version")) {
	    ; /* do nothing. The option is parsed by the compiler. */
	} else if (!strncmp(optstr, "-", 1)) {
	    fprintf(stderr, "%s: invalid option %s\n", argv[0], optstr);

	    return -1;
	} else {
	    if (__Eaddinfile(optstr) < 0) {

		return -1;
	    }
	}
    }
    if (opt) {
	fprintf(stderr, "%s: unterminated option -%c\n", argv[0], opt);

	return -1;
    }
    if (__Erun.opts.output.preproc) {
	if (__Erun.opts.output.assemble) {
	    fprintf(stderr, "%s: cannot specify -E and -S simultaneously\n",
		    argv[0]);

	    return -1;
	}
    }

    __EARTHQUAKE_PRINT_OPTS();

    return 0;
}

int
__Eparsefopt(uint8_t *optstr, char *progname)
{
    unsigned long optval;
    unsigned long *optptr;
    int retval;

    if (!strncmp(optstr, "-falign-functions", 17)) {
	optptr = &__Erun.opts.optim.alignfuncs;
	optstr = strchr(optstr, '=');
	optstr++;
	optval = strtoul(optstr, NULL, 10);
	if ((optval == 0)
	    || !powerof2(optval)) {
	    fprintf(stderr,
		    "%s: invalid value for -falign-functions: %lu\n",
		    progname, optval);
	    
	    return -1;
	}
	*optptr = optval;
	retval = 0;
    } else if (!strncmp(optstr, "-falign-jumps", 13)) {
	optptr = &__Erun.opts.optim.alignjumps;
	optstr = strchr(optstr, '=');
	optstr++;
	optval = strtoul(optstr, NULL, 10);
	if ((optval == 0)
	    || !powerof2(optval)) {
	    fprintf(stderr,
		    "%s: invalid value for -falign-jumps: %lu\n",
		    progname, optval);
	    
	    return -1;
	}
	*optptr = optval;
	retval = 0;
    } else if (!strncmp(optstr, "-falign-labels", 14)) {
	optptr = &__Erun.opts.optim.alignlabels;
	optstr = strchr(optstr, '=');
	optstr++;
	optval = strtoul(optstr, NULL, 10);
	if ((optval == 0)
	    || !powerof2(optval)) {
	    fprintf(stderr,
		    "%s: invalid value for -falign-labels: %lu\n",
		    progname, optval);
	    
	    return -1;
	}
	*optptr = optval;
	retval = 0;
    } else if (!strncmp(optstr, "-falign-loops", 13)) {
	optptr = &__Erun.opts.optim.alignloops;
	optstr = strchr(optstr, '=');
	optstr++;
	optval = strtoul(optstr, NULL, 10);
	if ((optval == 0)
	    || !powerof2(optval)) {
	    fprintf(stderr,
		    "%s: invalid value for -falign-loops: %lu\n",
		    progname, optval);
	    
	    return -1;
	}
	*optptr = optval;
	retval = 0;
    } else if (!strncmp(optstr, "-falign-structs", 15)) {
	optptr = &__Erun.opts.optim.alignstructs;
	optstr = strchr(optstr, '=');
	optstr++;
	optval = strtoul(optstr, NULL, 10);
	if ((optval == 0)
	    || !powerof2(optval)) {
	    fprintf(stderr,
		    "%s: invalid value for -falign-structs: %lu\n",
		    progname, optval);
	    
	    return -1;
	}
	*optptr = optval;
	retval = 0;
    } else if (!strncmp(optstr, "-falign-unions", 14)) {
	optptr = &__Erun.opts.optim.alignunions;
	optstr = strchr(optstr, '=');
	optstr++;
	optval = strtoul(optstr, NULL, 10);
	if ((optval == 0)
	    || !powerof2(optval)) {
	    fprintf(stderr,
		    "%s: invalid value for -falign-unions: %lu\n",
		    progname, optval);
	    
	    return -1;
	}
	*optptr = optval;
	retval = 0;
    } else if (!strncmp(optstr, "-fbounds-check", 14)) {
	optptr = &__Erun.opts.output.chkbounds;
	*optptr = TRUE;
	retval = 0;
    } else if (!strncmp(optstr, "-ffast-math", 11)) {
	optptr = &__Erun.opts.optim.fastmath;
	*optptr = TRUE;
	retval = 0;
    } else if (!strncmp(optstr, "-fforce-addr-reg", 11)) {
	optptr = &__Erun.opts.optim.forceaddrreg;
	*optptr = TRUE;
	retval = 0;
    } else if (!strncmp(optstr, "-fforce-mem", 11)) {
	optptr = &__Erun.opts.optim.forcememreg;
	*optptr = TRUE;
	retval = 0;
    } else if (!strncmp(optstr, "-fno-builtin", 11)) {
	optptr = &__Erun.opts.input.nobuiltin;
	*optptr = TRUE;
	retval = 0;
    } else if (!strncmp(optstr, "-fno-inline", 11)) {
	optptr = &__Erun.opts.optim.noinline;
	*optptr = TRUE;
	retval = 0;
    } else if (!strncmp(optstr, "-fomit-frame-pointer", 11)) {
	optptr = &__Erun.opts.optim.omitfp;
	*optptr = TRUE;
	retval = 0;
    } else if (!strncmp(optstr, "-foptimize-register-move", 11)) {
	optptr = &__Erun.opts.optim.regmove;
	*optptr = TRUE;
	retval = 0;
    } else {
	retval = -1;
    }
    
    return retval;
}

int
__Eparsemopt(uint8_t *optstr, char *progname)
{
    unsigned long *optptr;
    int retval;

    retval = -1;
    if (!strncmp(optstr, "-m128bit-long-double", 20)) {
	optptr = &__Erun.opts.optim.longdbl128;
	*optptr = TRUE;
	retval = 0;
    }
    
    return retval;
}

static void
__Eprtusage(void)
{
    struct __Eoptdesc *desc;

    desc = &__Eoptdescs[0];
    while (desc->str) {
	fprintf(stderr, "%s %s\n", desc->str, desc->desc);
	desc++;
    }

    return;
}

#if (__EARTHQUAKE_DEBUG_OPTS)
static void
__Eprtopts(void)
{
    fprintf(stderr, "cc opts\n");
    fprintf(stderr, "-------\n");
    fprintf(stderr, "input:\n");
    fprintf(stderr, "ansi == %lu\n", __Erun.opts.input.ansi);
    fprintf(stderr, "nobuiltin == %lu\n", __Erun.opts.input.nobuiltin);
    fprintf(stderr, "nostdinc == %lu\n", __Erun.opts.input.nostdinc);
    fprintf(stderr, "nostdlib == %lu\n", __Erun.opts.input.nostdlib);
    fprintf(stderr, "output:\n");
    fprintf(stderr, "arch == %lu\n", __Erun.opts.output.arch);
    fprintf(stderr, "format == %lu\n", __Erun.opts.output.format);
    fprintf(stderr, "preproc == %lu\n", __Erun.opts.output.preproc);
    fprintf(stderr, "assemble == %lu\n", __Erun.opts.output.assemble);
    fprintf(stderr, "debug == %lu\n", __Erun.opts.output.debug);
    fprintf(stderr, "nolink == %lu\n", __Erun.opts.output.nolink);
    fprintf(stderr, "profile == %lu\n", __Erun.opts.output.profile);
    fprintf(stderr, "chkbounds == %lu\n", __Erun.opts.output.chkbounds);
    fprintf(stderr, "optim:\n");
    fprintf(stderr, "optimize == %lu\n", __Erun.opts.optim.optimize);
    fprintf(stderr, "alignfuncs == %lu\n", __Erun.opts.optim.alignfuncs);
    fprintf(stderr, "alignjumps == %lu\n", __Erun.opts.optim.alignjumps);
    fprintf(stderr, "alignlabels == %lu\n", __Erun.opts.optim.alignlabels);
    fprintf(stderr, "alignloops == %lu\n", __Erun.opts.optim.alignloops);
    fprintf(stderr, "alignstructs == %lu\n", __Erun.opts.optim.alignstructs);
    fprintf(stderr, "alignunions == %lu\n", __Erun.opts.optim.alignunions);
    fprintf(stderr, "fastmath == %lu\n", __Erun.opts.optim.fastmath);
    fprintf(stderr, "forceaddrreg == %lu\n", __Erun.opts.optim.forceaddrreg);
    fprintf(stderr, "forcememreg == %lu\n", __Erun.opts.optim.forcememreg);
    fprintf(stderr, "longdbl128 == %lu\n", __Erun.opts.optim.longdbl128);
    fprintf(stderr, "omitfp == %lu\n", __Erun.opts.optim.omitfp);
    fprintf(stderr, "regmove == %lu\n", __Erun.opts.optim.regmove);
    fprintf(stderr, "noinline == %lu\n", __Erun.opts.optim.noinline);
}
#endif

