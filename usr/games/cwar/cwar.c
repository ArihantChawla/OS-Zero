static struct cwarmach *
cwarinitmach(void)
{
    struct cwarmach *mach = calloc(1, sizeof(struct cwarmach));

    if (cwarmach)
}

static void
cwarloop(struct cwarmach *mach)
{
    while (!(mach->mem[(mach->eip) >> 2])) {
        cwardispinstr(mach);
    }

    exit(0);
}

