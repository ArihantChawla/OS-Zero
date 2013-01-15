void
kinit(void)
{
    vminit((uint64_t *)&_pagetab);
    __asm__ __volatile__ ("sti\n");
}

