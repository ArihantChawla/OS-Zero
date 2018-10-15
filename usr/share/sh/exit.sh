#! /bin/sh

# exit-handling routines

# usage
# -----
# zero_exit $arg
#
# function
# --------
# call all exit handlers in order of addition with the argument $1;
# finally, terminate the program with the exit status of $1
zero_exit()
{
    arg="$1"
    func=""

    if [ -n "$zero_exit_handlers" ]; then
        for func in $zero_exit_handlers
        do
	    $func $arg
        done
    fi

    exit "$arg"
}

# usage
# -----
# zero_add_exit_handler $func
#
# function
# --------
# adds exit-handler func to the internal list zero_exit_handlers
zero_add_exit_handler()
{
    func="$1"

    if [ -n "$func" ]; then
        if [ -z "$zero_exit_handlers" ]; then
            zero_exit_handlers="$func"
        else
            zero_exit_handlers="$zero_exit_handlers $func"
        fi
    fi

    return 0
}

# usage
# -----
# zero_remove_exit_handler $func
#
# function
# --------
# remove exit-handler from internal list
zero_remove_exit_handler()
{
    func="$1"
    list=

    if [ -n "$func" ] && [ -n "$zero_exit_handlers" ]; then
        for exithandler in $zero_exit_handlers
        do
            if [ "$exithandler" = "$func" ]; then
                :
            elif [ -z "$list" ]; then
                list="$exithandler"
            else
                list="$list $exithandler"
            fi
        done
        zero_exit_handlers="$list"
    fi

    return 0
}

