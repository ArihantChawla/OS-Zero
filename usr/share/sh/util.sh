#! /bin/sh

zero_message() {
    echo "$zero_scriptname: $@"

    return 0;
}

zero_var_name() {
    shell_varname=`echo $1 | sed 'y,*+,pp,;s,[^_$zero_alnum],_,g'`

    return 0;
}

zero_cpp_name() {
    cpp_varname=`echo $1 | sed 'y,*$zero_lowercase_letters,P$zero_uppercase_letters,;s,[^_$zero_alnum],_,g'`

    return 0;
}

