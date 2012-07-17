AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4

AM_CFLAGS = @LIGHTNING_CFLAGS@ @LIB_CFLAGS@

INCLUDES = -I..

SUBDIRS = . Ecc

lib_LTLIBRARIES = libearthquake.la

libearthquake_la_SOURCES = \
	asm.c \
	cc.c \
	cpp.c \
	util.c \
	internal/icc.c \
	internal/icpp.c \
	internal/ierror.c \
	internal/ifile.c \
	internal/ihash.c \
	internal/imacro.c \
	internal/imath.c \
	internal/iopt.c \
	internal/itype.c \
	internal/iutil.c

libearthquake_la_LIBADD = \
	-lm

