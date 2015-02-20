long
zedinit(int argc, char *argv[])
{
    struct zedfile *ftab = calloc(8, sizeof(struct zedfile));

    zed.nfile = 8;
    zed.curfile = 0;
    zed.ftab = ftab;
}

