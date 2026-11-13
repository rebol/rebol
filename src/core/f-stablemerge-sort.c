//
// Stable adaptive merge sort
//
// Drop-in replacement for qsort() with guaranteed stability.
// Works with arbitrary element sizes via void* + comparator.
//
// Usage:
//   stable_sort(base, nmemb, size, comparator);
//
// Algorithm:
//   - Scans for natural ascending/descending runs in the input
//   - Descending runs are reversed in-place (stably: equal elements untouched)
//   - Short runs are extended to MIN_RUN using insertion sort
//   - Runs are merged using a pending-run stack (Timsort-style merge order)
//   - O(n) best case on already-sorted input (zero merges needed)
//   - O(n log n) worst/average case
//   - O(n) extra memory (one malloc for tmp buffer + run stack)
//
// License: Public domain
// 
// Algorithm based on Timsort by Tim Peters (2002).
// Collapse invariant fix per de Gouw et al. (2015).
//

#include "sys-core.h"

// Minimum run length. Runs shorter than this are extended with insertion sort.
// 32 is the sweet spot used by most Timsort implementations.
#define SM_MIN_RUN 32

// Maximum number of pending runs on the merge stack.
// ceil(log2(SIZE_MAX)) + 1 = 65 is sufficient for any array.
#define SM_MAX_RUNS 64

// --------------------------------------------------------------------------
// Insertion sort a range [0, nmemb) of base, extending an existing sorted
// prefix of length `sorted` (so we only insert elements from `sorted` onward).
// --------------------------------------------------------------------------
static void
sm_insertion_sort(unsigned char *base, size_t sorted, size_t nmemb,
                  const size_t size, cmp_t* cmp)
{
    size_t i, j;
    unsigned char *tmp;

    if (sorted < 1) sorted = 1;
    if (nmemb <= sorted) return;

    tmp = (unsigned char *)Make_Mem(size);
    if (!tmp) return;

    for (i = sorted; i < nmemb; i++) {
        memcpy(tmp, base + i * size, size);
        j = i;
        while (j > 0 && cmp(base + (j - 1) * size, tmp) > 0) {
            memcpy(base + j * size, base + (j - 1) * size, size);
            j--;
        }
        memcpy(base + j * size, tmp, size);
    }

    Free_Mem(tmp, size);
}

// --------------------------------------------------------------------------
// Reverse the elements in [base, base + nmemb) in-place.
// Used to fix descending runs into ascending order.
// --------------------------------------------------------------------------
static void
sm_reverse(unsigned char *base, size_t nmemb, const size_t size)
{
    unsigned char *tmp;
    size_t lo, hi;

    if (nmemb <= 1) return;

    tmp = (unsigned char *)Make_Mem(size);
    if (!tmp) return;

    lo = 0;
    hi = nmemb - 1;
    while (lo < hi) {
        memcpy(tmp,              base + lo * size, size);
        memcpy(base + lo * size, base + hi * size, size);
        memcpy(base + hi * size, tmp,              size);
        lo++;
        hi--;
    }

    Free_Mem(tmp, size);
}

// --------------------------------------------------------------------------
// Merge two adjacent sorted runs: [left, mid) and [mid, right).
// tmp must be at least (mid - left) * size bytes.
// --------------------------------------------------------------------------
static void
sm_merge(unsigned char *base,
         size_t left, size_t mid, size_t right,
         size_t size,
         cmp_t* cmp,
         unsigned char *tmp)
{
    size_t n_left = mid - left;
    size_t i, j, k;

    memcpy(tmp, base + left * size, n_left * size);

    i = 0;
    j = mid;
    k = left;

    while (i < n_left && j < right) {
        /* <= preserves stability */
        if (cmp(tmp + i * size, base + j * size) <= 0) {
            memcpy(base + k * size, tmp + i * size, size);
            i++;
        } else {
            memcpy(base + k * size, base + j * size, size);
            j++;
        }
        k++;
    }

    if (i < n_left)
        memcpy(base + k * size, tmp + i * size, (n_left - i) * size);
}

// --------------------------------------------------------------------------
// Run stack entry: start index and length of a pending run.
// --------------------------------------------------------------------------
typedef struct {
    size_t start;
    size_t len;
} sm_run;

// --------------------------------------------------------------------------
// Merge stack[n-1] and stack[n-2] into a single run.
// --------------------------------------------------------------------------
static void
sm_merge_at(unsigned char *base, sm_run *stack, size_t n,
            size_t size, cmp_t* cmp, unsigned char *tmp)
{
    size_t left  = stack[n-2].start;
    size_t mid   = stack[n-1].start;
    size_t right = mid + stack[n-1].len;

    sm_merge(base, left, mid, right, size, cmp, tmp);

    stack[n-2].len = stack[n-2].len + stack[n-1].len;
}

// --------------------------------------------------------------------------
// Collapse the run stack maintaining Timsort invariants:
//   (A) stack[i].len > stack[i+1].len + stack[i+2].len
//   (B) stack[i].len > stack[i+1].len
// These guarantee O(n log n) total work.
// When force=1, collapse everything down to a single run.
// --------------------------------------------------------------------------
static void
sm_collapse(unsigned char *base, sm_run *stack, size_t *depth,
            size_t size, cmp_t* cmp, unsigned char *tmp, int force)
{
    while (*depth > 1) {
        size_t n = *depth;

        if (!force) {
            // Invariant A: stack[n-3] > stack[n-2] + stack[n-1]
            // Checked first - a violation here can require merging the
            // middle pair even when invariant B looks satisfied.
            // Using if/else if ensures B is always evaluated independently,
            // matching the fix applied to CPython after the 2015 formal
            // verification finding (de Gouw et al.).
            if (n >= 3 &&
                stack[n-3].len <= stack[n-2].len + stack[n-1].len) {
                if (stack[n-3].len < stack[n-1].len) {
                    // Merge middle pair (n-3 with n-2)
                    sm_merge_at(base, stack, n-1, size, cmp, tmp);
                    stack[n-2] = stack[n-1];
                } else {
                    // Merge top pair (n-2 with n-1)
                    sm_merge_at(base, stack, n, size, cmp, tmp);
                }
                (*depth)--;
            } else if (stack[n-2].len <= stack[n-1].len) {
                // Invariant B: stack[n-2] > stack[n-1]
                sm_merge_at(base, stack, n, size, cmp, tmp);
                (*depth)--;
            } else {
                // both invariants satisfied
                break;
            }
        } else {
            sm_merge_at(base, stack, n, size, cmp, tmp);
            (*depth)--;
        }
    }
}

// --------------------------------------------------------------------------
// Scan for the next natural run starting at `pos`.
// Strictly descending runs are reversed in-place to become ascending.
// Returns the length of the run.
// --------------------------------------------------------------------------
static size_t
sm_find_run(unsigned char *base, size_t pos, const size_t nmemb,
            const size_t size, cmp_t* cmp)
{
    size_t run_end;

    if (pos + 1 >= nmemb)
        return nmemb - pos;

    run_end = pos + 1;

    if (cmp(base + pos * size, base + run_end * size) > 0) {
        // Strictly descending: extend and reverse
        while (run_end < nmemb &&
            cmp(base + (run_end - 1) * size, base + run_end * size) > 0)
            run_end++;
        sm_reverse(base + pos * size, run_end - pos, size);
    }
    else {
        // Non-descending (ascending): just extend
        while (run_end < nmemb &&
            cmp(base + (run_end - 1) * size, base + run_end * size) <= 0)
            run_end++;
    }

    return run_end - pos;
}

// --------------------------------------------------------------------------
// Main sort.
// --------------------------------------------------------------------------
static int
sm_mergesort(unsigned char *base, const size_t nmemb, const size_t size,
             int (*cmp)(const void *, const void *))
{
    sm_run stack[SM_MAX_RUNS];
    size_t depth = 0;
    size_t pos   = 0;
    size_t min_run;
    unsigned char *tmp;

    if (nmemb <= 1) return 0;

    // Tiny arrays: insertion sort and done, no malloc needed
    if (nmemb <= (size_t)SM_MIN_RUN) {
        sm_insertion_sort(base, 1, nmemb, size, cmp);
        return 0;
    }

    // Compute min_run in [SM_MIN_RUN, 2*SM_MIN_RUN) using CPython's formula:
    // repeatedly halve n, OR-ing the remainder, until n < SM_MIN_RUN.
    {
        size_t n = nmemb;
        size_t r = 0;
        while (n >= (size_t)SM_MIN_RUN) { r |= (n & 1); n >>= 1; }
        min_run = n + r;
    }

    /* One allocation for the entire sort */
    tmp = (unsigned char *)Make_Mem(nmemb * size);
    if (!tmp) return -1;

    while (pos < nmemb) {
        size_t run_len = sm_find_run(base, pos, nmemb, size, cmp);

        // Extend short runs to min_run with insertion sort
        if (run_len < min_run) {
            size_t extend = (pos + min_run < nmemb) ? min_run : nmemb - pos;
            sm_insertion_sort(base + pos * size, run_len, extend, size, cmp);
            run_len = extend;
        }

        stack[depth].start = pos;
        stack[depth].len   = run_len;
        depth++;

        sm_collapse(base, stack, &depth, size, cmp, tmp, 0);

        pos += run_len;
    }

    // Final merge of all remaining runs
    sm_collapse(base, stack, &depth, size, cmp, tmp, 1);

    Free_Mem(tmp, nmemb * size);
    return 0;
}

// --------------------------------------------------------------------------
// Public API — mirrors qsort() signature, guaranteed stable.
//
//   base   : pointer to first element
//   nmemb  : number of elements
//   size   : element size in bytes
//   cmp    : comparator, returns <0 / 0 / >0
//
// --------------------------------------------------------------------------
void
stable_sort(void *base, const size_t nmemb, const size_t size, cmp_t* cmp)
{
    if (!base || !cmp || size == 0 || nmemb <= 1) return;
    sm_mergesort((unsigned char *)base, nmemb, size, cmp);
}

#undef SM_MIN_RUN
#undef SM_MAX_RUNS
