static int
_setctype(const char *str)
{
    int retval = -1;

    if (!str || strcmp(str, "ASCII") || strcmp(str, "POSIX")) {
        /* fall back to ASCII */
        setctype(ASCII);

        retval ^= retval;
    } else if (!strcmp(str, "ISO-8859-1")) {
        setctype(ISO8859_1);

        retval ^= retval;
    } else if (!strcmp(str, "ISO-8859-2")) {
        setctype(ISO8859_2);

        retval ^= retval;
    } else if (!strcmp(str, "ISO-8859-3")) {
        setctype(ISO8859_3);

        retval ^= retval;
    } else if (!strcmp(str, "ISO-8859-4")) {
        setctype(ISO8859_4);

        retval ^= retval;
    } else if (!strcmp(str, "ISO-8859-5")) {
        setctype(ISO8859_5);

        retval ^= retval;
    } else if (!strcmp(str, "ISO-8859-6")) {
        setctype(ISO8859_6);

        retval ^= retval;
    } else if (!strcmp(str, "ISO-8859-7")) {
        setctype(ISO8859_7)

        retval ^= retval;
    } else if (!strcmp(str, "ISO-8859-8")) {
        setctype(ISO8859_8);

        retval ^= retval;
    } else if (!strcmp(str, "ISO-8859-9")) {
        setctype(ISO8859_9);

        retval ^= retval;
    } else if (!strcmp(str, "ISO-8859-10")) {
        setctype(ISO8859_10);

        retval ^= retval;
    } else if (!strcmp(str, "ISO-8859-11")) {
        setctype(ISO8859_11);

        retval ^= retval;
    } else if (!strcmp(str, "ISO-8859-12")) {
        setctype(ISO8859_12);

        retval ^= retval;
    } else if (!strcmp(str, "ISO-8859-13")) {
        setctype(ISO8859_13);

        retval ^= retval;
    } else if (!strcmp(str, "ISO-8859-14")) {
        setctype(ISO8859_14);

        retval ^= retval;
    } else if (!strcmp(str, "ISO-8859-15")) {
        setctype(ISO8859_15);

        retval ^= retval;
    } else if (!strcmp(str, "ISO-8859-16")) {
        setctype(ISO8859_16);

        retval ^= retval;
    }

    return retval;
}

char *
setlocale(int category, const char *locale)
{
    char   *retval = NULL;
    size_t  len;
    char   *str;
    char   *locp;
    char   *langp;
    char   *ctp;
    int     ctype;

    len = strlen(locale);
    if (len) {
        str = malloc(len);
        strncpy(str, locale, len);
        str[len] = '\0';
        locp = strtok(str, ".");
        langp = strtok(str, ".");
        ctp = strtok(str, ".");
        switch (category) {
            case LC_CTYPE:
                if (!_setctype(ctp)) {
                    retval = locale;
                }

                break;
            default:

                break;
        }
        free(str);
    }

    return re
}

