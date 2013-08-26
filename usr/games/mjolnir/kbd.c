/* mjolgetch() etc. */

static struct evkbdqchar *kbdevq;

int
mjolgetch(void)
{
    int ch;

    if (!kbdevq) {
        kbdevq = evreg(EVKEYDOWNMASK, EVKEYASCII);
        if (!kbdevq) {
            fprintf(stderr, "cannot allocate keyboard event queue\n");

            exit(1);
        }
    }
    ch = evdeqkbdchar(kbdevq);
    while (!ch) {
        pause();
        ch = evdeqkbdchar(kbdevq);
    }

    return ch;
}

