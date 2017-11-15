#!/usr/bin/bash

dir=$(dirname $0)
scriptsdir=$(realpath $dir)
libdir="${scriptsdir}/../ggp/lib"
gendir="${scriptsdir}/../ggp/gcc/generated"
for f in type.hh type.cc variant.hh variant.cc util.hh; do
    "${scriptsdir}/gen-standalone-headers.pl" \
        "--include-input=ggp/lib/${f}" \
        "--input=${libdir}/${f}" \
        --out-components=ggp,gcc \
        --out-gen-components=ggp,gcc,generated \
        "--output=${gendir}/${f}" \
        --style=gcc
done
