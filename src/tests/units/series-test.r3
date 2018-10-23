Rebol [
	Title:   "Rebol series test script"
	Author:  "Oldes"
	File: 	 %series-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Series"

===start-group=== "FIND & SELECT"

--test-- "SELECT or FIND NONE! anything == none - #473"
	--assert none? find none 1
	--assert none? select none 1

--test-- "FIND"
	--assert none? find/part [x] 'x 0
	--assert equal? [x] find/part [x] 'x 1
	--assert equal? [x] find/reverse tail [x] 'x
	--assert equal? [y] find/match [x y] 'x
	--assert equal? [x] find/last [x] 'x
	--assert equal? [x] find/last [x x x] 'x

--test-- https://github.com/rebol/rebol-issues/issues/66
	--assert none? find/skip [1 2 3 4 5 6] 2 3

--test-- https://github.com/rebol/rebol-issues/issues/88
	--assert "c" = find "abc" charset ["c"]
	--assert none? find/part "ab" "b" 1

--test-- https://github.com/rebol/rebol-issues/issues/2324
	str: "1.1.1"
	--assert "1.1.1" == find/part str "1." 2
	str: skip str 2
	--assert "1.1" == find str "1."
	--assert "1.1" == find/part str "1." 2

--test-- "SELECT"
	--assert 2 = select/part [1 2 1 3 1 2] 1 2
	--assert none? select/part [1 2 1 3 1 2] 2 2
	--assert 3 = select/part (skip [1 2 1 3 1 2] 2) 1 2


===end-group===

~~~end-file~~~