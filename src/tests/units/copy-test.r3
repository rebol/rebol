Rebol [
	Title:   "Rebol copy test script"
	Author:  "Oldes"
	File: 	 %copy-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "COPY"
;@@ https://github.com/red/red/issues/2167
;@@ https://github.com/Oldes/Rebol-issues/issues/1864
;@@ https://github.com/Oldes/Rebol-issues/issues/2293

copyable-data: make object! [
	string:   "foo"
	url:      http://
	ref:      @ref
	tag:      <a>
	file:     %foo
	email:    a@b.c
	binary:   #{}
	block:    [1 2]
	paren:    quote (1 2)
	path:     quote a/b
	set-path: quote a/b:
	get-path: quote :a/b
	lit-path: quote 'a/b
	map:      #[a: 1]
	object:   #(object! [a: 1])
	image:    #(image! 1x1 #{FFFFFF})
	bitset:   #(bitset! #{FF})
	vector:   #(u32! 3)
	error:    try [1 / 0]
	;TODO: struct,
]
not-copyable-data: make object! [
	integer:  1
	percent:  1%
	money:    $1
	char:     #"a"
	pair:     1x1
	tuple:    1.1.1
	date:     1-1-2000
	time:     1:1:1
	logic:    #(true)
	none:     #(none)
	gob:      #(gob![size: 20x20])
	word:     'foo
	lit-word: quote 'foo
	set-word: quote foo:
	get-word: quote :foo
	typeset:  #(typeset! [integer! percent!])
	module:   system/modules/help ;@@ or should it be copyable?
	event:    make event! []      ;@@ or should it be copyable?
	;handle
]

===start-group=== "Simple copy"
	foreach [k v] copyable-data [
		--test-- join "copy " k
		c: copy x: select copyable-data k
		--assert not same? :c :x
	]
	foreach [k v] not-copyable-data [
		--test-- join "copy " k
		--assert all [
			error? e: try [c: copy select not-copyable-data k]
			e/id = 'expect-arg
		]
	]
===end-group===


===start-group=== "Shallow copy"
	--test-- "copy object!"
		o: copyable-data
		c: copy o
		--assert not same? o c
		foreach [k v] o [--assert same? o/:k c/:k]

	--test-- "copy block!"
		o: copyable-data
		b: values-of o
		c: copy b
		--assert not same? b c
		repeat i length? b [--assert same? b/:i c/:i]

	--test-- "copy map!"
		m: make map! copyable-data
		c: copy m
		--assert not same? m c
		foreach [k v] o [--assert same? m/:k c/:k]
===end-group===


===start-group=== "Copy map!"
	--test-- "copy/deep map!"
		m1: #[]
		m1/b: copy [1 2 s: "" u: http://foo b: [3 4] o: #(object! [])]
		m1/s: copy ""
		m1/u: http://
		m1/o: #(object! [ s: "" o: #(object![]) ])
		m1/m: #[s: "" u: http://foo b: [3 4] o: #(object! [])]
		m2: copy/deep m1
		--assert not same? m1/b   m2/b
		--assert not same? m1/s   m2/s
		--assert not same? m1/u   m2/u
		--assert not same? m1/m   m2/m
		--assert     same? m1/o   m2/o      ;- object is not copied by default!
		--assert not same? m1/m/s m2/m/s
		--assert not same? m1/m/u m2/m/u
		--assert not same? m1/m/b m2/m/b
		--assert     same? m1/m/o m2/m/o

	--test-- "copy/types map! (only strings)"
		m3: copy/types m1 string!
		--assert     same? m1/b   m3/b
		--assert not same? m1/s   m3/s
		--assert     same? m1/u   m3/u
		--assert     same? m1/m   m3/m
		--assert     same? m1/o   m3/o
		--assert     same? m1/m/s m3/m/s ; because inside not copied map
		--assert     same? m1/m/u m3/m/u
		--assert     same? m1/m/b m3/m/b
		--assert     same? m1/m/o m3/m/o

	--test-- "copy/types map! (any strings)"
		m4: copy/types m1 any-string!
		--assert     same? m1/b m4/b
		--assert not same? m1/s m4/s
		--assert not same? m1/u m4/u
		--assert     same? m1/m m4/m
		--assert     same? m1/o m4/o

	--test-- "copy/types map! (only object)"
		m5: copy/types m1 object!
		--assert     same? m1/b   m5/b
		--assert     same? m1/s   m5/s
		--assert     same? m1/u   m5/u
		--assert     same? m1/m   m5/m
		--assert not same? m1/o   m5/o
		--assert     same? m1/o/s m5/o/s
		--assert     same? m1/o/o m5/o/o

	--test-- "copy/deep/types map! (only object)"
		m5: copy/deep/types m1 object!
		--assert     same? m1/b   m5/b
		--assert     same? m1/s   m5/s
		--assert     same? m1/u   m5/u
		--assert     same? m1/m   m5/m
		--assert not same? m1/o   m5/o
		--assert     same? m1/o/s m5/o/s
		--assert not same? m1/o/o m5/o/o ; because /deep is used

	--test-- "copy/deep/types map! (only maps and strings)"
		m6: copy/deep/types m1 #(typeset! [string! map!])
		--assert     same? m1/b   m6/b
		--assert not same? m1/s   m6/s
		--assert     same? m1/u   m6/u
		--assert not same? m1/m   m6/m
		--assert     same? m1/o   m6/o
		--assert not same? m1/m/s m6/m/s
		--assert     same? m1/m/u m6/m/u ; url is not copied
		--assert     same? m1/m/b m6/m/b ; block --//--
		--assert     same? m1/m/o m6/m/o

	--test-- "copy/deep/types map! (only maps and strings)"
		m7: copy/deep/types m1 #(typeset! [string! map!])
		--assert     same? m1/b   m7/b
		--assert not same? m1/s   m7/s
		--assert     same? m1/u   m7/u
		--assert not same? m1/m   m7/m
		--assert     same? m1/o   m7/o  
		--assert not same? m1/m/s m7/m/s
		--assert     same? m1/m/u m7/m/u ; url is not copied
		--assert     same? m1/m/b m7/m/b ; block --//--
		--assert     same? m1/m/o m7/m/o

	--test-- "copy/deep/types map (any-type!)"
		m8: copy/deep/types m1 any-type!
		--assert not same? m1/b   m8/b
		--assert not same? m1/s   m8/s
		--assert not same? m1/u   m8/u
		--assert not same? m1/m   m8/m
		--assert not same? m1/o   m8/o  
		--assert not same? m1/b/s m8/b/s
		--assert not same? m1/b/u m8/b/u
		--assert not same? m1/b/b m8/b/b
		--assert not same? m1/b/o m8/b/o
		--assert not same? m1/m/s m8/m/s
		--assert not same? m1/m/u m8/m/u
		--assert not same? m1/m/b m8/m/b
		--assert not same? m1/m/o m8/m/o
		--assert not same? m1/o/s m8/o/s
		--assert not same? m1/o/o m8/o/o
===end-group===


===start-group=== "Copy object!"
	--test-- "copy/deep object!"
		o1: make object! [
			b: copy [1 2 s: "" u: http://foo b: [3 4] o: #(object! [])]
			s: copy ""
			u: http://
			o: #(object! [ s: "" u: http://foo b: [3 4] o: #(object![]) ])
			m: #[s: "" u: http://foo b: [3 4] o: #(object! [])]
		]
		o2: copy/deep o1
		--assert not same? o1/b   o2/b
		--assert not same? o1/s   o2/s
		--assert not same? o1/u   o2/u
		--assert not same? o1/m   o2/m
		--assert     same? o1/o   o2/o      ;- object is not copied by default!
		--assert not same? o1/m/s o2/m/s
		--assert not same? o1/m/u o2/m/u
		--assert not same? o1/m/b o2/m/b
		--assert     same? o1/m/o o2/m/o

	--test-- "copy/types object! (only strings)"
		o3: copy/types o1 string!
		--assert     same? o1/b   o3/b
		--assert not same? o1/s   o3/s
		--assert     same? o1/u   o3/u
		--assert     same? o1/m   o3/m
		--assert     same? o1/o   o3/o

	--test-- "copy/types object! (any strings)"
		o4: copy/types o1 any-string!
		--assert     same? o1/b   o4/b
		--assert not same? o1/s   o4/s
		--assert not same? o1/u   o4/u
		--assert     same? o1/m   o4/m
		--assert     same? o1/o   o4/o

	--test-- "copy/types object! (only object)"
		o5: copy/types o1 object!
		--assert     same? o1/b   o5/b
		--assert     same? o1/s   o5/s
		--assert     same? o1/u   o5/u
		--assert     same? o1/m   o5/m
		--assert not same? o1/o   o5/o
		--assert     same? o1/o/s o5/o/s
		--assert     same? o1/o/o o5/o/o ; because not deep

	--test-- "copy/deep/types object! (only object)"
		o5: copy/deep/types o1 object!
		--assert     same? o1/b   o5/b
		--assert     same? o1/s   o5/s
		--assert     same? o1/u   o5/u
		--assert     same? o1/m   o5/m
		--assert not same? o1/o   o5/o
		--assert     same? o1/o/s o5/o/s
		--assert not same? o1/o/o o5/o/o ; because /deep is used

	--test-- "copy/deep/types object! (only maps and strings)"
		o6: copy/deep/types o1 #(typeset! [string! map!])
		--assert     same? o1/b   o6/b
		--assert not same? o1/s   o6/s
		--assert     same? o1/u   o6/u
		--assert not same? o1/m   o6/m
		--assert     same? o1/o   o6/o
		--assert not same? o1/m/s o6/m/s
		--assert     same? o1/m/u o6/m/u ; url is not copied
		--assert     same? o1/m/b o6/m/b ; block --//--
		--assert     same? o1/m/o o6/m/o

	--test-- "copy/deep/types object! (only objects and strings)"
		o7: copy/deep/types o1 #(typeset! [string! object!])
		--assert     same? o1/b   o7/b
		--assert not same? o1/s   o7/s
		--assert     same? o1/u   o7/u
		--assert     same? o1/m   o7/m
		--assert not same? o1/o   o7/o  
		--assert not same? o1/o/s o7/o/s
		--assert     same? o1/o/u o7/o/u ; url is not copied
		--assert     same? o1/o/b o7/o/b ; block --//--
		--assert not same? o1/o/o o7/o/o

	--test-- "copy/deep/types object! (any-type!)"
		o8: copy/deep/types o1 any-type!
		--assert not same? o1/b   o8/b
		--assert not same? o1/s   o8/s
		--assert not same? o1/u   o8/u
		--assert not same? o1/m   o8/m
		--assert not same? o1/o   o8/o  
		--assert not same? o1/b/s o8/b/s
		--assert not same? o1/b/u o8/b/u
		--assert not same? o1/b/b o8/b/b
		--assert not same? o1/b/o o8/b/o
		--assert not same? o1/m/s o8/m/s
		--assert not same? o1/m/u o8/m/u
		--assert not same? o1/m/b o8/m/b
		--assert not same? o1/m/o o8/m/o
		--assert not same? o1/o/s o8/o/s
		--assert not same? o1/o/o o8/o/o
===end-group===


===start-group=== "Copy block!"
	--test-- "copy/deep block!"
		b1: reduce [
			quote b: copy [1 2 s: "" u: http://foo b: [3 4] o: #(object! [])]
			quote s: copy ""
			quote u: http://
			quote o: #(object! [ s: "" u: http:// b: [] o: #(object![]) ])
			quote m: #[s: "" u: http://foo b: [3 4] o: #(object! [])]
		]
		b2: copy/deep b1
		--assert not same? b1/b   b2/b
		--assert not same? b1/s   b2/s
		--assert not same? b1/u   b2/u
		--assert not same? b1/m   b2/m
		--assert     same? b1/o   b2/o      ;- object is not copied by default!
		--assert not same? b1/m/s b2/m/s
		--assert not same? b1/m/u b2/m/u
		--assert not same? b1/m/b b2/m/b
		--assert     same? b1/m/o b2/m/o

	--test-- "copy/types block! (only strings)"
		b3: copy/types b1 string!
		--assert     same? b1/b   b3/b
		--assert not same? b1/s   b3/s
		--assert     same? b1/u   b3/u
		--assert     same? b1/m   b3/m
		--assert     same? b1/o   b3/o
		--assert     same? b1/b/s b3/b/s ; because inside not copied map
		--assert     same? b1/b/u b3/b/u
		--assert     same? b1/b/b b3/b/b
		--assert     same? b1/b/o b3/b/o

	--test-- "copy/types/deep block! (only strings)"
		; same like `copy/types`, because string does not hold any values
		b3: copy/types/deep b1 string!
		--assert     same? b1/b   b3/b
		--assert not same? b1/s   b3/s
		--assert     same? b1/u   b3/u
		--assert     same? b1/m   b3/m
		--assert     same? b1/o   b3/o
		--assert     same? b1/b/s b3/b/s ; because it is inside a block, which is not supposed to be copied
		--assert     same? b1/b/u b3/b/u
		--assert     same? b1/b/b b3/b/b
		--assert     same? b1/b/o b3/b/o

	--test-- "copy/types block! (any strings)"	
		b4: copy/types b1 any-string!
		--assert     same? b1/b b4/b
		--assert not same? b1/s b4/s
		--assert not same? b1/u b4/u
		--assert     same? b1/m b4/m
		--assert     same? b1/o b4/o

	--test-- "copy/types block! (only object)"
		b5: copy/types b1 object!
		--assert     same? b1/b   b5/b
		--assert     same? b1/s   b5/s
		--assert     same? b1/u   b5/u
		--assert     same? b1/m   b5/m
		--assert not same? b1/o   b5/o
		--assert     same? b1/o/s b5/o/s
		--assert     same? b1/o/b b5/o/b
		--assert     same? b1/o/o b5/o/o  ; same, because not /deep

	--test-- "copy/deep/types block! (only object)"
		b5: copy/deep/types b1 object!
		--assert     same? b1/b   b5/b
		--assert     same? b1/s   b5/s
		--assert     same? b1/u   b5/u
		--assert     same? b1/m   b5/m
		--assert not same? b1/o   b5/o
		--assert     same? b1/o/s b5/o/s
		--assert not same? b1/o/o b5/o/o ; because /deep is used

	--test-- "copy/deep/types block! (only block and strings)"
		b6: copy/deep/types b1 #(typeset! [block! string!])
		--assert not same? b1/b   b6/b
		--assert not same? b1/s   b6/s
		--assert     same? b1/u   b6/u
		--assert     same? b1/m   b6/m
		--assert     same? b1/o   b6/o  
		--assert not same? b1/b/s b6/b/s
		--assert     same? b1/b/u b6/b/u ; url is not copied
		--assert not same? b1/b/b b6/b/b
		--assert     same? b1/b/o b6/b/o

	--test-- "copy/deep/types block! (only objects, blocks and strings)"
		b7: copy/deep/types b1 #(typeset! [string! block! object!])
		--assert not same? b1/b   b7/b
		--assert not same? b1/s   b7/s
		--assert     same? b1/u   b7/u
		--assert     same? b1/m   b7/m
		--assert not same? b1/o   b7/o  
		--assert not same? b1/b/s b7/b/s
		--assert     same? b1/b/u b7/b/u ; url is not copied
		--assert not same? b1/b/b b7/b/b
		--assert not same? b1/b/o b7/b/o

	--test-- "copy/deep/types block! (any-type!)"
		b8: copy/deep/types b1 any-type!
		--assert not same? b1/b   b8/b
		--assert not same? b1/s   b8/s
		--assert not same? b1/u   b8/u
		--assert not same? b1/m   b8/m
		--assert not same? b1/o   b8/o  
		--assert not same? b1/b/s b8/b/s
		--assert not same? b1/b/u b8/b/u
		--assert not same? b1/b/b b8/b/b
		--assert not same? b1/b/o b8/b/o
		--assert not same? b1/m/s b8/m/s
		--assert not same? b1/m/u b8/m/u
		--assert not same? b1/m/b b8/m/b
		--assert not same? b1/m/o b8/m/o
		--assert not same? b1/o/s b8/o/s
		--assert not same? b1/o/u b8/o/u
		--assert not same? b1/o/b b8/o/b
		--assert not same? b1/o/o b8/o/o
===end-group===


~~~end-file~~~