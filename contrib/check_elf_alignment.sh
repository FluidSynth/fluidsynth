#!/bin/bash

# source: https://developer.android.com/guide/practices/page-sizes
# https://cs.android.com/android/platform/superproject/main/+/main:system/extras/tools/check_elf_alignment.sh
#
progname="${0##*/}"
progname="${progname%.sh}"

# usage: check_elf_alignment.sh [path to *.so files|path to *.apk]

cleanup_trap() {
  local status=$?
  if [ -n "${tmp}" -a -d "${tmp}" ]; then
    rm -rf "${tmp}"
  fi
  exit "$status"
}

usage() {
  echo "Host side script to check the ELF alignment of shared libraries."
  echo "Shared libraries are reported ALIGNED when their ELF regions are"
  echo "16 KB or 64 KB aligned. Otherwise they are reported as UNALIGNED."
  echo
  echo "Usage: ${progname} [input-path|input-APK|input-APEX]"
  echo "Set OBJDUMP to select an ELF reader (for example, the NDK llvm-objdump)."
}

if [ ${#} -ne 1 ]; then
  usage
  exit
fi

case ${1} in
  --help | -h | -\?)
    usage
    exit
    ;;

  *)
    dir="${1}"
    ;;
esac

if ! [ -f "${dir}" -o -d "${dir}" ]; then
  echo "Invalid file: ${dir}" >&2
  exit 1
fi

if [[ "${dir}" == *.apk ]]; then
  trap 'cleanup_trap' EXIT

  echo
  echo "Recursively analyzing $dir"
  echo

  if { zipalign --help 2>&1 | grep -q "\-P <pagesize_kb>"; }; then
    echo "=== APK zip-alignment ==="
    zipalign -v -c -P 16 4 "${dir}" | egrep 'lib/arm64-v8a|lib/x86_64|Verification'
    echo "========================="
  else
    echo "NOTICE: Zip alignment check requires build-tools version 35.0.0-rc3 or higher."
    echo "  You can install the latest build-tools by running the below command"
    echo "  and updating your \$PATH:"
    echo
    echo "    sdkmanager \"build-tools;35.0.0-rc3\""
  fi

  dir_filename=$(basename "${dir}")
  tmp=$(mktemp -d -t "${dir_filename%.apk}_out_XXXXX")
  unzip "${dir}" lib/* -d "${tmp}" >/dev/null 2>&1
  dir="${tmp}"
fi

if [[ "${dir}" == *.apex ]]; then
  trap 'cleanup_trap' EXIT

  echo
  echo "Recursively analyzing $dir"
  echo

  dir_filename=$(basename "${dir}")
  tmp=$(mktemp -d -t "${dir_filename%.apex}_out_XXXXX")
  deapexer extract "${dir}" "${tmp}" || { echo "Failed to deapex." && exit 1; }
  dir="${tmp}"
fi

RED="\e[31m"
GREEN="\e[32m"
ENDCOLOR="\e[0m"

unaligned_libs=()

echo
echo "=== ELF alignment ==="

matches="$(find "${dir}" -type f)"
IFS=$'\n'
for match in $matches; do
  # We could recursively call this script or rewrite it to though.
  [[ "${match}" == *".apk" ]] && echo "WARNING: doesn't recursively inspect .apk file: ${match}"
  [[ "${match}" == *".apex" ]] && echo "WARNING: doesn't recursively inspect .apex file: ${match}"

  [[ $(file "${match}") == *"ELF"* ]] || continue

  if ! headers="$("${OBJDUMP:-objdump}" -p "${match}")"; then
    echo "Failed to read ELF headers: ${match}" >&2
    exit 1
  fi
  res="$(printf '%s\n' "$headers" | awk '$1 == "LOAD" { print $NF }')"
  res_inline=${res//$'\n'/ }
  # Every LOAD segment must be aligned, not only the first one.
  if [[ -n "$res" ]] && ! grep -qvE '^2\*\*(1[4-9]|[2-9][0-9]|[1-9][0-9]{2,})$' <<< "$res"; then
    echo -e "${match}: ${GREEN}ALIGNED${ENDCOLOR} (${res_inline})"
  else
    echo -e "${match}: ${RED}UNALIGNED${ENDCOLOR} (${res_inline})"
    unaligned_libs+=("${match}")
  fi
done

if [ ${#unaligned_libs[@]} -gt 0 ]; then
  echo -e "${RED}Found ${#unaligned_libs[@]} unaligned libs (only arm64-v8a/x86_64 libs need to be aligned).${ENDCOLOR}"
  exit -1
elif [ -n "${dir_filename}" ]; then
  echo -e "ELF Verification Successful"
  exit 0
fi
echo "====================="
