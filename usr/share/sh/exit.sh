#! /bin/sh

zero_exit() {
    if test -n "$zero_exit_handlers" ; then
        for handler in $zero_exit_handlers
        do
            $handler "$1"
        done
        zero_exit_handlers=
    fi

    (exit "$1")
    exit "$1";
}

zero_add_exit_handler() {
    handler="$1"

    if test -n "$handler" ; then
        if test -z "$zero_exit_handlers" ; then
            zero_exit_handlers="$handler"
        else
            zero_exit_handlers="$zero_exit_handlers $handler"
        fi
    fi

    return 0;
}

zero_remove_exit_handler() {
    handler="$1"
    exithandlers=

    if test -n "$handler" && test -n "$zero_exit_handlers" ; then
        for exithandler in $zero_exit_handlers
        do
            if test "$exithandler" = "$handler" ; then
                :
            elif test -z "$exithandlers" ; then
                exithandlers="$exithandler"
            else
                exithandlers="$exithandlers $exithandler"
            fi
        done
        zero_exit_handlers="$exithandlers"
    fi

    return 0;
}

