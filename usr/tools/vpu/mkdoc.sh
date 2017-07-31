#! /bin/sh

if test -z `which pdflatex` ; then
    echo "pdflatex not found"

    exit 1
fi

pdflatex -shell-escape zpm.tex

