#! /bin/sh

# processing of command-line options

zero_lastarg=""

# debugging routine
debug_printopt()
{
    opt=""
    long=0
    narg=0

    echo "OPTIONS"
    echo "-------"
    for opt in $_OPTS
    do
	eval val='$_OPT_'$opt
	if [ -z "$val" ]; then
	    eval val='$_LONG_OPT_'$opt
	    if [ ! -z "$val" ]; then
		long=1
	    fi
	fi
	eval narg='$_HAVE_ARG_'$opt
	if [ -z "$narg" ]; then
	    eval narg='$_HAVE_LONG_ARG_'$opt
	fi
	if [ "$narg" -eq 1 ]; then
	    echo $opt" == $val"
	else
	    echo $opt
	fi
    done
}

# another debugging routine
debug_printarg()
{
    arg=""

    echo "ARGUMENTS"
    echo "---------"
    for arg in $_ARGS
    do
	echo " $arg"
    done
}

#
# register option for program
#
# usage
# -----
# zero_regopt opt long takeval
#
# arguments
# ---------
# - opt - name of the option string (without hyphen-prefixes)
# - long is true (non-zero) if this is an option with two hyphens, e.g. --foo
#   (which can also be used like --foo=value)
# - non-zero value for takeval means the option takes an argument value
#
# function
# --------
# - for each short option opt we declare
#   _HAVE_ARG_opt holding the optional value for the option
# - for each long option lopt we declare
#   _HAVE_ARG_lopt holding the optional value for the option#
zero_regopt()
{
    opt=`echo $1 | sed 's,^\([-]*\)\(.*\)\$,\2,'`
    long=$2

    if [ "$long" -eq 0 ]; then
	if [ $# -eq 2 ]; then
	    eval '_HAVE_ARG_'$opt="0"
	else
	    takeval=$3
	    if [ -z "$takeval" || "$takeval" -eq 0 ]; then
		eval '_HAVE_ARG_'$opt1=0
	    else
		eval '_HAVE_ARG_'$opt=1
	    fi
	fi
    elif [ $# -eq 2 ]; then
	eval '_HAVE_LONG_ARG_'$opt=0
    else
	takeval=$3
	if [ -z "$takeval" ] || [ "$takeval" -eq 0 ]; then
	    eval '_HAVE_LONG_ARG_'$opt=0
	else
	    eval '_HAVE_LONG_ARG_'$opt=1
	fi
    fi
}

#
# set option value, either to "true" or the provided value if present
#
# usage
# -----
# zero_setopt opt long [val]
#
# arguments
# ---------
# - opt - name of the option string without hyphens in front
# - if long is non-zero, option is of style --opt value or --opt=value
# - if supplied, the value val will be assigned to opt (default "true")
#
# function
# --------
# - for all short options we declare _OPT_opt with boolean value "true" or
#   a supplied value
# - for all long options we declare _LONG_OPT_opt with the value of "true" or
#   a supplied value
#
# TODO
# ----
# callback-support to manage in-program parameters
zero_setopt()
{
    opt=$1
    long=$2
    narg=0

    opt=`echo $opt | sed 's,^\([-]*\)\(.*\)\$,\2,'`
    if [ "$long" -eq 0 ]; then
	eval narg='$_HAVE_ARG_'$opt
    else
	eval narg='$_HAVE_LONG_ARG_'$opt
    fi
    if [ -z "$narg" ]; then
	echo -n "invalid option $_OPT_"$opt
    elif [ "$narg" -eq 1 ]; then
	if [ "$long" -eq 1 ]; then
	    if [ $# -eq 3 ]; then
		val=$3
		eval '_LONG_OPT_'$opt=$val
	    else
		eval '_LONG_OPT_'$opt="true"
	    fi
	else
	    if [ $# -eq 3 ]; then
		val=$3
		eval '_OPT_'$opt=$val
	    else
		eval '_OPT_'$opt="true"
	    fi
	fi
    fi
}

#
# add non-option argument to the current list; set zero_lastarg to the last one
# supplied on the command line
# - this could be used for, say, mv or cp with the last argument of a directory
#
# usage
# -----
# zero_addarg $arg
#
# arguments
# ---------
# - $1 - option string (may contain hyphens in front)
zero_addarg()
{
    arg=$1

    if [ -z "$_ARGS" ]; then
	_ARGS="$arg"
    else
	_ARGS=$_ARGS" $arg"
    fi
    zero_lastarg="$arg"
}

#
# parse and assign command-line options
#
# usage
# -----
# zero_parseopt $@
#
# function
# --------
# - we support the following types of options
#   - boolean: "true" (default value) or "false"
#   - short: -V, -V $val, -V=$val
#   - long: --opt, --opt $val, --opt=$val
# - argument "--" alone notifies the rest of the options are literal arguments
#
zero_parseopt()
{
    narg=0
    argsonly=0
    arg=""
    opt=""
    val=""
    long=0

    for arg
    do
	case $arg in
	    --*)
		val=""
		long=1
		if [ "$narg" -eq 1 ]; then
		    zero_setopt $opt 1 $arg
		elif [ "$argsonly" -eq 1 ]; then
		    zero_addarg $arg
		else
		    arg=`echo $arg | sed 's,^\([-]*\)\(.*\)\$,\2,'`
		    eval narg='$_HAVE_LONG_ARG_'$arg
		    if [ -z "$nargs" ]; then
			opt=`echo $arg | sed 's,^\(.*\)\(\=\)\(.*\)\$,\1,'`
			val=`echo $arg | sed 's,^\(.*\)\(\=\)\(.*\)\$,\3,'`
			eval narg='$_HAVE_LONG_ARG_'$opt
			if [ ! -z "$val" ]; then
			    if [ "$opt" != "$val" ]; then
				if [ "$narg" -eq 1 ]; then
				    zero_setopt $opt 1 $val
				    narg=0
				    long=0
				else
				    zero_setopt $opt 1
				fi
			    else
				if [ "$narg" -eq 0 ]; then
				    zero_setopt $opt 1
				fi
			    fi
			fi
		    else
			if [ "$nargs" -eq 1 ]; then
			    opt="$arg"
			else
			    zero_setopt $opt 1
			fi
		    fi
		fi
		if [ -z "$_OPTS" ]; then
		    _OPTS="$opt"
		else
		    _OPTS=$_OPTS" $opt"
		fi
		;;
	    -*)
		long=0
		if [ "$argsonly" -eq 1 ]; then
		    if [ -z "$_ARGS" ]; then
			_ARGS="$arg"
		    else
			_ARGS=$_ARGS" $arg"
		    fi
		else
		    opt=`echo $arg | sed 's,^\([-]*\)\(.*\)\$,\2,'`
		    if [ "$narg" -eq 1 ]; then
			zero_setopt $opt 0 $arg
			narg=0
			opt=""
		    else
			eval narg='$_HAVE_ARG_'$opt
			if [ ! -z "$narg" ]; then
			    if [ "$narg" -eq 0 ]; then
				zero_setopt $opt 0
			    fi
			    if [ -z "$_OPTS" ]; then
				_OPTS="$opt"
			    else
				_OPTS=$_OPTS" $opt"
			    fi
			else
			    echo "invalid option $opt"

			    exit 1
			fi
		    fi
		fi
		;;
	    *)
		if [ "$narg" -eq 1 ]; then
		    if [ "$long" -eq 1 ]; then
			zero_setopt $opt 1 $arg
		    else
			zero_setopt $opt 0 $arg
		    fi
		    narg=0
		    opt=""
		else
		    zero_addarg $arg
		fi
		;;
	    --)
		argsonly=1
		;;
	esac
    done
}

# test routine
test()
{
    zero_regopt "--foo" 1 1
    zero_regopt "--bar" 1 0
    zero_regopt "--xyzzy" 1
    zero_parseopt $@
    debug_printopt
    debug_printarg
}

test $@
