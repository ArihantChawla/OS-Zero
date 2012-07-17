#
# init.m4 - GNU Autoconf initialization for Lightning.
# Copyright (C) 2004 Tuomo Venäläinen
#
# See the file COPYING for information about using this software.

# Save configure command-line options.
#
# NOTE: this needs to be called early in configure to work. Call it right after
# AC_INIT.
#
AC_DEFUN([LIGHTNING_SAVE_OPTS],
[
    saveopts=`echo $1`

    if test -n "$saveopts" ; then
        if test -z "$lightning_configure_opts" ; then
            lightning_configure_opts="$saveopts"
        else
            lightning_configure_opts="$lightning_configure_opts $saveopts"
        fi
    fi
])

# LIGHTNING_INIT([OPTIONS])
#
# Initialization function for Lightning configure script.
#
AC_DEFUN([LIGHTNING_INIT],
[
    # Parse command line options.
    LIGHTNING_PARSE_OPTS

    # Set shell variables.
    LIGHTNING_SET_VARS

    # Set package versions.
    LIGHTNING_SET_VERSION
    TORNADO_SET_VERSION
    EARTHQUAKE_SET_VERSION
    ECC_SET_VERSION
    THUNDER_SET_VERSION

    # Define C macros.
    LIGHTNING_SET_DEFINES

    # Create templates for macros defined in include/config.h.
    LIGHTNING_SET_TEMPLATES

    # Initialize configuration with default values.
    LIGHTNING_SET_DEFAULTS

    # Check for system type.
    LIGHTNING_CHECK_SYSTEM

    # Set system-specific options.
    # LIGHTNING_CHECK_SYSTEM_OPTS
])

