#!/bin/sh
set -eu

# The existing runner prints assertion failures without returning nonzero.
# Preserve its output and reject failures or incomplete runs explicitly.
log=/tmp/rebol-tests.log
build/rebol3-bulk-linux-x64 -s src/tests/run-tests.r3 > "$log" 2>&1 || {
    cat "$log"
    exit 1
}
cat "$log"
grep -q 'Finished.*All tests' "$log"
if grep -Eq 'Number of Assertions Failed: *[1-9]|FAILED units files:|TEST FAILURES' "$log"; then
    exit 1
fi
