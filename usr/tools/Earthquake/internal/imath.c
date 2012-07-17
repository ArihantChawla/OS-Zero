/*
 * imath.c - mathematical routines for Earthquake C Environment.
 * Copyright (C) 2004 Tuomo Venäläinen
 *
 * See the file COPYING for information about using this software.
 */

#define FALSE 0
#define TRUE  1

#include <zero/param.h>

#include <main.h>

int
__Eatoi(const char *str)
{
    unsigned long minus;
    int retval;
    int ch;

    minus = FALSE;
    retval = 0;
    ch = *str++;
    if (ch == '-') {
	minus = TRUE;
	ch = *str++;
    }
    while ((ch) && __Eisdigit(ch)) {
	retval *= 10;
	retval += ch - '0';
	ch = *str++;
    }
    if (minus) {
	retval = -retval;
    }

    return retval;
}

long
__Eatol(const char *str)
{
    unsigned long minus;
    long retval;
    int ch;

    minus = FALSE;
    retval = 0;
    ch = *str++;
    if (ch == '-') {
	minus = TRUE;
	ch = *str++;
    }
    while ((ch) && __Eisdigit(ch)) {
	retval *= 10;
	retval += ch - '0';
	ch = *str++;
    }
    if (minus) {
	retval = -retval;
    }

    return retval;
}

long long
__Eatoll(const char *str)
{
    long long retval;
    unsigned long minus;
    int ch;

    minus = FALSE;
    retval = 0;
    ch = *str++;
    if (ch == '-') {
	minus = TRUE;
	ch = *str++;
    }
    while ((ch) && __Eisdigit(ch)) {
	retval *= 10;
	retval += ch - '0';
	ch = *str++;
    }
    if (minus) {
	retval = -retval;
    }

    return retval;
}

double
__Eatof(const char *str)
{
    double retval;
    double frac;
    unsigned long ul;
    unsigned long div;
    unsigned long minus;
    int ch;

    minus = FALSE;
    ul = 0;
    ch = *str;
    if (ch == '-') {
	minus = TRUE;
	ch = *++str;
    }
    while ((ch) && __Eisdigit(ch)) {
	str++;
	ul *= 10;
	ul += ch - '0';
	ch = *str;
    }
    retval = (double)ul;
    if (ch == '.') {
	div = 10;
	ch = *++str;
	while ((ch) && __Eisdigit(ch)) {
	    frac = (ch - '0') / div;
	    retval += frac;
	    ch = *str++;
	}
    }
    if (minus) {
	retval = -retval;
    }

    return retval;
}

long double
__Eatold(const char *str)
{
    unsigned long long ull;
    unsigned long long div;
    unsigned long minus;
    long double retval;
    long double frac;
    int ch;

    minus = FALSE;
    ull = 0;
    ch = *str;
    if (ch == '-') {
	minus = TRUE;
	ch = *++str;
    }
    while ((ch) && __Eisdigit(ch)) {
	str++;
	ull *= 10;
	ull += ch - '0';
	ch = *str;
    }
    retval = (long double)ull;
    if (ch == '.') {
	div = 10;
	ch = *++str;
	while ((ch) && __Eisdigit(ch)) {
	    frac = (ch - '0') / div;
	    retval += frac;
	    ch = *str++;
	}
    }
    if (minus) {
	retval = -retval;
    }

    return retval;
}

long
__Eatow(const char *str)
{
    unsigned long minus;
    long retval;
    int ch;

    minus = FALSE;
    retval = 0;
    ch = *str++;
    if (ch == '-') {
	minus = TRUE;
	ch = *str++;
    }
    while ((ch) && __Eisdigit(ch)) {
	retval *= 10;
	retval += ch - '0';
	ch = *str++;
    }
    if (minus) {
	retval = -retval;
    }

    return retval;
}

unsigned int
__Eatoui(const char *str)
{
    unsigned int retval;
    int ch;

    retval = 0;
    ch = *str++;
    while ((ch) && __Eisdigit(ch)) {
	retval *= 10;
	retval += ch - '0';
	ch = *str++;
    }

    return retval;
}

unsigned long
__Eatoul(const char *str)
{
    unsigned long retval;
    int ch;

    retval = 0;
    ch = *str++;
    while ((ch) && __Eisdigit(ch)) {
	retval *= 10;
	retval += ch - '0';
	ch = *str++;
    }

    return retval;
}

unsigned long long
__Eatoull(const char *str)
{
    unsigned long long retval;
    int ch;

    retval = 0;
    ch = *str++;
    while ((ch) && __Eisdigit(ch)) {
	retval *= 10;
	retval += ch - '0';
	ch = *str++;
    }

    return retval;
}

unsigned long
__Eatouw(const char *str)
{
    unsigned long retval;
    int ch;

    retval = 0;
    ch = *str++;
    while ((ch) && __Eisdigit(ch)) {
	retval *= 10;
	retval += ch - '0';
	ch = *str++;
    }

    return retval;
}

long
__Ehibit(long val)
{
    unsigned long mask;
    long retval;
    long shift;

    if (val <= 0) {

	return -1;
    }

    mask = 1;
    retval = -1;
    for (shift = 0 ; shift < 8 * LONGSIZE - 2 ; shift++) {
	if (val & mask) {
	    retval = shift;
	}
	mask <<= 1;
    }

    return retval;
}

long
__Elobit(long val)
{
    unsigned long mask;
    long shift;

    if (val <= 0) {

	return -1;
    }

    mask = 1;
    for (shift = 0 ; shift < 8 * LONGSIZE - 2 ; shift++) {
	if (val & mask) {

	    return shift;
	}
	mask <<= 1;
    }

    /* NOTREACHED */
    return -1;
}

long
__Ehibitu(unsigned long val)
{
    unsigned long shift;
    unsigned long mask;
    long retval;

    if (val == 0) {

	return -1;
    }

    mask = 1;
    retval = -1;
    for (shift = 0 ; shift < 8 * LONGSIZE - 1 ; shift++) {
	if (val & mask) {
	    retval = shift;
	}
	mask <<= 1;
    }

    return retval;
}

long
__Elobitu(unsigned long val)
{
    unsigned long shift;
    unsigned long mask;

    if (val == 0) {

	return -1;
    }

    mask = 1;
    for (shift = 0 ; shift < 8 * LONGSIZE - 1 ; shift++) {
	if (val & mask) {

	    return shift;
	}
	mask <<= 1;
    }

    /* NOTREACHED */
    return -1;
}

long
__Emaxshift(long val)
{
    long hibit;
    long retval;

    if (val == 0) {

	return -1;
    }

    hibit = __Ehibit(val);
    retval = (LONGSIZE << 3) - hibit - 2;

    return retval;
}

long
__Emaxshiftu(unsigned long val)
{
    long hibit;
    long retval;

    if (val == 0) {

	return -1;
    }

    hibit = __Ehibitu(val);
    retval = (LONGSIZE << 3) - hibit - 1;

    return retval;
}

