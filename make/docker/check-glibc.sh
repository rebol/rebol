#!/bin/sh
set -eu

binary=${1:?Usage: check-glibc.sh BINARY}
versions=$(objdump -T "$binary" | sed -n 's/.*(GLIBC_\([0-9.]*\)).*/\1/p' | sort -Vu)
test -n "$versions"
maximum=$(printf '%s\n' "$versions" | tail -n 1)
printf 'Required glibc versions:\n%s\nMaximum required glibc: %s\n' "$versions" "$maximum"
test "$(printf '%s\n' "$maximum" 2.39 | sort -V | tail -n 1)" = 2.39
ldd "$binary"
