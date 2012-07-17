#
# gfx.m4 - GNU Autoconf graphics configuration for Lightning.
# Copyright (C) 2004 Tuomo Venäläinen
#
# See the file COPYING for information about using this software.

AC_DEFUN([KERNEL_INIT],
[
    KERNEL_SET_DRIVERS
])

AC_DEFUN([KERNEL_SET_DRIVERS],
[
    KERNEL_SET_GRAPHICS_DRIVER
])

AC_DEFUN([KERNEL_SET_GRAPHICS_DRIVER],
[
    KERNEL_GRAPHICS_DRIVER="tdfx"
    AC_SUBST([KERNEL_GRAPHICS_DRIVER])
])

