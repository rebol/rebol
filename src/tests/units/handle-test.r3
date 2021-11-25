Rebol [
	Title:   "Rebol3 handle test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %handle-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

;@@ https://github.com/Oldes/Rebol-issues/issues/1868

~~~start-file~~~ "handle!"

===start-group=== "context handles"

; create some context handles for testing
h1: rc4/key #{0000}
h2: rc4/key #{0000}
h3: aes/key #{00000000000000000000000000000000} none
h4: aes/key #{00000000000000000000000000000000} none

--test-- "same? handles"
	; handles are same if they have same type and also same data
	--assert h1  == h1
	--assert h1 !== h2
	--assert same? h1 h1
	--assert same? h4 h4
	--assert not same? h1 h2 
	--assert not same? h2 h1
	--assert not same? h1 h4
	--assert not same? h4 h1
	--assert not same? h3 h4
	--assert not same? h4 h3

--test-- "equal? handles"
	; handles are equal, if they have same type
	--assert h1 = h1
	--assert h1 = h2
	--assert equal? h1 h1
	--assert equal? h1 h2
	--assert equal? h2 h1
	--assert equal? h3 h3
	--assert equal? h3 h4
	--assert equal? h4 h3
	--assert not-equal? h1 h4
	--assert not-equal? h4 h1
	--assert not-equal? h2 h3
	--assert not-equal? h3 h2

--test-- "lesser? / greater? handles"
	--assert h3 < h1
	--assert h1 > h3
	--assert lesser?  h3 h1
	--assert greater? h1 h3

--test-- "sort/find handles"
	blk: reduce [h1 h3 h2 h4]
	--assert 1 = index? find blk h1
	--assert 2 = index? find blk h3
	--assert 3 = index? find blk h2
	--assert 4 = index? find blk h4
	--assert {[#[handle! aes] #[handle! aes] #[handle! rc4] #[handle! rc4]]} = mold try [sort blk]
	--assert {[#[handle! rc4] #[handle! rc4] #[handle! aes] #[handle! aes]]} = mold try [sort/reverse blk]

--test-- "handle as a key in map"
	m: #()
	--assert not error? try [m/(h1): 1]
	--assert not error? try [repend m [h2 2 h3 3]]
	--assert 1 = try [pick m h1]
	--assert 2 = try [m/(h2)]
	--assert 3 = try [select m h3]

--test-- "set operations with handles"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1765
	--assert "[#[handle! rc4] #[handle! aes]]" = mold unique     reduce [h1 h1 h3]
	--assert "[#[handle! rc4] #[handle! rc4]]" = mold difference reduce [h1 h3] reduce [h3 h2]
	--assert "[#[handle! rc4] #[handle! aes]]" = mold union      reduce [h1 h3] reduce [h3 h1]
	--assert                "[#[handle! aes]]" = mold intersect  reduce [h1 h3] reduce [h3 h2]
	--assert                "[#[handle! rc4]]" = mold exclude    reduce [h1 h3] reduce [h3 h2]

--test-- "query handle's type"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2465
	; short and easy way
	--assert h1/type = 'rc4
	--assert h3/type = 'aes
	; for consistency with other types (like date, image, etc..)
	--assert [type] = words-of h1
	--assert 'rc4 = query/mode h1 'type
	--assert all [object? o: query h1 o/type = 'rc4]

===end-group===

~~~end-file~~~