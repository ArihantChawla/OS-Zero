# Process this file with autoconf to produce a configure script.
 
PATH=$PATH:/usr/bin:/bin:/usr/local/bin:/usr/ccs/bin:/usr/contrib/bin:/usr/ucb/bin
export PATH

# Initialize Autoconf.
AC_INIT(util.h)
AC_PREREQ(2.50)

# Save configure command line options.
LIGHTNING_SAVE_OPTS([$@])

AC_CONFIG_MACRO_DIR([m4])

AC_CONFIG_HEADERS(autoconf.h)
#AC_CONFIG_FILES(Makefile kernel/Makefile kernel/lib/Makefile kernel/lib/kio/Makefile kernel/lib/kfs/Makefile kernel/lib/kdrv/Makefile kernel/lib/kdrv/bus/Makefile kernel/lib/kdrv/crypt/Makefile kernel/lib/kc/Makefile kernel/lib/kdrv/gfx/Makefile kernel/arch/Makefile kernel/arch/x86/Makefile kernel/arch/x86/boot/Makefile kernel/test/Makefile lib/Makefile Tornado/Makefile Tornado/crt0/Makefile Tornado/test/Makefile Earthquake/Makefile Earthquake/Ecc/Makefile lib/libsh/Makefile lsh/Makefile include/Makefile include/Earthquake/Makefile Thunder/Makefile include/Thunder/Makefile Thunder/test/Makefile Ldos/Makefile include/sys/Makefile include/Lightning/Makefile include/Lightning/arch/x86/Makefile include/Tornado/Makefile)
AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(Ecc/Makefile)

AC_CONFIG_AUX_DIR(config_aux)

# Set version information.
#LIGHTNING_SET_VERSION

# Initialize Automake.
AM_INIT_AUTOMAKE(Earthquake, 0.0.1)

# Initialize Lightning configuration.
LIGHTNING_INIT

# Initialize kernel configuration
#KERNEL_INIT

# Check for programs.
LIGHTNING_PROGS
AM_PROG_CC_C_O

# Set C compiler flags.
LIGHTNING_SET_COMPILER_FLAGS

# Check for libtool.
AC_PROG_LIBTOOL

# Check the sanity of what has been done so far.
AM_SANITY_CHECK

# Check for maintainer mode.
AM_MAINTAINER_MODE

# Check for types.
#LIGHTNING_TYPES

# Check for compiler characteristics.
#LIGHTNING_C

# Set install directories.
LIGHTNING_SET_INSTALL_DIRS

AC_OUTPUT

LIGHTNING_PRINT_CONFIGURE_INFO

