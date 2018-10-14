#! /bin/sh

zero_lastarg=""

zero_printopt()
{
    opt=""
    long=0
    narg=""

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
	if [ ! -z "$narg" ]; then
	    if [ $narg -eq 1 ]; then
		echo $opt" == $val"
	    else
		echo $opt
	    fi
	fi
    done
}

zero_printarg()
{
    arg=""

    echo "ARGUMENTS"
    echo "---------"
    for arg in $_ARGS
    do
	echo " $arg"
    done
}

# zero_regopt opt long takeval
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
    else
	if [ $# -eq 2 ]; then
	    eval '_HAVE_LONG_ARG_'$opt=0
	else
	    takeval=$3
	    if [ -z "$takeval" ] || [ "$takeval" -eq 0 ]; then
		eval '_HAVE_LONG_ARG_'$opt=0
	    else
		eval '_HAVE_LONG_ARG_'$opt=1
	    fi
	fi
    fi
}

# zero_setopt opt long [val]
zero_setopt()
{
    opt=$1
    long=$2
    narg=0

    eval narg='$_HAVE_ARG_'$opt
    if [ -z "$narg" ]; then
	eval narg='$_HAVE_LONG_ARG_'$opt
    fi
    if [ -z "$narg" ]; then
	echo -n "invalid option $_OPT_"$opt
    else
	if [ "$narg" -eq 1 ]; then
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
    fi
}

zero_addarg()
{
    if [ -z "$_ARGS" ]; then
	_ARGS="$arg"
    else
	_ARGS=$_ARGS" $arg"
    fi
    zero_lastarg="$arg"
}

zero_parseopt()
{
    narg=0
    argsonly=0
    opt=""
    arg=""

    for arg
    do
	case $arg in
	    --*)
		if [ "$argsonly" -eq 1 ]; then
		    if [ -z "$_ARGS" ]; then
			_ARGS="$arg"
		    else
			_ARGS=$_ARGS" $arg"
		    fi
		else
		    opt=`echo $arg | sed 's,^\([-]*\)\(.*\)\$,\2,'`
		    if [ "$narg" -eq 1 ]; then
			zero_setopt $opt 1 $arg
			narg=0
			opt=""
		    else
			eval narg='$_HAVE_LONG_ARG_'$opt
			if [ "$narg" -eq 0 ]; then
			    zero_setopt $opt 1
			fi
			if [ -z "$_OPTS" ]; then
			    _OPTS="$opt"
			else
			    _OPTS=$_OPTS" $opt"
			fi
		    fi
		fi
		long=1
		;;
	    -*)
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
		long=0
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

test()
{
    zero_regopt "--foo" 1 1
    zero_regopt "--bar" 1 0
    zero_regopt "--xyzzy" 1
    zero_parseopt $@
    zero_printopt
    zero_printarg
}

test $@
