#! /bin/sh

zero_abort() {
    echo "$zero_scriptname: $2"

    zero_exit "$1";
}

zero_error() {
    echo "$1"
    zero_cleanup

    return 0
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
    EXIT=$ZERO_TRAPEXIT
    HUP=$ZERO_SIGHUP
    INT=$ZERO_SIGINT
    PIPE=$ZERO_SIGPIPE
    TERM=$ZERO_SIGTERM

    # Other signals.
    QUIT=$ZERO_SIGQUIT
    ILL=$ZERO_SIGILL
    ABRT=$ZERO_SIGABRT
    FPE=$ZERO_SIGFPE
    KILL=$ZERO_SIGKILL
    SEGV=$ZERO_SIGSEGV
    ALRM=$ZERO_SIGALRM

    zero_trap "EXIT" 'zero_exit "$?"'
    zero_trap "HUP" 'zero_trap_sighup "$?"'
    zero_trap "INT" 'zero_trap_sigint "$?"'
    zero_trap "PIPE" 'zero_trap_sigpipe "$?"'
    zero_trap "TERM" 'zero_trap_sigterm "$?"'

    return 0
}

# usage
# -----
# zero_trap $trap $cmdlist
zero_trap() {
    trap="$1"
    cmdlist='$2'
    reason=""

    case $trap in
        EXIT)
            trap=$ZERO_TRAPEXIT
	    reason="EXIT"
            ;;
        HUP | SIGHUP)
            trap=$ZERO_SIGHUP
	    reason="SIGHUP"
            ;;
        INT | SIGINT)
            trap=$ZERO_SIGINT
	    reason="SIGINT"
            ;;
        QUIT | SIGQUIT)
            trap=$ZERO_SIGQUIT
	    reason="SIGQUIT"
            ;;
        ILL | SIGILL)
            trap=$ZERO_SIGILL
	    reason="EXIT"
            ;;
        ABRT | SIGABRT)
            trap=$ZERO_SIGABRT
	    reason="SIGABRT"
            ;;
        FPE | SIGFPE)
            trap=$ZERO_SIGFPE
	    reason="SIGFPE"
            ;;
        KILL | SIGKILL)
            trap=$ZERO_SIGKILL
	    reason="SIGKILL"
            ;;
        SEGV | SIGSEGV)
            trap=$ZERO_SIGSEGV
	    reason="SIGSEGV"
            ;;
        ALRM | SIGALRM)
            trap=$ZERO_SIGALRM
	    reason="SIGALRM"
            ;;
        PIPE | SIGPIPE)
            trap=$ZERO_SIGPIPE
	    reason="SIGPIPE"
            ;;
        TERM | SIGTERM)
            trap=$ZERO_SIGTERM
	    reason="SIGTERM"
            ;;
	*)
	    trap=-1
	    reason="$trap"
    esac
    case $trap in
	-1)
	    echo "$zero_scriptname: unknown trap $reason"
	    ;;
        0 | 1 | 2 | 13 | 15)
            ;;
        9 | 14 | $CHLD | $SIGCHLD)
            echo "$zero_scriptname: not all shells allow trapping of $reason"
            ;;
        *)
            echo "$zero_scriptname: it may not be safe/portable to trap $reason"
            ;;
    esac
    if [ "$trap" -eq -1 ]; then

	return $zero_errinval
    fi
    trap $@

    return 0
}

# zero_send_signal $sig [$pidspec] [$verbose]
zero_send_signal()
{
    sig="$1"
    pidspec="$$"
    opt=""

    if [ ! -z "$2" ]; then
	pidspec="$2"
    fi
    if [ ! -z "$3" ]; then
	opt="-v"
    fi

    echo -n "sending signal $sig to processes"
    kill $opt -s $sig $pidspec
}

# zero_signal_self $sig [$verbose]
zero_signal_self()
{
    sig="$1"
    opt=""

    pidspec="$$"
    if [ ! -z "$2" ]; then
	opt="-v"
    fi

    zero_send_signal $sig $pidspec $opt
}

# zero_signal_group $sig [$pidspec] [$verbose]
zero_signal_group()
{
    sig="$1"
    pidspec=""
    opt=""

    if [ -z "$2" ]; then
	pidspec=0
    elif [ "$2" = "self" ]; then
	pidspec="-$$"
    else
	pidspec="$2"
    fi
    if [ ! -z "$3" ]; then
	opt="-v"
    fi

    zero_send_signal $sig $pidspec $opt
}

zero_trap_sighup() {
    zero_exit "$ZERO_SIGHUP"
}

zero_trap_sigint() {
    zero_exit "$ZERO_SIGINT"
}

zero_trap_sigpipe() {
    zero_exit "$ZERO_SIGPIPE"
}

zero_trap_sigterm() {
    zero_exit "$ZERO_SIGTERM"
}

