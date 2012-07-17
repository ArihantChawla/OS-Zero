#
# defaults.m4 - GNU Autoconf defaults.
# Copyright (C) 2004 Tuomo Venäläinen
#
# See the file COPYING for information about using this software.

# Substituted variables
# ---------------------
#
# LIGHTNING_BUILD_DATE			Date and time of last build.
# LIGHTNING_AUTHORS			List of Lightning authors.
# LIGHTNING_DISTRIBUTION_ROOT		Source top-level directory.
#
# LIGHTNING_HEADER_PATH			Paths to be searched for header files.
# LIGHTNING_LIBRARY_PATH		Paths to be searched for libraries.
#

# Set Lightning macros and variables.
#
AC_DEFUN([LIGHTNING_SET_DEFAULTS],
[
    # Set some basic variables
    DATE="`date '+%d %B %Y'`"
    AC_SUBST([DATE])
    LIGHTNING_BUILD_DATE="`date`"
    AC_DEFINE_UNQUOTED([LIGHTNING_BUILD_DATE], ["$LIGHTNING_BUILD_DATE"])

    LIGHTNING_AUTHORS="Vendu"
    AC_DEFINE_UNQUOTED([LIGHTNING_AUTHORS], ["$LIGHTNING_AUTHORS"])

    # Check for the distribution root directory
    LIGHTNING_DISTRIBUTION_ROOT=`pwd`
    AC_SUBST([LIGHTNING_DISTRIBUTION_ROOT])

    # Directories to be added to the header path
    LIGHTNING_HEADER_PATH="-I$LIGHTNING_DISTRIBUTION_ROOT/include -I/usr/local/include"
    AC_SUBST([LIGHTNING_HEADER_PATH])

    # Directories to be added to the library path
    LIGHTNING_LIBRARY_PATH="-L/usr/local/lib"
    AC_SUBST([LIGHTNING_LIBRARY_PATH])

    # Default headers.
    AM_CPPFLAGS="$LIGHTNING_HEADER_PATH $AM_CPPFLAGS"
    AC_SUBST([AM_CPPFLAGS])

    # Default libraries.
    LIBS="$LIGHTNING_LIBRARY_PATH $LIBS"
    AC_SUBST([LIBS])
])

