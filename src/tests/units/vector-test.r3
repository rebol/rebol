Rebol [
	Title:   "Rebol vector test script"
	Author:  "Oldes"
	File: 	 %vector-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "VECTOR"

===start-group=== "VECTOR"

--test-- "issue/2346"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2346
	--assert [] = to-block make vector! 0

--test-- "issue/1036"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1036
	--assert 2 = index? load mold/all next make vector! [integer! 32 4 [1 2 3 4]]

--test-- "VECTOR can be initialized using a block with CHARs"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2348
	--assert vector? v: make vector! [integer! 8 [#"^(00)" #"^(01)" #"^(02)" #"a" #"b"]]
	--assert  0 = v/1
	--assert 98 = v/5

	--assert vector? v: make vector! [integer! 16 [#"^(00)" #"^(01)" #"^(02)" #"a" #"b"]]
	--assert  0 = v/1
	--assert 98 = v/5

--test-- "Random shuffle of vector vs. block"
	;@@ https://github.com/Oldes/Rebol-issues/issues/947
	v1: make vector! [integer! 32 5 [1 2 3 4 5]]
	v2: random v1
	--assert same? v1 v2
	b1: [1 2 3 4 5]
	b2: random b1
	--assert same? b1 b2

--test-- "Some vector! formats are invalid"
	;@@ https://github.com/Oldes/Rebol-issues/issues/350
	--assert error? try [make vector! [- decimal! 32]]
	--assert error? try [make vector! [- integer! 32]]

--test-- "FIRST, LAST on vector"
	;@@ https://github.com/Oldes/Rebol-issues/issues/459
	v: make vector! [integer! 8 [1 2 3]]
	--assert 1 = first v
	--assert 3 = last v
	--assert 1 = v/1
	--assert 3 = v/3

--test-- "to-block vector!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/865
	--assert [0 0] = to-block make vector! [integer! 32 2]
	--assert [1 2] = to block! #[ui16! [1 2]]

--test-- "LOAD/MOLD on vector"
	--assert v = load mold/all v
	--assert v = do load mold v

--test-- "Conversion from VECTOR to BINARY"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2347
	--assert #{0102} = to binary! make vector! [integer! 8 [1 2]]
	--assert #{01000200} = to binary! make vector! [integer! 16 [1 2]]
	--assert #{0100000002000000} = to binary! make vector! [integer! 32 [1 2]]
	--assert 1 = to integer! head reverse to binary! make vector! [integer! 64 [1]]
	--assert #{0000803F} = to binary! make vector! [decimal! 32 [1.0]]
	--assert 1.0 = to decimal! head reverse to binary! make vector! [decimal! 64 [1.0]]

--test-- "VECTOR can be initialized using binary data"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1410
	--assert vector? v: make vector! [integer! 16 #{010002000300}]
	--assert 1 = v/1
	--assert 3 = v/3

	b: to binary! make vector! [decimal! 32 [1.0 -1.0]]
	v: make vector! compose [decimal! 32 (b)]
	--assert v/1 = 1.0
	--assert v/2 = -1.0
	--assert b = to binary! v

--test-- "Croping input specification when size and series is provided"
	--assert 2 = length? v: make vector! [integer! 16 2 [1 2 3 4]]
	--assert 2 = v/2
	--assert none? v/3
	--assert 1 = length? v: make vector! [integer! 16 1 #{01000200}]
	--assert none? v/2
	--assert 1 = length? v: #[si16! 1 #{01000200}]
	--assert none? v/2

--test-- "Extending input specification when size and series is provided"
	--assert 4 = length? v: make vector! [integer! 16 4 [1 2]]
	--assert 2 = v/2
	--assert 0 = v/4
	--assert none? v/5

--test-- "Vector created with specified index"
	--assert 2 = index? v: make vector! [integer! 16 [1 2] 2]
	--assert 2 = index? v: make vector! [integer! 16 #{01000200} 2]
	--assert 2 = index? v: #[si16! [1 2] 2]
	--assert 2 = index? v: #[si16! #{01000200} 2]

--test-- "MOLD of unsigned vector"
	;@@ https://github.com/Oldes/Rebol-issues/issues/756
	--assert "make vector! [integer! 32 2 [0 0]]" = mold make vector! [signed integer! 32 2]
	--assert "make vector! [unsigned integer! 32 2 [0 0]]" = mold make vector! [unsigned integer! 32 2]

--test-- "MOLD/flat on vector"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2349
	--assert (mold/flat make vector! [integer! 8 12]) = {make vector! [integer! 8 12 [0 0 0 0 0 0 0 0 0 0 0 0]]}
	--assert (mold/all/flat make vector! [integer! 8 12]) = "#[vector! integer! 8 12 [0 0 0 0 0 0 0 0 0 0 0 0]]"
	--assert (mold make vector! [integer! 8  2]) = {make vector! [integer! 8 2 [0 0]]}
	--assert (mold make vector! [integer! 8 20]) = {make vector! [integer! 8 20 [
    0 0 0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 0 0 0
]]}
	v: make vector! [integer! 8 20]
	--assert (mold reduce [
	1 2
	v
	3 4
]) = {[
    1 2 make vector! [integer! 8 20 [
        0 0 0 0 0 0 0 0 0 0
        0 0 0 0 0 0 0 0 0 0
    ]]
    3 4
]}

--test-- "QUERY on vector"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2352
	v: make vector! [unsigned integer! 16 2]
	o: query v
	--assert object? o
	--assert not o/signed
	--assert o/type = 'integer!
	--assert o/size = 16
	--assert o/length = 2
--test-- "QUERY/MODE on vector"
	--assert [signed type size length] = query/mode v none
	--assert [16 integer!] = query/mode v [size type]
	--assert block? b: query/mode v [signed: length:]
	--assert all [not b/signed b/length = 2]
	--assert 16 = query/mode v 'size
	--assert 16 = size? v
--test-- "REFLECT on vector"
	--assert 16 = reflect v 'size
	--assert  2 = reflect v 'length
	--assert 'integer! = reflect v 'type
	--assert false = reflect v 'signed
	--assert [unsigned integer! 16 2] = reflect v 'spec
	--assert [unsigned integer! 16 2] = spec-of v
--test-- "ACCESSORS on vector"
	--assert 16 = v/size
	--assert  2 = v/length
	--assert 'integer! = v/type
	--assert false     = v/signed
===end-group===

===start-group=== "VECTOR compact construction"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2396
	--test-- "Compact construction syntax (empty)"
		--assert (mold #[si8! ]) = "make vector! [integer! 8 0 []]"
		--assert (mold #[si16!]) = "make vector! [integer! 16 0 []]"
		--assert (mold #[si32!]) = "make vector! [integer! 32 0 []]"
		--assert (mold #[si64!]) = "make vector! [integer! 64 0 []]"
		--assert (mold #[ui8! ]) = "make vector! [unsigned integer! 8 0 []]"
		--assert (mold #[ui16!]) = "make vector! [unsigned integer! 16 0 []]"
		--assert (mold #[ui32!]) = "make vector! [unsigned integer! 32 0 []]"
		--assert (mold #[ui64!]) = "make vector! [unsigned integer! 64 0 []]"
		--assert (mold #[f32! ]) = "make vector! [decimal! 32 0 []]"
		--assert (mold #[f64! ]) = "make vector! [decimal! 64 0 []]"

	--test-- "Compact construction syntax (size)"
		--assert (mold #[si8!  3]) = "make vector! [integer! 8 3 [0 0 0]]"
		--assert (mold #[si16! 3]) = "make vector! [integer! 16 3 [0 0 0]]"
		--assert (mold #[si32! 3]) = "make vector! [integer! 32 3 [0 0 0]]"
		--assert (mold #[si64! 3]) = "make vector! [integer! 64 3 [0 0 0]]"
		--assert (mold #[ui8!  3]) = "make vector! [unsigned integer! 8 3 [0 0 0]]"
		--assert (mold #[ui16! 3]) = "make vector! [unsigned integer! 16 3 [0 0 0]]"
		--assert (mold #[ui32! 3]) = "make vector! [unsigned integer! 32 3 [0 0 0]]"
		--assert (mold #[ui64! 3]) = "make vector! [unsigned integer! 64 3 [0 0 0]]"
		--assert (mold #[f32!  3]) = "make vector! [decimal! 32 3 [0.0 0.0 0.0]]"
		--assert (mold #[f64!  3]) = "make vector! [decimal! 64 3 [0.0 0.0 0.0]]"

	--test-- "Compact construction syntax (data)"
		--assert (mold #[si8!  [1 2]]) = "make vector! [integer! 8 2 [1 2]]"
		--assert (mold #[si16! [1 2]]) = "make vector! [integer! 16 2 [1 2]]"
		--assert (mold #[si32! [1 2]]) = "make vector! [integer! 32 2 [1 2]]"
		--assert (mold #[si64! [1 2]]) = "make vector! [integer! 64 2 [1 2]]"
		--assert (mold #[ui8!  [1 2]]) = "make vector! [unsigned integer! 8 2 [1 2]]"
		--assert (mold #[ui16! [1 2]]) = "make vector! [unsigned integer! 16 2 [1 2]]"
		--assert (mold #[ui32! [1 2]]) = "make vector! [unsigned integer! 32 2 [1 2]]"
		--assert (mold #[ui64! [1 2]]) = "make vector! [unsigned integer! 64 2 [1 2]]"
		--assert (mold #[f32!  [1 2]]) = "make vector! [decimal! 32 2 [1.0 2.0]]"
		--assert (mold #[f64!  [1 2]]) = "make vector! [decimal! 64 2 [1.0 2.0]]"

	--test-- "Compact construction syntax (data with index)"
		--assert (mold v: #[si8!  [1 2] 2]) = "make vector! [integer! 8 1 [2]]"
		--assert 2 = index? v
		--assert (mold v: #[si16! [1 2] 2]) = "make vector! [integer! 16 1 [2]]"
		--assert 2 = index? v
		--assert (mold v: #[si32! [1 2] 2]) = "make vector! [integer! 32 1 [2]]"
		--assert 2 = index? v
		--assert (mold v: #[si64! [1 2] 2]) = "make vector! [integer! 64 1 [2]]"
		--assert 2 = index? v
		--assert (mold v: #[ui8!  [1 2] 2]) = "make vector! [unsigned integer! 8 1 [2]]"
		--assert 2 = index? v
		--assert (mold v: #[ui16! [1 2] 2]) = "make vector! [unsigned integer! 16 1 [2]]"
		--assert 2 = index? v
		--assert (mold v: #[ui32! [1 2] 2]) = "make vector! [unsigned integer! 32 1 [2]]"
		--assert 2 = index? v
		--assert (mold v: #[ui64! [1 2] 2]) = "make vector! [unsigned integer! 64 1 [2]]"
		--assert 2 = index? v
		--assert (mold v: #[f32!  [1 2] 2]) = "make vector! [decimal! 32 1 [2.0]]"
		--assert 2 = index? v
		--assert (mold v: #[f64!  [1 2] 2]) = "make vector! [decimal! 64 1 [2.0]]"
		--assert 2 = index? v
===end-group===

===start-group=== "VECTOR semi-compact construction"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2396
	--test-- "Compact construction syntax (empty)"
		--assert (mold make vector! [si8! ]) = "make vector! [integer! 8 0 []]"
		--assert (mold make vector! [si16!]) = "make vector! [integer! 16 0 []]"
		--assert (mold make vector! [si32!]) = "make vector! [integer! 32 0 []]"
		--assert (mold make vector! [si64!]) = "make vector! [integer! 64 0 []]"
		--assert (mold make vector! [ui8! ]) = "make vector! [unsigned integer! 8 0 []]"
		--assert (mold make vector! [ui16!]) = "make vector! [unsigned integer! 16 0 []]"
		--assert (mold make vector! [ui32!]) = "make vector! [unsigned integer! 32 0 []]"
		--assert (mold make vector! [ui64!]) = "make vector! [unsigned integer! 64 0 []]"
		--assert (mold make vector! [f32! ]) = "make vector! [decimal! 32 0 []]"
		--assert (mold make vector! [f64! ]) = "make vector! [decimal! 64 0 []]"

	--test-- "Compact construction syntax (size)"
		--assert (mold make vector! [si8!  3]) = "make vector! [integer! 8 3 [0 0 0]]"
		--assert (mold make vector! [si16! 3]) = "make vector! [integer! 16 3 [0 0 0]]"
		--assert (mold make vector! [si32! 3]) = "make vector! [integer! 32 3 [0 0 0]]"
		--assert (mold make vector! [si64! 3]) = "make vector! [integer! 64 3 [0 0 0]]"
		--assert (mold make vector! [ui8!  3]) = "make vector! [unsigned integer! 8 3 [0 0 0]]"
		--assert (mold make vector! [ui16! 3]) = "make vector! [unsigned integer! 16 3 [0 0 0]]"
		--assert (mold make vector! [ui32! 3]) = "make vector! [unsigned integer! 32 3 [0 0 0]]"
		--assert (mold make vector! [ui64! 3]) = "make vector! [unsigned integer! 64 3 [0 0 0]]"
		--assert (mold make vector! [f32!  3]) = "make vector! [decimal! 32 3 [0.0 0.0 0.0]]"
		--assert (mold make vector! [f64!  3]) = "make vector! [decimal! 64 3 [0.0 0.0 0.0]]"

	--test-- "Compact construction syntax (data)"
		--assert (mold make vector! [si8!  [1 2]]) = "make vector! [integer! 8 2 [1 2]]"
		--assert (mold make vector! [si16! [1 2]]) = "make vector! [integer! 16 2 [1 2]]"
		--assert (mold make vector! [si32! [1 2]]) = "make vector! [integer! 32 2 [1 2]]"
		--assert (mold make vector! [si64! [1 2]]) = "make vector! [integer! 64 2 [1 2]]"
		--assert (mold make vector! [ui8!  [1 2]]) = "make vector! [unsigned integer! 8 2 [1 2]]"
		--assert (mold make vector! [ui16! [1 2]]) = "make vector! [unsigned integer! 16 2 [1 2]]"
		--assert (mold make vector! [ui32! [1 2]]) = "make vector! [unsigned integer! 32 2 [1 2]]"
		--assert (mold make vector! [ui64! [1 2]]) = "make vector! [unsigned integer! 64 2 [1 2]]"
		--assert (mold make vector! [f32!  [1 2]]) = "make vector! [decimal! 32 2 [1.0 2.0]]"
		--assert (mold make vector! [f64!  [1 2]]) = "make vector! [decimal! 64 2 [1.0 2.0]]"

	--test-- "Compact construction syntax (data with index)"
		--assert (mold v: make vector! [si8!  [1 2] 2]) = "make vector! [integer! 8 1 [2]]"
		--assert 2 = index? v
		--assert (mold v: make vector! [si16! [1 2] 2]) = "make vector! [integer! 16 1 [2]]"
		--assert 2 = index? v
		--assert (mold v: make vector! [si32! [1 2] 2]) = "make vector! [integer! 32 1 [2]]"
		--assert 2 = index? v
		--assert (mold v: make vector! [si64! [1 2] 2]) = "make vector! [integer! 64 1 [2]]"
		--assert 2 = index? v
		--assert (mold v: make vector! [ui8!  [1 2] 2]) = "make vector! [unsigned integer! 8 1 [2]]"
		--assert 2 = index? v
		--assert (mold v: make vector! [ui16! [1 2] 2]) = "make vector! [unsigned integer! 16 1 [2]]"
		--assert 2 = index? v
		--assert (mold v: make vector! [ui32! [1 2] 2]) = "make vector! [unsigned integer! 32 1 [2]]"
		--assert 2 = index? v
		--assert (mold v: make vector! [ui64! [1 2] 2]) = "make vector! [unsigned integer! 64 1 [2]]"
		--assert 2 = index? v
		--assert (mold v: make vector! [f32!  [1 2] 2]) = "make vector! [decimal! 32 1 [2.0]]"
		--assert 2 = index? v
		--assert (mold v: make vector! [f64!  [1 2] 2]) = "make vector! [decimal! 64 1 [2.0]]"
		--assert 2 = index? v
===end-group===

===start-group=== "VECTOR math"

--test-- "VECTOR 8bit integer add/subtract"
	v: #[ui8![1 2 3 4]]
	--assert (v + 200) = #[ui8![201 202 203 204]]
	; the values are truncated on overflow:
	--assert (v + 200) = #[ui8![145 146 147 148]]
	--assert (v - 400) = #[ui8![1 2 3 4]]
	subtract (add v 10) 10
	--assert v = #[ui8![1 2 3 4]]
	1 + v
	--assert v = #[ui8![2 3 4 5]]
	-1.0 + v
	--assert v = #[ui8![1 2 3 4]]

	v: #[si8![1 2 3 4]]
	--assert (v + 125) = #[si8![126 127 -128 -127]]
	--assert (v - 125) = #[si8![1 2 3 4]]

--test-- "VECTOR 8bit integer multiply"
	v: #[ui8![1 2 3 4]]
	--assert (v * 4) = #[ui8![4 8 12 16]]
	; the values are truncated on overflow:
	--assert (v * 20) = #[ui8![80 160 240 64]] ;64 = (16 * 20) - 256

	v: #[si8![1 2 3 4]]
	--assert (v * 2.0) = #[si8![2 4 6 8]]
	; the decimal is first converted to integer (2):
	--assert (v * 2.4) = #[si8![4 8 12 16]]
	subtract (add v 10) 10
	--assert v = #[si8![4 8 12 16]]

--test-- "VECTOR 16bit integer multiply"
	v: #[ui16![1 2 3 4]]
	--assert (v * 4)  = #[ui16![4 8 12 16]]
	--assert (v * 20) = #[ui16![80 160 240 320]]
	multiply v 2
	--assert v = #[ui16![160 320 480 640]]

	v: #[ui16![1 2 3 4]]
	--assert (10   * copy v) = #[ui16![10 20 30 40]]
	--assert (10.0 * copy v) = #[ui16![10 20 30 40]]

	; the values are truncated on overflow:
	v: #[ui16![1 2 3 4]]
	--assert (v * 10000) = #[ui16![10000 20000 30000 40000]]
	--assert (v * 10.0)  = #[ui16![34464 3392 37856 6784]]

--test-- "VECTOR 16bit integer divide"
	v: #[ui16![80 160 240 320]]
	v / 20 / 2
	divide v 2
	--assert v = #[ui16![1 2 3 4]]
	--assert error? try [10 / v]
	--assert error? try [ v / 0] 

--test-- "VECTOR 32bit decimal add/subtract"
	v: #[f32![1 2 3 4]]
	--assert (v + 200) = #[f32![201 202 203 204]]
	--assert (v + 0.5) = #[f32![201.5 202.5 203.5 204.5]]
	; notice the precision lost with 32bit decimal value:
	v - 0.1
	--assert 2013 = to integer! 10 * v/1 ; result is not 201.4 as would be with 64bit

--test-- "VECTOR 64bit decimal add/subtract"
	v: #[f64![1 2 3 4]]
	--assert (v + 200) = #[f64![201 202 203 204]]
	--assert (v + 0.5) = #[f64![201.5 202.5 203.5 204.5]]
	--assert (v - 0.1) = #[f64![201.4 202.4 203.4 204.4]]

--test-- "VECTOR 64bit decimal multiply/divide"
	v: #[f64![1 2 3 4]]
	--assert (v * 20.5) = #[f64![20.5 41.0 61.5 82.0]]
	--assert (v / 20.5) = #[f64![1.0 2.0 3.0 4.0]]

--test-- "VECTOR math operation with vector not at head"
	v: #[si8![1 2 3 4]]
	--assert (2 + skip v 2) = #[si8![5 6]]
	--assert v = #[si8![1 2 5 6]]

===end-group===


===start-group=== "VECTOR Compare"
	--test-- "compare vectors"
	;@@  https://github.com/Oldes/Rebol-issues/issues/458
	--assert equal? (make vector! 3)(make vector! 3)
	--assert not equal? #[ui16! [1 2]] #[ui16! [1 2 3]]
	--assert #[ui16! [1 2]] = #[ui16! [1 2]]
	--assert #[ui16! [1 2]] < #[ui16! [1 2 0]]
	--assert #[ui16! [1 2]] < #[ui16! [1 2 1]]
	--assert #[ui16! [1 2]] < #[ui16! [2 2]]
	--assert #[ui16! [2 2]] > #[ui16! [1 2]]

===end-group===


===start-group=== "VECTOR copy"

--test-- "COPY"
	;@@ https://github.com/Oldes/Rebol-issues/issues/463
	;@@ https://github.com/Oldes/Rebol-issues/issues/2400
	v1: #[ui16! [1 2]]
	v2: v1
	v3: copy v2
	--assert     same? v1 v2
	--assert not same? v1 v3
	v2/1: 3
	--assert v1/1 = 3
	--assert v3/1 = 1
	

--test-- "COPY/PART"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2399
	v: #[ui16! [1 2 3 4]]
	--assert           2 = length? copy/part v 2
	--assert #{01000200} = to-binary copy/part v 2
	--assert #{03000400} = to-binary copy/part skip v 2 2

===end-group===


===start-group=== "PICK"
	--test-- "PICK of vector!"
	;@@  https://github.com/Oldes/Rebol-issues/issues/748
	v: #[ui32! [1 2 3]]
	--assert all [
		1   = pick v 1
		2   = pick v 2
		none? pick v -1
		none? pick v 0
		none? pick v 10
	]
===end-group===


===start-group=== "POKE"
	--test-- "POKE into vector!"
	v: #[ui32! [1 2 3]]
	--assert all [
		10 = poke v 1 10
		10 = pick v 1
	]
	;@@  https://github.com/Oldes/Rebol-issues/issues/2427
	--assert all [
		error? err: try [poke v 10 1]
		err/id = 'out-of-range
	]
	--assert all [
		error? err: try [poke v 0 1]
		err/id = 'out-of-range
	]

	--test-- "POKE into decimal vector"
	;@@ https://github.com/metaeducation/rebol-issues/issues/2508
	--assert all [
		vector? a: make vector! [decimal! 32 3]
		1.0 = poke a 1 1.0
		1.0 = a/1
		1.0 = pick a 1
	]
===end-group===


===start-group=== "FIND-MAX / FIND-MIN"
	;@@ https://github.com/Oldes/Rebol-issues/issues/460
	v: #[si32! [1 2 3 -1]]
	--test-- "FIND-MAX vector!" --assert  3 = first find-max v
	--test-- "FIND-MIN vector!" --assert -1 = first find-min v
===end-group===


~~~end-file~~~
