#!/bin/bash

set -e

compiler=''
plugin=''
input_file=''
output_file=''

parse_options() {
    local default_compiler="${compiler}"
    local default_plugin="${plugin}"
    local default_input_file="${input_file}"
    local default_output_file="${output_file}"

    while [[ -n "${1}" ]]; do
        case "${1}" in
            --compiler)
                compiler="${2}"
                shift 2
                ;;
            --help)
                cat <<EOF
Usage: $0 [FLAGS]
FLAGS:
--compiler <COMPILER> - GNU compiler to use for compiling the test source file with a plugin, default: ${default_compiler}
--help - prints this message and quits
--plugin <PLUGIN> - a path to the compiler plugin, default: ${default_plugin}
--type <TYPE> - a type of compiled file (either 'c' for C file, or 'cc' for C++ file), default: ${default_file_type}
EOF
                exit 0
                ;;
            --input-file)
                input_file="${2}"
                shift 2
                ;;
            --output-file)
                output_file="${2}"
                shift 2
                ;;
            --plugin)
                plugin="${2}"
                shift 2
                ;;
            --type)
                file_type="${2}"
                shift 2
                ;;
            *=*)
                echo "--foo=bar flags are not supported, use --foo bar"
                exit 1
                ;;
            *)
                echo "unknown flag ${1}, use --help to get help" >&2
                exit 1
                ;;
        esac
    done

    if [ -z "${compiler}" ]; then
        echo "Compiler not specified" >&2
        exit 1
    fi
    if [ -z "${plugin}" ]; then
        echo "Plugin not specified" >&2
        exit 1
    fi
    if [ -z "${input_file}" ]; then
        echo "Input file not specified" >&2
        exit 1
    fi
    if [ -z "${output_file}" ]; then
        echo "Output file not specified" >&2
        exit 1
    fi
}

parse_options "${@}"

input_file="${MESON_SOURCE_ROOT}/${MESON_SUBDIR}/${input_file}"
output_file="${MESON_BUILD_ROOT}/${MESON_SUBDIR}/${output_file}"

tmpdir=$(dirname "${output_file}")
mkdir -p "${tmpdir}"

"${compiler}" "-fplugin=${plugin}" -c -o "${output_file}" "${input_file}"
rm -f "${output_file}"
rmdir "${tmpdir}"
