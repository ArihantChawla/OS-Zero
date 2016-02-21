size_t      symnhashbit;
struct sym *symhash;

struct sym *
zpfinitsymhash(size_t nhashbit)
{
    struct sym *hash = NULL;

    hash = calloc(1 << nhashbit, sizeof(struct sym *));
    if (!hash) {

        exit(ZPF_NOMEM);
    }
    symhash = hash;
    symnnhashbit = nhashbit;

    return hash;
}

void
zpfhashsym(struct sym *sym)
{
    
}

