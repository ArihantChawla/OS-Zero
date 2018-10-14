#! /bin/sh

zero_abort() {
    echo "$zero_scriptname: $2"

    zero_exit "$1";
}

zero_error() {
    echo "$1"
    zero_cleanup

    return 0;
}

# System V Release 4.2 shell is said not to allow trapping of SIGSEGV, SIGALRM,
# or SIGCHLD.
zero_init_traps() {
    zero_erracces=1
    zero_errcmd=2
    zero_errexist=3
    zero_errftype=4
    zero_errinval=5
    zero_errio=6
    zero_errisdir=7
    zero_errloop=8
    zero_errnocmd=9
    zero_errnodev=10
    zero_errnoent=11
    zero_errnoexec=12
    zero_errnotblk=13
    zero_errnotdir=14
    zero_errnotempty=15
    zero_errnotsock=16
    zero_errnotty=17
    zero_errpipe=18

    # The signals that are known to be safe to trap.
    zero_TRAPEXIT=0
    zero_SIGHUP=1
    zero_SIGINT=2
    zero_SIGPIPE=13
    zero_SIGTERM=15

    # Other signals.
    zero_SIGQUIT=3
    zero_SIGILL=4
    zero_SIGABRT=6
    zero_SIGFPE=8
    zero_SIGKILL=9
    zero_SIGSEGV=11
    zero_SIGALRM=14

    zero_trap "$zero_TRAPEXIT" 'zero_exit "$?"'
    zero_trap "$zero_SIGHUP" 'zero_trap_sighup "$?"'
    zero_trap "$zero_SIGINT" 'zero_trap_sigint "$?"'
    zero_trap "$zero_SIGPIPE" 'zero_trap_sigpipe "$?"'
    zero_trap "$zero_SIGTERM" 'zero_trap_sigterm "$?"'

    return 0;
}

zero_trap() {
    signal="$1"
    commands='$2'

    case $signal in
        EXIT)
            signal=0
            ;;
        HUP | SIGHUP)
            signal=1
            ;;
        INT | SIGINT)
            signal=2
            ;;
        QUIT | SIGQUIT)
            signal=3
            ;;
        ILL | SIGILL)
            signal=4
            ;;
        ABRT | SIGABRT)
            signal=6
            ;;
        FPE | SIGFPE)
            signal=8
            ;;
        KILL | SIGKILL)
            signal=9
            ;;
        SEGV | SIGSEGV)
            signal=11
            ;;
        ALRM | SIGALRM)
            signal=14
            ;;
        PIPE | SIGPIPE)
            signal=13
            ;;
        TERM | SIGTERM)
            signal=15
            ;;
    esac

    case $signal in
        0 | 1 | 2 | 13 | 15)
            ;;
        9 | 14 | CHLD | SIGCHLD)
            echo "$zero_scriptname: not all shells allow trapping of $signal"
            ;;
        *)
            echo "$zero_scriptname: it may not be safe/portable to trap $signal"
            ;;
    esac

    trap "$@"

    return 0;
}

zero_trap_sighup() {
    zero_exit "$zero_SIGHUP";
}

zero_trap_sigint() {
    zero_exit "$zero_SIGINT";
}

zero_trap_sigpipe() {
    zero_exit "$zero_SIGPIPE";
}

zero_trap_sigterm() {
    zero_exit "$zero_SIGTERM";
}

