#!/bin/sh
set -eu

target=test
case "${1-}" in
    '') ;;
    --build-only) target=build ;;
    *) echo 'Usage: sh make/docker/build.sh [--build-only]' >&2; exit 2 ;;
esac

# Run from any directory; the build context is always this checkout.
root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$root"
image=rebol3-ubuntu24:local
docker build --platform linux/amd64 --target "$target" -t "$image" -f make/docker/Dockerfile .
container=$(docker create "$image" /bin/true)
trap 'docker rm "$container" >/dev/null' EXIT
mkdir -p build/ubuntu24
docker cp "$container:/src/build/rebol3-bulk-linux-x64" build/ubuntu24/
printf 'Binary exported to %s/build/ubuntu24/rebol3-bulk-linux-x64\n' "$root"
