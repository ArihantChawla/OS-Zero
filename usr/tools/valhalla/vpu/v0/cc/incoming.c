static struct ccmacro *
ccgetmacro(char *str, char **retstr)
{
    struct cctoken *token;
    struct ccmacro *macro = NULL;
    struct ccval   *val;

    if (!strncmp((char *)str, "#define", 7)) {
        str += 7;
        while ((*str) && isspace(*str)) {
            str++;
        }
        if ((*str) && (isalpha(*str) || *str == '_')) {
            ptr = str;
            str++;
            while ((*str) && (isalnum(*str) || *str == '_')) {
                str++;
            }
            while ((*str) && isspace(*str)) {
                str++;
            }
            if (*str == '(') {
                fprintf(stderr, "implement macros with arguments\n");

                exit(1);
            } else {
                *str++ = '\0';
                while ((*str) && isspace(*str)) {
                    str++;
                }
                while (*str) {
                    val = ccgetval(str, &str);
                    if (val) {
                        macro = malloc(sizeof(struct ccmacro));
                        if (!macro) {
                            fprintf(stderr, "cannot allocate macro\n");

                            exit(1);
                        }
                        macro->type = CC_CONST_MACRO;
                        macro->name = strdup((char *)ptr);
                        macro->namelen = strlen(ptr);
#if 0
                        macro->fname = name;
                        macro->fnamelen = strlen(name);
#endif
                        macro->val = val;
                        ccaddmacro(macro);
                    } else {

                    }
                } else {
                    fprintf(stderr, "invalid #define directive %s\n", ptr);

                    exit(1);
                }
            }
        }
    }

    return macro;
}

