/* mjolgetch() etc. */

static struct evkbdqchar *kbdevq;

int
mjolgetch(void)
{
    int ch;

    if (!kbdevq) {
        kbdevq = evreg(EVKEYDOWNMASK, EVKEY8BIT);
        if (!kbdevq) {
            fprintf(stderr, "cannot allocate keyboard event queue\n");

            exit(1);
        }
    }
    ch = evdeqkbdchar(kbdevq);
    do {
        pause();
        ch = evdeqkbdchar(kbdevq);
    } while (!ch);

    return ch;
}

