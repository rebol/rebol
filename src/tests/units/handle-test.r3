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

;; Two registered handle types are needed, with two live handles of each.
;; The GUI extension provides them: GUI-WINDOW and GUI-WIDGET. It is used
;; instead of `rc4` and `aes`, which are no longer built in.
;;
;; The windows are opened HIDDEN - nothing here needs to be seen, and a test
;; run must not steal focus or need a display it may not have.
;;
;; Do this test only if the extension is available and can open a window!
if all [
	not error? try [
		;; a locally built extension can be pointed at without installing
		;; it - the same two lines the GUI extension's own test.r3 uses
		if modules-dir: get-env 'REBOL_MODULES_DIR [
			system/options/modules: dirize to-rebol-file modules-dir
		]
		try [system/modules/gui: none] ;; make sure it is loaded fresh
		gui: import 'gui
	]
	not error? try [
		w1: open-window/hidden 200x100
		w2: open-window/hidden 200x100
		;; h1 and h2 are GUI-WIDGET, h3 and h4 are GUI-WINDOW
		h1: add-button w1 "A" 10x10 80x24
		h2: add-button w1 "B" 10x40 80x24
		h3: w1
		h4: w2
	]
][
--test-- "same? handles"
	;; handles are same if they refer to the same handle
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
	;; handles are equal only if they are the SAME handle - having the same
	;; type is not enough. `h1/type = h2/type` is the type question.
	--assert h1 = h1
	--assert not h1 = h2
	--assert equal? h1 h1
	--assert not equal? h1 h2
	--assert not equal? h2 h1
	--assert equal? h3 h3
	--assert not equal? h3 h4
	--assert not equal? h4 h3
	--assert not-equal? h1 h2
	--assert not-equal? h1 h4
	--assert not-equal? h4 h1
	--assert not-equal? h2 h3
	--assert not-equal? h3 h2
	;; ... and equality does not depend on WHEN the value was taken. Each
	;; one carries a copy of the handle's flags, which include the bit the
	;; collector sets and clears.
	h1-again: h1
	recycle
	--assert h1 = h1-again
	--assert h1 == h1-again
	--assert same? h1 h1-again

--test-- "lesser? / greater? handles"
	;; different types order by type NAME: GUI-WIDGET before GUI-WINDOW
	--assert h1 < h3
	--assert h3 > h1
	--assert lesser?  h1 h3
	--assert greater? h3 h1
	;; whatever order two handles of ONE type end up in, it is antisymmetric
	--assert either h1 < h2 [h2 > h1][h2 < h1]
	--assert either h3 < h4 [h4 > h3][h4 < h3]
	--assert h1 >= h1
	--assert h1 <= h1

--test-- "sort/find handles"
	blk: reduce [h1 h3 h2 h4]
	--assert 1 = index? find blk h1
	--assert 2 = index? find blk h3
	--assert 3 = index? find blk h2
	--assert 4 = index? find blk h4
	;; A handle molds with the address it holds, so the sorted block cannot
	;; be compared as text - its TYPES can, and that is what sorting by type
	;; name is for.
	types: func [b [block!]][collect [foreach h b [keep h/type]]]
	--assert [GUI-WIDGET GUI-WIDGET GUI-WINDOW GUI-WINDOW] = types sort copy blk
	--assert [GUI-WINDOW GUI-WINDOW GUI-WIDGET GUI-WIDGET] = types sort/reverse copy blk
	;; and a sort must not depend on the order it started from
	--assert (types sort reduce [h1 h3 h2 h4]) = (types sort reduce [h4 h2 h3 h1])

--test-- "handle as a key in map"
	m: #[]
	--assert not error? try [m/(h1): 1]
	--assert not error? try [repend m [h2 2 h3 3]]
	--assert 1 = try [pick m h1]
	--assert 2 = try [m/(h2)]
	--assert 3 = try [select m h3]
	;; two handles of one type are two keys, not one
	--assert 3 = length? words-of m
	--assert none? select m h4
	;; and a key stays findable across a collection, for the same reason
	;; equality does not depend on when the value was taken
	recycle
	--assert 1 = try [pick m h1]
	--assert 2 = try [pick m h2]

--test-- "set operations with handles"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1765
	--assert 2 = length? u: unique     reduce [h1 h1 h3]
	--assert did all [find u h1  find u h3]
	--assert 2 = length? d: difference reduce [h1 h3] reduce [h3 h2]
	--assert did all [find d h1  find d h2  not find d h3]
	--assert 2 = length? n: union      reduce [h1 h3] reduce [h3 h1]
	--assert did all [find n h1  find n h3]
	--assert [GUI-WINDOW] = types intersect reduce [h1 h3] reduce [h3 h2]
	--assert [GUI-WIDGET] = types exclude   reduce [h1 h3] reduce [h3 h2]

--test-- "query handle's type"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2465
	;; short and easy way
	--assert h1/type = 'GUI-WIDGET
	--assert h3/type = 'GUI-WINDOW
	;; for consistency with other types (like date, image, etc..)
	;@@ https://github.com/Oldes/Rebol-issues/issues/906
	--assert [type] = words-of h1
	--assert 'GUI-WIDGET = query h1 'type
	--assert all [object? o: query h1 object! o/type = 'GUI-WIDGET]

--test-- "released handles"
	;; A closed window and the widgets it held keep their handles, and keep
	;; their type - the type is a property of the handle, not of the native
	;; thing it was holding.
	close-window w1
	--assert h1/type = 'GUI-WIDGET
	--assert h3/type = 'GUI-WINDOW
	--assert not h3/open?
	;; A word the extension does not know is refused rather than answered
	;; with none - which is what leaves PD_Handle free to supply `type`
	;; above. An extension which answers none here takes `type` with it.
	--assert error? try [h1/no-such-accessor]
	;; ... and they are still themselves, and still not each other
	--assert h1 = h1
	--assert not h1 = h2
	--assert not h1 = h3

	close-window w2
	set [h1 h2 h3 h4 w1 w2 blk u d n m] none
] ;<- if all []
===end-group===

~~~end-file~~~