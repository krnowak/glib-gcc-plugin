#!/usr/bin/bash

dir=$(dirname $0)
scriptsdir=$(realpath $dir)
libdir="${scriptsdir}/../ggp/lib"
gendir="${scriptsdir}/../ggp/test/generated"
for f in type.hh type.cc variant.hh variant.cc util.hh; do
    "${scriptsdir}/gen-standalone-headers.pl" \
        "--include-input=ggp/lib/${f}" \
        "--input=${libdir}/${f}" \
        --out-components=ggp,test \
        --out-gen-components=ggp,test,generated \
        "--output=${gendir}/${f}" \
        --style=stl
done
