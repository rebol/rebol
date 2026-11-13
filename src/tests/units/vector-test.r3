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

--test-- "issue/1026"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1026
	--assert #(int32! []) == make vector! 0
	--assert #(int32! []) == make vector! []
	
--test-- "VECTOR can be initialized using a block with CHARs"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2348
	--assert vector? v: make vector! [integer! 8 [#"^(00)" #"^(01)" #"^(02)" #"a" #"b"]]
	--assert  0 = v/1
	--assert 98 = v/5

	--assert vector? v: make vector! [integer! 16 [#"^(00)" #"^(01)" #"^(02)" #"a" #"b"]]
	--assert  0 = v/1
	--assert 98 = v/5

--test-- "Make vector with get-words"
	data: [1 2 3 4]
	size: 2
	--assert {#(uint8! [1 2 3 4])}   == mold make vector! [uint8! :data]
	--assert {#(uint8! [1 2])}       == mold make vector! [uint8! :size :data]
	index: 3
	--assert {#(uint8! [3 4])}       == mold make vector! [uint8! :data :index]
	size: 4
	--assert {#(uint8! [3 4])}       == mold make vector! [uint8! :size [1 2 3 4 5] :index]
	--assert {#(uint8! [1 2 3 4] 3)} == mold/all make vector! [uint8! :size [1 2 3 4 5] :index]

--test-- "Make vector using direct values"
	--assert (make vector! [1 2 3 4]) == #(int64! [1 2 3 4])
	--assert (make vector! [1.0 2]) == #(float64! [1.0 2.0])

--test-- "Make empty vector"
	--assert #(uint8! []) == transcode/one "#(uint8!)"
	--assert #(uint32! []) == transcode/one "#(uint32!)"
	--assert #(float32! []) == transcode/one "#(float32!)"

--test-- "Make vector from binary"
	--assert #(uint8! []) == attempt [to vector! #{}]
	--assert #(uint8! [1 255]) == attempt [to vector! #{01FF}]

--test-- "Random shuffle of vector vs. block"
	;@@ https://github.com/Oldes/Rebol-issues/issues/910
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

--test-- "HEAD, TAIL on vector"
	;@@ https://github.com/Oldes/Rebol-issues/issues/462
	v: #(u8! [1 2 3])
	--assert tail? tail v
	--assert head? head v

--test-- "to-block vector!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/865
	--assert [0 0] = to-block make vector! [integer! 32 2]
	--assert [1 2] = to block! #(u16! [1 2])

--test-- "to-binary vector!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2590
	--assert #{01000200} = to binary! #(u16! [1 2])
	--assert #{0100000002000000} = to binary! #(i32! [1 2])
	--assert #{0000803F00000040} = to binary! #(f32! [1 2])
	--assert #{01000000000000000200000000000000} = to binary! #(i64! [1 2])
	--assert #{000000000000F03F0000000000000040} = to binary! #(f64! [1 2])
	;@@ https://github.com/Oldes/Rebol-issues/issues/2518
	--assert #{0200} = to binary! next #(u16! [1 2])
	--assert #{02000000} = to binary! next #(i32! [1 2])
	--assert #{00000040} = to binary! next #(f32! [1 2])
	--assert #{0200000000000000} = to binary! next #(i64! [1 2])
	--assert #{0000000000000040} = to binary! next #(f64! [1 2])
	;@@ https://github.com/Oldes/Rebol-issues/issues/2458
	--assert #{01000200} = to binary! protect #(u16! [1 2])

--test-- "LOAD/MOLD on vector"
	--assert v = load mold/all v
	--assert v = do load mold v
	;@@ https://github.com/Oldes/Rebol-issues/issues/1036
	--assert 2 = index? load mold/all next make vector! [integer! 32 4 [1 2 3 4]]

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
	;- It's not supported to specify size with the construction syntax anymore
	;--assert 1 = length? v: #(i16! 1 #{01000200})
	;--assert none? v/2

--test-- "Extending input specification when size and series is provided"
	--assert 4 = length? v: make vector! [integer! 16 4 [1 2]]
	--assert 2 = v/2
	--assert 0 = v/4
	--assert none? v/5

--test-- "Vector created with specified index"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1038
	--assert 2 = index? v: make vector! [integer! 16 [1 2] 2]
	--assert 2 = index? v: make vector! [integer! 16 #{01000200} 2]
	--assert 2 = index? v: #(i16! [1 2] 2)
	--assert 2 = index? v: #(i16! #{01000200} 2)

--test-- "MOLD of unsigned vector"
	;@@ https://github.com/Oldes/Rebol-issues/issues/756
	--assert "#(int32! [0 0])" = mold make vector! [signed integer! 32 2]
	--assert "#(uint32! [0 0])" = mold make vector! [unsigned integer! 32 2]
	--assert "#(int32! [0 0])" = mold/all make vector! [signed integer! 32 2]
	--assert "#(uint32! [0 0])" = mold/all make vector! [unsigned integer! 32 2]

--test-- "MOLD/flat on vector"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2349
	--assert (mold/flat make vector! [integer! 8 12]) = {#(int8! [0 0 0 0 0 0 0 0 0 0 0 0])}
	--assert (mold/all/flat make vector! [integer! 8 12]) = "#(int8! [0 0 0 0 0 0 0 0 0 0 0 0])"
	--assert (mold make vector! [integer! 8  2]) = "#(int8! [0 0])"
	--assert (mold make vector! [integer! 8 20]) = {#(int8! [
    0 0 0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 0 0 0
])}
	v: make vector! [integer! 8 20]
	--assert (mold reduce [
	1 2
	v
	3 4
]) = {[
    1 2 #(int8! [
        0 0 0 0 0 0 0 0 0 0
        0 0 0 0 0 0 0 0 0 0
    ])
    3 4
]}

--test-- "QUERY on vector as object"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2352
	v: make vector! [unsigned integer! 16 2]
	o: query v object!
	--assert object? o
	--assert not o/signed
	--assert o/type = 'integer!
	--assert o/size = 16
	--assert o/length = 2
	--assert o/minimum = 0
	--assert o/maximum = 0
--test-- "QUERY on vector"
	--assert [signed type size length minimum maximum range sum mean median variance sample-variance population-deviation sample-deviation] = query v none
	--assert [16 integer!] = query v [:size :type]
	--assert block? b: query v [signed length]
	--assert all [not b/signed b/length = 2]
	--assert 16 = query v 'size
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

--test-- "REVERSE on vector"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2515
	--assert #(u8!  [3 2 1]) = reverse #(u8!  [1 2 3])
	--assert #(u16! [3 2 1]) = reverse #(u16! [1 2 3])
	--assert #(u32! [3 2 1]) = reverse #(u32! [1 2 3])
	--assert #(u64! [3 2 1]) = reverse #(u64! [1 2 3])
	--assert #(i8!  [3 2 1]) = reverse #(i8!  [1 2 3])
	--assert #(i16! [3 2 1]) = reverse #(i16! [1 2 3])
	--assert #(i32! [3 2 1]) = reverse #(i32! [1 2 3])
	--assert #(i64! [3 2 1]) = reverse #(i64! [1 2 3])
	--assert #(f32! [3.0 2.0 1.0]) = reverse #(f32! [1 2 3])
	--assert #(f64! [3.0 2.0 1.0]) = reverse #(f64! [1 2 3])

	--assert #(u8!  [2 1 3]) = reverse/part #(u8!  [1 2 3]) 2
	--assert #(u16! [2 1 3]) = reverse/part #(u16! [1 2 3]) 2
	--assert #(u32! [2 1 3]) = reverse/part #(u32! [1 2 3]) 2
	--assert #(u64! [2 1 3]) = reverse/part #(u64! [1 2 3]) 2
	--assert #(i8!  [2 1 3]) = reverse/part #(i8!  [1 2 3]) 2
	--assert #(i16! [2 1 3]) = reverse/part #(i16! [1 2 3]) 2
	--assert #(i32! [2 1 3]) = reverse/part #(i32! [1 2 3]) 2
	--assert #(i64! [2 1 3]) = reverse/part #(i64! [1 2 3]) 2
	--assert #(f32! [2.0 1.0 3.0]) = reverse/part #(f32! [1 2 3]) 2
	--assert #(f64! [2.0 1.0 3.0]) = reverse/part #(f64! [1 2 3]) 2

	--assert #(u8!  [1 3 2]) = head reverse next #(u8!  [1 2 3])
	--assert #(u16! [1 3 2]) = head reverse next #(u16! [1 2 3])
	--assert #(u32! [1 3 2]) = head reverse next #(u32! [1 2 3])
	--assert #(u64! [1 3 2]) = head reverse next #(u64! [1 2 3])
	--assert #(i8!  [1 3 2]) = head reverse next #(i8!  [1 2 3])
	--assert #(i16! [1 3 2]) = head reverse next #(i16! [1 2 3])
	--assert #(i32! [1 3 2]) = head reverse next #(i32! [1 2 3])
	--assert #(i64! [1 3 2]) = head reverse next #(i64! [1 2 3])
	--assert #(f32! [1.0 3.0 2.0]) = head reverse next #(f32! [1 2 3])
	--assert #(f64! [1.0 3.0 2.0]) = head reverse next #(f64! [1 2 3])
===end-group===

===start-group=== "VECTOR compact construction"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2396
	--test-- "Compact construction syntax (empty)"
		;- Not supported anymore!
		;--assert (mold #(i8! ))  == "#(int8! [])"
		;--assert (mold #(i16!))  == "#(int16! [])"
		;--assert (mold #(i32!))  == "#(int32! [])"
		;--assert (mold #(i64!))  == "#(int64! [])"
		;--assert (mold #(u8! ))  == "#(uint8! [])"
		;--assert (mold #(u16!))  == "#(uint16! [])"
		;--assert (mold #(u32!))  == "#(uint32! [])"
		;--assert (mold #(u64!))  == "#(uint64! [])"
		;--assert (mold #(f32! )) == "#(float32! [])"
		;--assert (mold #(f64! )) == "#(float64! [])"

	--test-- "Compact construction syntax (size)"
		;- Not supported anymore!
		;--assert (mold #(i8!  3)) == "#(int8! [0 0 0])"
		;--assert (mold #(i16! 3)) == "#(int16! [0 0 0])"
		;--assert (mold #(i32! 3)) == "#(int32! [0 0 0])"
		;--assert (mold #(i64! 3)) == "#(int64! [0 0 0])"
		;--assert (mold #(u8!  3)) == "#(uint8! [0 0 0])"
		;--assert (mold #(u16! 3)) == "#(uint16! [0 0 0])"
		;--assert (mold #(u32! 3)) == "#(uint32! [0 0 0])"
		;--assert (mold #(u64! 3)) == "#(uint64! [0 0 0])"
		;--assert (mold #(f32! 3)) == "#(float32! [0.0 0.0 0.0])"
		;--assert (mold #(f64! 3)) == "#(float64! [0.0 0.0 0.0])"

	--test-- "Compact construction syntax (data)"
		--assert (mold #(i8!  [1 2])) == "#(int8! [1 2])"
		--assert (mold #(i16! [1 2])) == "#(int16! [1 2])"
		--assert (mold #(i32! [1 2])) == "#(int32! [1 2])"
		--assert (mold #(i64! [1 2])) == "#(int64! [1 2])"
		--assert (mold #(u8!  [1 2])) == "#(uint8! [1 2])"
		--assert (mold #(u16! [1 2])) == "#(uint16! [1 2])"
		--assert (mold #(u32! [1 2])) == "#(uint32! [1 2])"
		--assert (mold #(u64! [1 2])) == "#(uint64! [1 2])"
		--assert (mold #(f32! [1 2])) == "#(float32! [1.0 2.0])"
		--assert (mold #(f64! [1 2])) == "#(float64! [1.0 2.0])"

	--test-- "Compact construction syntax (data with index)"
		--assert (mold v: #(i8!  [1 2] 2)) == "#(int8! [2])"
		--assert 2 = index? v
		--assert (mold v: #(i16! [1 2] 2)) == "#(int16! [2])"
		--assert 2 = index? v
		--assert (mold v: #(i32! [1 2] 2)) == "#(int32! [2])"
		--assert 2 = index? v
		--assert (mold v: #(i64! [1 2] 2)) == "#(int64! [2])"
		--assert 2 = index? v
		--assert (mold v: #(u8!  [1 2] 2)) == "#(uint8! [2])"
		--assert 2 = index? v
		--assert (mold v: #(u16! [1 2] 2)) == "#(uint16! [2])"
		--assert 2 = index? v
		--assert (mold v: #(u32! [1 2] 2)) == "#(uint32! [2])"
		--assert 2 = index? v
		--assert (mold v: #(u64! [1 2] 2)) == "#(uint64! [2])"
		--assert 2 = index? v
		--assert (mold v: #(f32!  [1 2] 2)) == "#(float32! [2.0])"
		--assert 2 = index? v
		--assert (mold v: #(f64!  [1 2] 2)) == "#(float64! [2.0])"
		--assert 2 = index? v
===end-group===

===start-group=== "VECTOR semi-compact construction"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2396
	--test-- "Compact construction syntax (empty)"
		--assert (mold make vector! [i8! ]) == "#(int8! [])"
		--assert (mold make vector! [i16!]) == "#(int16! [])"
		--assert (mold make vector! [i32!]) == "#(int32! [])"
		--assert (mold make vector! [i64!]) == "#(int64! [])"
		--assert (mold make vector! [u8! ]) == "#(uint8! [])"
		--assert (mold make vector! [u16!]) == "#(uint16! [])"
		--assert (mold make vector! [u32!]) == "#(uint32! [])"
		--assert (mold make vector! [u64!]) == "#(uint64! [])"
		--assert (mold make vector! [f32!]) == "#(float32! [])"
		--assert (mold make vector! [f64!]) == "#(float64! [])"

	--test-- "Compact construction syntax (empty, long names)"
		--assert (mold make vector! [int8! ])  == "#(int8! [])"
		--assert (mold make vector! [int16!])  == "#(int16! [])"
		--assert (mold make vector! [int32!])  == "#(int32! [])"
		--assert (mold make vector! [int64!])  == "#(int64! [])"
		--assert (mold make vector! [uint8! ]) == "#(uint8! [])"
		--assert (mold make vector! [byte!  ]) == "#(uint8! [])"
		--assert (mold make vector! [uint16!]) == "#(uint16! [])"
		--assert (mold make vector! [uint32!]) == "#(uint32! [])"
		--assert (mold make vector! [uint64!]) == "#(uint64! [])"
		--assert (mold make vector! [float!])  == "#(float32! [])"
		--assert (mold make vector! [double!]) == "#(float64! [])"

	--test-- "Compact construction syntax (size)"
		--assert (mold make vector! [i8!  3]) == "#(int8! [0 0 0])"
		--assert (mold make vector! [i16! 3]) == "#(int16! [0 0 0])"
		--assert (mold make vector! [i32! 3]) == "#(int32! [0 0 0])"
		--assert (mold make vector! [i64! 3]) == "#(int64! [0 0 0])"
		--assert (mold make vector! [u8!  3]) == "#(uint8! [0 0 0])"
		--assert (mold make vector! [u16! 3]) == "#(uint16! [0 0 0])"
		--assert (mold make vector! [u32! 3]) == "#(uint32! [0 0 0])"
		--assert (mold make vector! [u64! 3]) == "#(uint64! [0 0 0])"
		--assert (mold make vector! [f32! 3]) == "#(float32! [0.0 0.0 0.0])"
		--assert (mold make vector! [f64! 3]) == "#(float64! [0.0 0.0 0.0])"

	--test-- "Compact construction syntax (data)"
		--assert (mold make vector! [i8!  [1 2]]) == "#(int8! [1 2])"
		--assert (mold make vector! [i16! [1 2]]) == "#(int16! [1 2])"
		--assert (mold make vector! [i32! [1 2]]) == "#(int32! [1 2])"
		--assert (mold make vector! [i64! [1 2]]) == "#(int64! [1 2])"
		--assert (mold make vector! [u8!  [1 2]]) == "#(uint8! [1 2])"
		--assert (mold make vector! [u16! [1 2]]) == "#(uint16! [1 2])"
		--assert (mold make vector! [u32! [1 2]]) == "#(uint32! [1 2])"
		--assert (mold make vector! [u64! [1 2]]) == "#(uint64! [1 2])"
		--assert (mold make vector! [f32! [1 2]]) == "#(float32! [1.0 2.0])"
		--assert (mold make vector! [f64! [1 2]]) == "#(float64! [1.0 2.0])"

	--test-- "Compact construction syntax (data with index)"
		--assert (mold v: make vector! [i8!  [1 2] 2]) = "#(int8! [2])"
		--assert 2 = index? v
		--assert (mold v: make vector! [i16! [1 2] 2]) = "#(int16! [2])"
		--assert 2 = index? v
		--assert (mold v: make vector! [i32! [1 2] 2]) = "#(int32! [2])"
		--assert 2 = index? v
		--assert (mold v: make vector! [i64! [1 2] 2]) = "#(int64! [2])"
		--assert 2 = index? v
		--assert (mold v: make vector! [u8!  [1 2] 2]) = "#(uint8! [2])"
		--assert 2 = index? v
		--assert (mold v: make vector! [u16! [1 2] 2]) = "#(uint16! [2])"
		--assert 2 = index? v
		--assert (mold v: make vector! [u32! [1 2] 2]) = "#(uint32! [2])"
		--assert 2 = index? v
		--assert (mold v: make vector! [u64! [1 2] 2]) = "#(uint64! [2])"
		--assert 2 = index? v
		--assert (mold v: make vector! [f32! [1 2] 2]) = "#(float32! [2.0])"
		--assert 2 = index? v
		--assert (mold v: make vector! [f64! [1 2] 2]) = "#(float64! [2.0])"
		--assert 2 = index? v

	--test-- "Construction syntax"
		--assert (mold v: #(i8!  [1 2] 2)) = "#(int8! [2])"
		--assert 2 = index? v
		--assert (mold v: #(i16! [1 2] 2)) = "#(int16! [2])"
		--assert 2 = index? v
		--assert (mold v: #(i32! [1 2] 2)) = "#(int32! [2])"
		--assert 2 = index? v
		--assert (mold v: #(i64! [1 2] 2)) = "#(int64! [2])"
		--assert 2 = index? v
		--assert (mold v: #(u8!  [1 2] 2)) = "#(uint8! [2])"
		--assert 2 = index? v
		--assert (mold v: #(u16! [1 2] 2)) = "#(uint16! [2])"
		--assert 2 = index? v
		--assert (mold v: #(u32! [1 2] 2)) = "#(uint32! [2])"
		--assert 2 = index? v
		--assert (mold v: #(u64! [1 2] 2)) = "#(uint64! [2])"
		--assert 2 = index? v
		--assert (mold v: #(f32! [1 2] 2)) = "#(float32! [2.0])"
		--assert 2 = index? v
		--assert (mold v: #(f64! [1 2] 2)) = "#(float64! [2.0])"
		--assert 2 = index? v
===end-group===

===start-group=== "VECTOR math"

--test-- "VECTOR 8bit integer add/subtract"
	v: #(u8![1 2 3 4])
	--assert (v: v + 200) = #(u8![201 202 203 204])
	; the values are truncated on overflow:
	--assert (v: v + 200) = #(u8![145 146 147 148])
	--assert (v: v - 400) = #(u8![1 2 3 4])
	v: subtract (add v 10) 10
	--assert v = #(u8![1 2 3 4])
	v: 1 + v
	--assert v = #(u8![2 3 4 5])
	v: -1.0 + v
	--assert v = #(u8![1 2 3 4])

	v: #(i8![1 2 3 4])
	--assert (v: v + 125) = #(i8![126 127 -128 -127])
	--assert (v: v - 125) = #(i8![1 2 3 4])

--test-- "VECTOR 8bit integer multiply"
	v: #(u8![1 2 3 4])
	--assert (v: v * 4) = #(u8![4 8 12 16])
	; the values are truncated on overflow:
	--assert (v: v * 20) = #(u8![80 160 240 64]) ;64 = (16 * 20) - 256

	v: #(i8![1 2 3 4])
	--assert (v: v * 2.0) = #(i8![2 4 6 8])
	; the decimal is first converted to integer (2):
	--assert (v: v * 2.4) = #(i8![4 8 12 16])
	v: divide (multiply v 2) 2
	--assert v = #(i8![4 8 12 16])

--test-- "VECTOR 16bit integer multiply"
	v: #(u16![1 2 3 4])
	--assert (v: v * 4)  = #(u16![4 8 12 16])
	--assert (v: v * 20) = #(u16![80 160 240 320])
	v: multiply v 2
	--assert v = #(u16![160 320 480 640])

	v: #(u16![1 2 3 4])
	--assert (10   * v) = #(u16![10 20 30 40])
	--assert (10.0 * v) = #(u16![10 20 30 40])

	; the values are truncated on overflow:
	v: #(u16![1 2 3 4])
	--assert (v: v * 10000) = #(u16![10000 20000 30000 40000])
	--assert (v: v * 10.0)  = #(u16![34464 3392 37856 6784])

--test-- "VECTOR 16bit integer divide"
	v: #(u16![80 160 240 320])
	v: v / 20 / 2
	v: divide v 2
	--assert v = #(u16![1 2 3 4])
	--assert error? try [10 / v]
	--assert error? try [ v / 0] 

--test-- "VECTOR 32bit decimal add/subtract"
	v: #(f32![1 2 3 4])
	--assert (v: v + 200) = #(f32![201 202 203 204])
	--assert (v: v + 0.5) = #(f32![201.5 202.5 203.5 204.5])
	; notice the precision lost with 32bit decimal value:
	v: v - 0.1
	--assert 2013 = to integer! 10 * v/1 ; result is not 201.4 as would be with 64bit

--test-- "VECTOR 64bit decimal add/subtract"
	v: #(f64![1 2 3 4])
	--assert (v: v + 200) = #(f64![201 202 203 204])
	--assert (v: v + 0.5) = #(f64![201.5 202.5 203.5 204.5])
	--assert (v: v - 0.1) = #(f64![201.4 202.4 203.4 204.4])

--test-- "VECTOR 64bit decimal multiply/divide"
	v: #(f64![1 2 3 4])
	--assert (v: v * 20.5) = #(f64![20.5 41.0 61.5 82.0])
	--assert (v: v / 20.5) = #(f64![1.0 2.0 3.0 4.0])

--test-- "VECTOR math operation with vector not at head"
	v: #(i8![1 2 3 4])
	--assert (2 + skip v 2) = #(i8![5 6])
	--assert v = #(i8![1 2 3 4])

--test-- "VECTOR + vector"
	--assert (#(i8! [1 2]) + #(i8! [3 4])) = #(i8! [4 6])
	--assert (#(i16! [1 2]) + #(i16! [3 4 5])) = #(i16! [4 6])
	--assert (#(u32! [1 2]) + #(u32! [1 3 4] 2)) = #(u32! [4 6])
	--assert (#(f64! [1 1 2] 2) + #(f64! [1 3 4] 2)) = #(f64! [4 6])

--test-- "VECTOR - vector"
	--assert (#(i8! [4 6]) - #(i8! [3 4])) = #(i8! [1 2])
	--assert (#(i16! [4 6]) - #(i16! [3 4 5])) = #(i16! [1 2])
	--assert (#(u32! [4 6]) - #(u32! [1 3 4] 2)) = #(u32! [1 2])
	--assert (#(f64! [1 4 6] 2) - #(f64! [1 3 4] 2)) = #(f64! [1 2])

--test-- "VECTOR * vector"
	--assert (#(i8! [1 2]) * #(i8! [3 4])) = #(i8! [3 8])
	--assert (#(i16! [1 2]) * #(i16! [3 4 5])) = #(i16! [3 8])
	--assert (#(u32! [1 2]) * #(u32! [1 3 4] 2)) = #(u32! [3 8])
	--assert (#(f64! [1 1 2] 2) * #(f64! [1 3 4] 2)) = #(f64! [3 8])

--test-- "VECTOR / vector"
	--assert (#(i8! [10 20]) / #(i8! [2 4])) = #(i8! [5 5])
	--assert (#(i16! [10 20]) / #(i16! [2 4 5])) = #(i16! [5 5])
	--assert (#(u32! [10 20]) / #(u32! [1 2 4] 2)) = #(u32! [5 5])
	--assert (#(f64! [1 10 20] 2) / #(f64! [1 2 4] 2)) = #(f64! [5 5])

;@@ https://github.com/Oldes/Rebol-issues/issues/2524
;@@ https://github.com/Oldes/Rebol-issues/issues/2617
--test-- "VECTOR or"
	--assert (#(int8!  [1 2 3 4]) or 2) == #(int8!  [3 2 3 6])
	--assert (#(int16! [1 2 3 4]) or 2) == #(int16! [3 2 3 6])
	--assert (#(int32! [1 2 3 4]) or 2) == #(int32! [3 2 3 6])
	--assert (#(int64! [1 2 3 4]) or 2) == #(int64! [3 2 3 6])
	--assert (#(uint8!  [1 2 3 4]) or 2) == #(uint8!  [3 2 3 6])
	--assert (#(uint16! [1 2 3 4]) or 2) == #(uint16! [3 2 3 6])
	--assert (#(uint32! [1 2 3 4]) or 2) == #(uint32! [3 2 3 6])
	--assert (#(uint64! [1 2 3 4]) or 2) == #(uint64! [3 2 3 6])
	--assert all [error? e: try [#(float32! [1 2]) or 1]  e/id = 'not-related]
	--assert all [error? e: try [#(float64! [1 2]) or 1]  e/id = 'not-related]

--test-- "VECTOR and"
	--assert (#(int8!  [1 2 3 4]) and 10) == #(int8!  [0 2 2 0])
	--assert (#(int16! [1 2 3 4]) and 10) == #(int16! [0 2 2 0])
	--assert (#(int32! [1 2 3 4]) and 10) == #(int32! [0 2 2 0])
	--assert (#(int64! [1 2 3 4]) and 10) == #(int64! [0 2 2 0])
	--assert (#(uint8!  [1 2 3 4]) and 10) == #(uint8!  [0 2 2 0])
	--assert (#(uint16! [1 2 3 4]) and 10) == #(uint16! [0 2 2 0])
	--assert (#(uint32! [1 2 3 4]) and 10) == #(uint32! [0 2 2 0])
	--assert (#(uint64! [1 2 3 4]) and 10) == #(uint64! [0 2 2 0])
	--assert all [error? e: try [#(float32! [1 2]) and 1]  e/id = 'not-related]
	--assert all [error? e: try [#(float64! [1 2]) and 1]  e/id = 'not-related]

--test-- "VECTOR xor"
	--assert (#(int8!  [1 2 3 4]) xor 2) == #(int8!  [3 0 1 6])
	--assert (#(int16! [1 2 3 4]) xor 2) == #(int16! [3 0 1 6])
	--assert (#(int32! [1 2 3 4]) xor 2) == #(int32! [3 0 1 6])
	--assert (#(int64! [1 2 3 4]) xor 2) == #(int64! [3 0 1 6])
	--assert (#(uint8!  [1 2 3 4]) xor 2) == #(uint8!  [3 0 1 6])
	--assert (#(uint16! [1 2 3 4]) xor 2) == #(uint16! [3 0 1 6])
	--assert (#(uint32! [1 2 3 4]) xor 2) == #(uint32! [3 0 1 6])
	--assert (#(uint64! [1 2 3 4]) xor 2) == #(uint64! [3 0 1 6])
	--assert all [error? e: try [#(float32! [1 2]) xor 2]  e/id = 'not-related]
	--assert all [error? e: try [#(float64! [1 2]) xor 2]  e/id = 'not-related]

--test-- "VECTOR remainder"
	--assert (#(int8!  [1 2 3 4]) % 2) == #(int8!  [1 0 1 0])
	--assert (#(int16! [1 2 3 4]) % 2) == #(int16! [1 0 1 0])
	--assert (#(int32! [1 2 3 4]) % 2) == #(int32! [1 0 1 0])
	--assert (#(int64! [1 2 3 4]) % 2) == #(int64! [1 0 1 0])
	--assert (#(uint8!  [1 2 3 4]) % 2) == #(uint8!  [1 0 1 0])
	--assert (#(uint16! [1 2 3 4]) % 2) == #(uint16! [1 0 1 0])
	--assert (#(uint32! [1 2 3 4]) % 2) == #(uint32! [1 0 1 0])
	--assert (#(uint64! [1 2 3 4]) % 2) == #(uint64! [1 0 1 0])
	--assert (#(float32! [1 2 3 4]) % 2) == #(float32! [1 0 1 0])
	--assert (#(float64! [1 2 3 4]) % 2) == #(float64! [1 0 1 0])
--test-- "VECTOR remainder with zero"
	--assert all [error? e: try [#(int8! [1 2]) % 0]  e/id = 'zero-divide]
	--assert all [error? e: try [#(float32! [1 2]) % 0]  e/id = 'zero-divide]
	--assert all [error? e: try [#(float64! [1 2]) % 0]  e/id = 'zero-divide]

--test-- "VECTOR or vector"
	--assert (#(int8!  [1 2 3 4]) or #(i8! [5 6 7 8])) == #(int8! [5 6 7 12])
	--assert (#(int16! [1 2 3 4]) or #(i16! [5 6 7 8])) == #(int16! [5 6 7 12])
	--assert (#(int32! [1 2 3 4]) or #(i32! [5 6 7 8])) == #(int32! [5 6 7 12])
	--assert (#(int64! [1 2 3 4]) or #(i64! [5 6 7 8])) == #(int64! [5 6 7 12])
	--assert (#(uint8!  [1 2 3 4]) or #(u8! [5 6 7 8])) == #(uint8!  [5 6 7 12])
	--assert (#(uint16! [1 2 3 4]) or #(u16! [5 6 7 8])) == #(uint16! [5 6 7 12])
	--assert (#(uint32! [1 2 3 4]) or #(u32! [5 6 7 8])) == #(uint32! [5 6 7 12])
	--assert (#(uint64! [1 2 3 4]) or #(u64! [5 6 7 8])) == #(uint64! [5 6 7 12])
	--assert all [error? e: try [#(float32! [1 2]) or #(float32! [1 2])]  e/id = 'not-related]
	--assert all [error? e: try [#(float64! [1 2]) or #(float64! [1 2])]  e/id = 'not-related]

--test-- "VECTOR and vector"
	--assert (#(int8!  [1 2 3 4]) and #(i8! [5 6 7 8])) == #(int8!  [1 2 3 0])
	--assert (#(int16! [1 2 3 4]) and #(i16! [5 6 7 8])) == #(int16! [1 2 3 0])
	--assert (#(int32! [1 2 3 4]) and #(i32! [5 6 7 8])) == #(int32! [1 2 3 0])
	--assert (#(int64! [1 2 3 4]) and #(i64! [5 6 7 8])) == #(int64! [1 2 3 0])
	--assert (#(uint8!  [1 2 3 4]) and #(u8! [5 6 7 8])) == #(uint8!  [1 2 3 0])
	--assert (#(uint16! [1 2 3 4]) and #(u16! [5 6 7 8])) == #(uint16! [1 2 3 0])
	--assert (#(uint32! [1 2 3 4]) and #(u32! [5 6 7 8])) == #(uint32! [1 2 3 0])
	--assert (#(uint64! [1 2 3 4]) and #(u64! [5 6 7 8])) == #(uint64! [1 2 3 0])
	--assert all [error? e: try [#(float32! [1 2]) and #(float32! [1 2])]  e/id = 'not-related]
	--assert all [error? e: try [#(float64! [1 2]) and #(float64! [1 2])]  e/id = 'not-related]

--test-- "VECTOR xor vector"
	--assert (#(int8!  [1 2 3 4]) xor #(i8! [5 6 7 8])) == #(int8! [4 4 4 12])
	--assert (#(int16! [1 2 3 4]) xor #(i16! [5 6 7 8])) == #(int16! [4 4 4 12])
	--assert (#(int32! [1 2 3 4]) xor #(i32! [5 6 7 8])) == #(int32! [4 4 4 12])
	--assert (#(int64! [1 2 3 4]) xor #(i64! [5 6 7 8])) == #(int64! [4 4 4 12])
	--assert (#(uint8!  [1 2 3 4]) xor #(u8! [5 6 7 8])) == #(uint8!  [4 4 4 12])
	--assert (#(uint16! [1 2 3 4]) xor #(u16! [5 6 7 8])) == #(uint16! [4 4 4 12])
	--assert (#(uint32! [1 2 3 4]) xor #(u32! [5 6 7 8])) == #(uint32! [4 4 4 12])
	--assert (#(uint64! [1 2 3 4]) xor #(u64! [5 6 7 8])) == #(uint64! [4 4 4 12])
	--assert all [error? e: try [#(float32! [1 2]) xor #(float32! [1 2])]  e/id = 'not-related]
	--assert all [error? e: try [#(float64! [1 2]) xor #(float64! [1 2])]  e/id = 'not-related]

--test-- "VECTOR remainder vector"
	--assert (#(int8!  [1 2 3 4]) % #(i8! [2 2 2 2])) == #(int8!  [1 0 1 0])
	--assert (#(int16! [1 2 3 4]) % #(i16! [2 2 2 2])) == #(int16! [1 0 1 0])
	--assert (#(int32! [1 2 3 4]) % #(i32! [2 2 2 2])) == #(int32! [1 0 1 0])
	--assert (#(int64! [1 2 3 4]) % #(i64! [2 2 2 2])) == #(int64! [1 0 1 0])
	--assert (#(uint8!  [1 2 3 4]) % #(u8! [2 2 2 2])) == #(uint8!  [1 0 1 0])
	--assert (#(uint16! [1 2 3 4]) % #(u16! [2 2 2 2])) == #(uint16! [1 0 1 0])
	--assert (#(uint32! [1 2 3 4]) % #(u32! [2 2 2 2])) == #(uint32! [1 0 1 0])
	--assert (#(uint64! [1 2 3 4]) % #(u64! [2 2 2 2])) == #(uint64! [1 0 1 0])
	--assert (#(float32! [1 2 3 4]) % #(float32! [2 2 2 2])) == #(float32! [1 0 1 0])
	--assert (#(float64! [1 2 3 4]) % #(float64! [2 2 2 2])) == #(float64! [1 0 1 0])
===end-group===


===start-group=== "VECTOR ´minimum/maximum"
	vi08: #(i8!  [1 -2 0])
	vi16: #(i16! [1 -2 0])
	vi32: #(i32! [1 -2 0])
	vi64: #(i64! [1 -2 0])
	vu08: #(u8!  [1 2 0])
	vu16: #(u16! [1 2 0])
	vu32: #(u32! [1 2 0])
	vu64: #(u64! [1 2 0])
	vf32: #(f32! [1 -2 0])
	vf64: #(f64! [1 -2 0])
	--test-- "Find minimum of the vector"
		--assert vi08/min == -2
		--assert vi16/min == -2
		--assert vi32/min == -2
		--assert vi64/min == -2
		--assert vu08/min ==  0
		--assert vu16/min ==  0
		--assert vu32/min ==  0
		--assert vu64/min ==  0
		--assert vf32/min == -2.0
		--assert vf64/min == -2.0
		;; it can be used also full word
		--assert vi08/minimum == -2
	--test-- "Find maximum of the vector"
		--assert vi08/max == 1
		--assert vi16/max == 1
		--assert vi32/max == 1
		--assert vi64/max == 1
		--assert vu08/max == 2
		--assert vu16/max == 2
		--assert vu32/max == 2
		--assert vu64/max == 2
		--assert vf32/max == 1.0
		--assert vf64/max == 1.0
		--assert vi08/maximum == 1

	--test-- "Find min/max using query v1"
		--assert [minimum: -2   maximum: 1]   == query vi08 [minimum maximum]
		--assert [minimum: -2   maximum: 1]   == query vi16 [minimum maximum]
		--assert [minimum: -2   maximum: 1]   == query vi32 [minimum maximum]
		--assert [minimum: -2   maximum: 1]   == query vi64 [minimum maximum]
		--assert [minimum:  0   maximum: 2]   == query vu08 [minimum maximum]
		--assert [minimum:  0   maximum: 2]   == query vu16 [minimum maximum]
		--assert [minimum:  0   maximum: 2]   == query vu32 [minimum maximum]
		--assert [minimum:  0   maximum: 2]   == query vu64 [minimum maximum]
		--assert [minimum: -2.0 maximum: 1.0] == query vf32 [minimum maximum]
		--assert [minimum: -2.0 maximum: 1.0] == query vf64 [minimum maximum]
	--test-- "Find min/max using query v2"
		--assert [-2   1]   == query vi08 [:minimum :maximum]
		--assert [-2   1]   == query vi16 [:minimum :maximum]
		--assert [-2   1]   == query vi32 [:minimum :maximum]
		--assert [-2   1]   == query vi64 [:minimum :maximum]
		--assert [ 0   2]   == query vu08 [:minimum :maximum]
		--assert [ 0   2]   == query vu16 [:minimum :maximum]
		--assert [ 0   2]   == query vu32 [:minimum :maximum]
		--assert [ 0   2]   == query vu64 [:minimum :maximum]
		--assert [-2.0 1.0] == query vf32 [:minimum :maximum]
		--assert [-2.0 1.0] == query vf64 [:minimum :maximum]

	vi08: #(i8!  [])
	vi16: #(i16! [])
	vi32: #(i32! [])
	vi64: #(i64! [])
	vu08: #(u8!  [])
	vu16: #(u16! [])
	vu32: #(u32! [])
	vu64: #(u64! [])
	vf32: #(f32! [])
	vf64: #(f64! [])
	--test-- "Find minimum of the empty vector"
		--assert none? vi08/min
		--assert none? vi16/min
		--assert none? vi32/min
		--assert none? vi64/min
		--assert none? vu08/min
		--assert none? vu16/min
		--assert none? vu32/min
		--assert none? vu64/min
		--assert none? vf32/min
		--assert none? vf64/min
	--test-- "Find maximum of the empty vector"
		--assert none? vi08/max
		--assert none? vi16/max
		--assert none? vi32/max
		--assert none? vi64/max
		--assert none? vu08/max
		--assert none? vu16/max
		--assert none? vu32/max
		--assert none? vu64/max
		--assert none? vf32/max
		--assert none? vf64/max
===end-group===


===start-group=== "VECTOR statictics"
;@@ https://github.com/Oldes/Rebol-issues/issues/2648
	all-modes: [minimum maximum range sum mean median variance sample-variance population-deviation sample-deviation]
	all-get-modes: [:minimum :maximum :range :sum :mean :median :variance :sample-variance :population-deviation :sample-deviation]
	--test-- "int8! vector statictics"
	v: #(int8! [-2 -1 1 2 4])
	--assert (query v all-modes) == [
	    minimum: -2
	    maximum: 4
	    range: 6
	    sum: 4
	    mean: 0.8
	    median: 1.0
	    variance: 4.56
	    sample-variance: 5.7
	    population-deviation: 2.13541565040626
	    sample-deviation: 2.38746727726266
	]

	--assert (query v all-get-modes) == [
	    -2
	    4
	    6
	    4
	    0.8
	    1.0
	    4.56
	    5.7
	    2.13541565040626
	    2.38746727726266
	]

	--test-- "uint64! vector statictics"
	v: #(uint64! [4 9 11 12 17])
	--assert (query v all-modes) == [
	    minimum: 4
	    maximum: 17
	    range: 13
	    sum: 53
	    mean: 10.6
	    median: 11.0
	    variance: 17.84
	    sample-variance: 22.3
	    population-deviation: 4.22374241638857
	    sample-deviation: 4.72228758124704
	]

	--assert (query v all-get-modes) == [
	    4
	    17
	    13
	    53
	    10.6
	    11.0
	    17.84
	    22.3
	    4.22374241638857
	    4.72228758124704
	]

	--test-- "float64! vector statictics"
	v: #(float64! [1.62 1.72 1.64 1.7 1.78 1.64 1.65 1.64 1.66 1.74])
	--assert (query v all-modes) == [
	    minimum: 1.62
	    maximum: 1.78
	    range: 0.16
	    sum: 16.79
	    mean: 1.679
	    median: 1.655
	    variance: 0.002529
	    sample-variance: 0.00281
	    population-deviation: 0.0502891638427207
	    sample-deviation: 0.0530094331227943
	]

	--assert (query v all-get-modes) == [
	    1.62
	    1.78
	    0.16
	    16.79
	    1.679
	    1.655
	    0.002529
	    0.00281
	    0.0502891638427207
	    0.0530094331227943
	]

	--test-- "QUERY on empty vector"
	--assert (query #(u8! []) all-get-modes) == [_ _ _ _ _ _ _ _ _ _]

	--test-- "QUERY on single value vector"
	--assert (query #(u8! [1])  all-modes) == [
	    minimum: 1
	    maximum: 1
	    range: 0
	    sum: 1
	    mean: 1.0
	    median: 1.0
	    variance: 0.0
	    sample-variance: _
	    population-deviation: 0.0
	    sample-deviation: _
	]
	--assert (query #(u8! [1]) all-get-modes) == [1 1 0 1 1.0 1.0 0.0 _ 0.0 _]

===end-group===


===start-group=== "VECTOR Compare"
	--test-- "compare vectors"
	;@@  https://github.com/Oldes/Rebol-issues/issues/458
	--assert equal? (make vector! 3)(make vector! 3)
	--assert not equal? #(u16! [1 2]) #(u16! [1 2 3])
	--assert #(u16! [1 2]) = #(u16! [1 2])
	--assert #(u16! [1 2]) < #(u16! [1 2 0])
	--assert #(u16! [1 2]) < #(u16! [1 2 1])
	--assert #(u16! [1 2]) < #(u16! [2 2])
	--assert #(u16! [2 2]) > #(u16! [1 2])

	--test-- "compare vectors - i64"
		vi1: #(int64! [-1 2]) vi2: #(int64! [1 2])
		--assert vi1 < vi2
		--assert not (vi1 > vi2)
		--assert vi1 = vi1
		--assert #(i64! [9223372036854775807]) > #(i64! [9223372036854775806]) 

	--test-- "compare vectors - u64"
		vi1: #(uint64! [1 2]) vi2: #(uint64! [1 3])
		--assert vi1 < vi2
		--assert not (vi1 > vi2)
		--assert #(u64! [0#FFFFFFFFFFFFFFFF]) > #(u64! [0#FEFFFFFFFFFFFFFF])

	--test-- "compare vectors - f32"
		vf1: #(f32! [-1 2]) vf2: #(f32! [1 2])
		--assert vf1 < vf2
		--assert not (vf1 > vf2)
		--assert vf1 = vf1

	--test-- "compare vectors - f64 - negative zero"
		vz1: #(float64! [-0.0])
		vz2: #(float64! [ 0.0])
		--assert vz1 = vz2
		--assert not vz1 < vz2
		--assert not vz1 > vz2

	--test-- "compare vectors - cross-signedness at 64-bit width"
		vs: #(i64! [-1])
		vu: #(u64! [0#FFFFFFFFFFFFFFFF])  ;; UINT64_MAX — same bit pattern as -1 in two's complement
		--assert not vs = vu   ;; must NOT silently treat as equal
		--assert     vs < vu   ;; -1 is numerically far less than UINT64_MAX

		--assert #(u64! [1 2])  = #(u32! [1 2])
		--assert #(i64! [1 2])  = #(i32! [1 2])
		--assert #(i64! [1 2]) != #(i32! [1 3])
		--assert #(i64! [-1])   = #(i32! [-1])
		--assert #(i64! [-1])   < #(i32! [0])
		--assert #(i64! [-1])  != #(u32! [-1])

	--test-- "compare vectors - incompatible categories should error"
		--assert error? try [#(i64! [1 2]) = #(f64! [1.0 2.0])]


===end-group===


===start-group=== "VECTOR copy"

--test-- "COPY"
	;@@ https://github.com/Oldes/Rebol-issues/issues/463
	;@@ https://github.com/Oldes/Rebol-issues/issues/2400
	v1: #(u16! [1 2])
	v2: v1
	v3: copy v2
	--assert     same? v1 v2
	--assert not same? v1 v3
	v2/1: 3
	--assert v1/1 = 3
	--assert v3/1 = 1
	

--test-- "COPY/PART"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2399
	v: #(u16! [1 2 3 4])
	--assert           2 = length? copy/part v 2
	--assert #{01000200} = to-binary copy/part v 2
	--assert #{03000400} = to-binary copy/part skip v 2 2

===end-group===


===start-group=== "TAKE"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2714
	--test-- "take of vector!"
		v: #(i32! [10 20 30 40 50])
		--assert (take v) == 10
		--assert (take/last v) == 50
		--assert (take/part v 2) == #(i32! [20 30])
		--assert (take/part/last v 1) == #(i32! [40])
		--assert empty? v
		--assert none? take v
		--assert none? take/last v

	--test-- "take/part with count exceeding remaining length should clamp, not error"
		v: #(i32! [1 2 3])
		--assert (take/part v 100) == #(i32! [1 2 3])

	--test-- "take of vector! not at head"
		v: #(i32! [10 20 30 40 50])
		v2: skip v 2                    ; v2 view starts at "30" (index 2)
		--assert (take v2) == 30        ; default take is relative to current position, not absolute head
		--assert v2 == #(i32! [40 50])
		--assert  v == #(i32! [10 20 40 50]) ; same underlying series -- shrinks for both refs

	--test-- "take/last of vector! not at head"
		v: #(i32! [10 20 30 40 50])
		v2: skip v 3                    ; view = [40 50]
		--assert (take/last v2) == 50
		--assert v2 == #(i32! [40])
		--assert v == #(i32! [10 20 30 40])

	--test-- "take/part of vector! not at head - v1"
		v: skip #(i32! [10 20 30 40 50]) 2
		--assert (take/part v 2) == #(i32! [30 40])
		--assert v == #(i32! [50])
		--assert (head v) == #(i32! [10 20 50])

	--test-- "take/part of vector! not at head - v2"
		v: #(i32! [10 20 30 40 50])
		v2: skip v 1                    ; view = [20 30 40 50]
		--assert (take/part v2 2) == #(i32! [20 30])
		--assert v2 == #(i32! [40 50])
		--assert v == #(i32! [10 40 50])

	--test-- "take/part/last of vector! not at head"
		v: skip #(i32! [10 20 30 40 50]) 2
		--assert (take/part/last v 4) == #(i32! [30 40 50])
		--assert empty? v
		--assert (head v) == #(i32! [10 20])

	--test-- "take/part/last must not reach before the current index"
		v: #(i32! [10 20 30 40 50])
		v2: skip v 3                    ; view = [40 50], only 2 elements visible
		--assert (take/part/last v2 5) == #(i32! [40 50])   ; clamp to what's visible, not the full tail
		--assert empty? v2
		--assert v == #(i32! [10 20 30])   ; the hidden prefix [10 20 30] must survive untouched

	--test-- "take/last exactly at the visible boundary"
		v: #(i32! [10 20 30 40 50])
		v2: skip v 2                    ; view = [30 40 50], visible = 3
		--assert (take/part/last v2 3) == #(i32! [30 40 50])   ; exactly all visible elements
		--assert empty? v2
		--assert v == #(i32! [10 20])
===end-group===


===start-group=== "PICK"
	--test-- "PICK of vector!"
	;@@  https://github.com/Oldes/Rebol-issues/issues/748
	v: #(u32! [1 2 3])
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
	v: #(u32! [1 2 3])
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
	v: #(i32! [1 2 3 -1])
	--test-- "FIND-MAX vector!" --assert  3 = first find-max v
	--test-- "FIND-MIN vector!" --assert -1 = first find-min v
===end-group===


===start-group=== "SORT"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1101
	--test-- "SORT vector!"
		--assert  #(i8!  [1 2 3 4]) == sort #(i8!  [2 4 1 3])
		--assert  #(i16! [1 2 3 4]) == sort #(i16! [2 4 1 3])
		--assert  #(i32! [1 2 3 4]) == sort #(i32! [2 4 1 3])
		--assert  #(i64! [1 2 3 4]) == sort #(i64! [2 4 1 3])
		--assert  #(f32! [1 2 3 4]) == sort #(f32! [2 4 1 3])
		--assert  #(f64! [1 2 3 4]) == sort #(f64! [2 4 1 3])
	--test-- "SORT/reverse vector!"
		--assert  #(i8!  [4 3 2 1]) == sort/reverse #(i8!  [2 4 1 3])
		--assert  #(i16! [4 3 2 1]) == sort/reverse #(i16! [2 4 1 3])
		--assert  #(i32! [4 3 2 1]) == sort/reverse #(i32! [2 4 1 3])
		--assert  #(i64! [4 3 2 1]) == sort/reverse #(i64! [2 4 1 3])
		--assert  #(f32! [4 3 2 1]) == sort/reverse #(f32! [2 4 1 3])
		--assert  #(f64! [4 3 2 1]) == sort/reverse #(f64! [2 4 1 3])
	--test-- "SORT/part vector!"
		--assert  #(i8!  [1 2 4 3]) == sort/part #(i8!  [2 4 1 3]) 3
		--assert  #(i16! [1 2 4 3]) == sort/part #(i16! [2 4 1 3]) 3
		--assert  #(i32! [1 2 4 3]) == sort/part #(i32! [2 4 1 3]) 3
		--assert  #(i64! [1 2 4 3]) == sort/part #(i64! [2 4 1 3]) 3
		--assert  #(f32! [1 2 4 3]) == sort/part #(f32! [2 4 1 3]) 3
		--assert  #(f64! [1 2 4 3]) == sort/part #(f64! [2 4 1 3]) 3
	--test-- "SORT/part/reverse vector!"
		--assert  #(i8!  [4 2 1 3]) == sort/part/reverse #(i8!  [2 4 1 3]) 3
		--assert  #(i16! [4 2 1 3]) == sort/part/reverse #(i16! [2 4 1 3]) 3
		--assert  #(i32! [4 2 1 3]) == sort/part/reverse #(i32! [2 4 1 3]) 3
		--assert  #(i64! [4 2 1 3]) == sort/part/reverse #(i64! [2 4 1 3]) 3
		--assert  #(f32! [4 2 1 3]) == sort/part/reverse #(f32! [2 4 1 3]) 3
		--assert  #(f64! [4 2 1 3]) == sort/part/reverse #(f64! [2 4 1 3]) 3
	--test-- "SORT next vector!"
		--assert  #(i8!  [2 1 3 4]) == head sort next #(i8!  [2 4 1 3])
		--assert  #(i16! [2 1 3 4]) == head sort next #(i16! [2 4 1 3])
		--assert  #(i32! [2 1 3 4]) == head sort next #(i32! [2 4 1 3])
		--assert  #(i64! [2 1 3 4]) == head sort next #(i64! [2 4 1 3])
		--assert  #(f32! [2 1 3 4]) == head sort next #(f32! [2 4 1 3])
		--assert  #(f64! [2 1 3 4]) == head sort next #(f64! [2 4 1 3])
	--test-- "SORT/reverse next vector!"
		--assert  #(i8!  [2 4 3 1]) == head sort/reverse next #(i8!  [2 4 1 3])
		--assert  #(i16! [2 4 3 1]) == head sort/reverse next #(i16! [2 4 1 3])
		--assert  #(i32! [2 4 3 1]) == head sort/reverse next #(i32! [2 4 1 3])
		--assert  #(i64! [2 4 3 1]) == head sort/reverse next #(i64! [2 4 1 3])
		--assert  #(f32! [2 4 3 1]) == head sort/reverse next #(f32! [2 4 1 3])
		--assert  #(f64! [2 4 3 1]) == head sort/reverse next #(f64! [2 4 1 3])
	--test-- "SORT/part next vector!"
		--assert  #(i8!  [2 1 4 3]) == head sort/part next #(i8!  [2 4 1 3]) 2
		--assert  #(i16! [2 1 4 3]) == head sort/part next #(i16! [2 4 1 3]) 2
		--assert  #(i32! [2 1 4 3]) == head sort/part next #(i32! [2 4 1 3]) 2
		--assert  #(i64! [2 1 4 3]) == head sort/part next #(i64! [2 4 1 3]) 2
		--assert  #(f32! [2 1 4 3]) == head sort/part next #(f32! [2 4 1 3]) 2
		--assert  #(f64! [2 1 4 3]) == head sort/part next #(f64! [2 4 1 3]) 2
	--test-- "SORT/part/reverse next vector!"
		--assert  #(i8!  [2 4 1 3]) == head sort/part/reverse next #(i8!  [2 4 1 3]) 2
		--assert  #(i16! [2 4 1 3]) == head sort/part/reverse next #(i16! [2 4 1 3]) 2
		--assert  #(i32! [2 4 1 3]) == head sort/part/reverse next #(i32! [2 4 1 3]) 2
		--assert  #(i64! [2 4 1 3]) == head sort/part/reverse next #(i64! [2 4 1 3]) 2
		--assert  #(f32! [2 4 1 3]) == head sort/part/reverse next #(f32! [2 4 1 3]) 2
		--assert  #(f64! [2 4 1 3]) == head sort/part/reverse next #(f64! [2 4 1 3]) 2
	--test-- "SORT/skip vector!"
		--assert  all [
			error? e: try [sort/skip #(i8!  [2 4 1 3]) 2]
			e/id = 'feature-na
		]
	--test-- "SORT/compare vector!"
		--assert  all [
			error? e: try [sort/compare #(i8!  [2 4 1 3]) func[a b][a < b]]
			e/id = 'feature-na
		]
===end-group===


===start-group=== "Vector modification actions"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1326
	;@@ https://github.com/Oldes/Rebol-issues/issues/2527
	--test-- "APPEND vector number"
		--assert (append #(i8! [1 2]) 3) == #(i8! [1 2 3])
		--assert (append next #(i16! [1 2]) 3) == #(i16! [1 2 3])
		--assert (append #(i32! [1 2]) 3.5) == #(i32! [1 2 3])
		--assert (append/part #(i64! [1 2]) 3 2) == #(i64! [1 2 3])
		--assert (append/dup #(f32! [1 2]) 3 2) == #(f32! [1 2 3 3])
	
	--test-- "APPEND vector block"
		--assert (append #(i8! [1 2]) [3 4]) == #(i8! [1 2 3 4])
		--assert (append #(i16! [1 2]) [3.5 4.1]) == #(i16! [1 2 3 4])
		--assert (append next #(i32! [1 2]) [3 4]) == #(i32! [1 2 3 4])
		--assert (append/part #(i64! [1 2]) [3 4] 1) == #(i64! [1 2 3])
		--assert (append/part #(f32! [1 2]) [3 4] 3) == #(f32! [1 2 3 4])
		--assert (append/dup  #(f64! [1 2]) [3 4] 2) == #(f64! [1 2 3 4 3 4])

	--test-- "APPEND vector vector"
		--assert (append #(i8! [1 2]) #(i8! [3 4])) == #(i8! [1 2 3 4])
		--assert (append #(i16! [1 2]) #(f32! [3.5 4.1])) == #(i16! [1 2 3 4])
		--assert (append next #(i32! [1 2]) #(i8! [3 4])) == #(i32! [1 2 3 4])
		--assert (append/part #(i64! [1 2]) #(i8! [3 4]) 1) == #(i64! [1 2 3])
		--assert (append/part #(f32! [1 2]) #(i8! [3 4]) 3) == #(f32! [1 2 3 4])
		--assert (append/dup  #(f64! [1 2]) #(i8! [3 4]) 2) == #(f64! [1 2 3 4 3 4])

	--test-- "APPEND vector binary"
		--assert (append #(i8! [1 2]) #{0304}) == #(i8! [1 2 3 4])
		--assert (append #(i16! [1 2]) #{03000400})   == #(i16! [1 2 3 4])
		--assert (append next #(i8! [1 2]) #{0304})   == #(i8! [1 2 3 4])
		--assert (append/part #(i8! [1 2]) #{0304} 1) == #(i8! [1 2 3])
		--assert (append/part #(i8! [1 2]) #{0304} 3) == #(i8! [1 2 3 4])
		--assert (append/dup  #(i8! [1 2]) #{0304} 2) == #(i8! [1 2 3 4 3 4])
	--test-- "APPEND vector binary (invalid)"
		--assert all [
			error? e: try [append #(i16! [1 2]) #{03}]
			e/id = 'invalid-data
			e/arg1 = #{03}
		]
		--assert all [
			error? e: try [append/part #(i16! [1 2]) #{0304} 1]
			e/id = 'invalid-data
			e/arg1 = #{0304}
		]

	--test-- "INSERT vector number"
		--assert all [
			(insert v: #(i8! [1 2]) 3) == #(i8! [1 2])
			v == #(i8! [3 1 2])
		]
		--assert all [
			(insert next v: #(i8! [1 2]) 3) == #(i8! [2])
			v == #(i8! [1 3 2])
		]
		--assert all [
			(insert v: #(i8! [1 2]) 3.5) == #(i8! [1 2])
			v == #(i8! [3 1 2])
		]
		--assert all [
			(insert/part v: #(i8! [1 2]) 3 2) == #(i8! [1 2])
			v == #(i8! [3 1 2])
		]
		--assert all [
			(insert/dup v: #(i8! [1 2]) 3 2) == #(i8! [1 2])
			v == #(i8! [3 3 1 2])
		]

	--test-- "INSERT vector block"
		--assert all [
			(insert v: #(i8! [1 2]) [3 4]) == #(i8! [1 2])
			v == #(i8! [3 4 1 2])
		]
		--assert all [
			(insert v: #(i8! [1 2]) [3.5 4.1]) == #(i8! [1 2])
			v == #(i8! [3 4 1 2])
		]
		--assert all [
			(insert next v: #(i8! [1 2]) [3 4]) == #(i8! [2])
			v == #(i8! [1 3 4 2])
		]
		--assert all [
			(insert/part v: #(i8! [1 2]) [3 4] 1) == #(i8! [1 2])
			v == #(i8! [3 1 2])
		]
		--assert all [
			(insert/part v: #(i8! [1 2]) [3 4] 3) == #(i8! [1 2])
			v == #(i8! [3 4 1 2])
		]
		--assert all [
			(insert/dup v: #(i8! [1 2]) [3 4] 2) == #(i8! [1 2])
			v == #(i8! [3 4 3 4 1 2])
		]

	--test-- "INSERT vector vector"
		--assert all [
			(insert v: #(i8! [1 2]) #(i8! [3 4])) == #(i8! [1 2])
			v == #(i8! [3 4 1 2])
		]
		--assert all [
			(insert v: #(i16! [1 2]) #(f32! [3.5 4.1])) == #(i16! [1 2])
			v == #(i16! [3 4 1 2])
		]
		--assert all [
			(insert next v: #(i32! [1 2]) #(i8! [3 4])) == #(i32! [2])
			v == #(i32! [1 3 4 2])
		]
		--assert all [
			(insert/part v: #(i64! [1 2]) #(i8! [3 4]) 1) == #(i64! [1 2])
			v == #(i64! [3 1 2])
		]
		--assert all [
			(insert/part v: #(f32! [1 2]) #(i8! [3 4]) 3) == #(f32! [1 2])
			v == #(f32! [3 4 1 2])
		]
		--assert all [
			(insert/dup v: #(f64! [1 2]) #(i8! [3 4]) 2) == #(f64! [1 2])
			v == #(f64! [3 4 3 4 1 2])
		]

	--test-- "CHANGE vector number"
		--assert all [
			(change v: #(i8! [1 2]) 3) == #(i8! [2])
			v == #(i8! [3 2])
		]
		--assert all [
			(change next v: #(i8! [1 2 3]) 4) == #(i8! [3])
			v == #(i8! [1 4 3])
		]
		--assert all [
			(change/part v: #(i8! [1 2]) 3 1) == #(i8! [2])
			v == #(i8! [3 2])
		]
		--assert all [
			(change/part v: #(i8! [1 2]) 3 3) == #(i8! [])
			v == #(i8! [3])
		]
		--assert all [
			(change/dup v: #(i8! [1 2]) 3 2) == #(i8! [])
			v == #(i8! [3 3])
		]

	--test-- "CHANGE vector block"
		--assert all [
			(change v: #(i8! [1 2]) [3 4]) == #(i8! [])
			v == #(i8! [3 4])
		]
		--assert all [
			(change v: #(i8! [1 2]) [3.5 4.1]) == #(i8! [])
			v == #(i8! [3 4])
		]
		--assert all [
			(change v: #(i8! [1 2 3]) [3 4]) == #(i8! [3])
			v == #(i8! [3 4 3])
		]
		--assert all [
			(change next v: #(i8! [1 2 3]) [3 4]) == #(i8! [])
			v == #(i8! [1 3 4])
		]
		--assert all [
			(change/part v: #(i8! [1 2]) [3 4] 1) == #(i8! [2])
			v == #(i8! [3 4 2])
		]
		--assert all [
			(change/part v: #(i8! [1 2]) [3 4] 3) == #(i8! [])
			v == #(i8! [3 4])
		]
		--assert all [
			(change/dup v: #(i8! [1 2]) [3 4] 2) == #(i8! [])
			v == #(i8! [3 4 3 4])
		]
		--assert all [
			(change/dup v: #(i8! [1 2 3 4 5]) [6 7] 2) == #(i8! [5])
			v == #(i8! [6 7 6 7 5])
		]

	--test-- "CHANGE vector vector"
		--assert all [
			(change v: #(i8! [1 2]) #(i8! [3 4])) == #(i8! [])
			v == #(i8! [3 4])
		]
		--assert all [
			(change v: #(i16! [1 2]) #(f32! [3.5 4.1])) == #(i16! [])
			v == #(i16! [3 4])
		]
		--assert all [
			(change v: #(i32! [1 2 3]) #(i8! [3 4])) == #(i32! [3])
			v == #(i32! [3 4 3])
		]
		--assert all [
			(change next v: #(i64! [1 2 3]) #(i8! [3 4])) == #(i64! [])
			v == #(i64! [1 3 4])
		]
		--assert all [
			(change/part v: #(f32! [1 2]) #(i8! [3 4]) 1) == #(f32! [2])
			v == #(f32! [3 4 2])
		]
		--assert all [
			(change/part v: #(f64! [1 2]) #(i8! [3 4]) 3) == #(f64! [])
			v == #(f64! [3 4])
		]
		--assert all [
			(change/dup v: #(i8! [1 2]) #(u16! [3 4]) 2) == #(i8! [])
			v == #(i8! [3 4 3 4])
		]
		--assert all [
			(change/dup v: #(i16! [1 2 3 4 5]) #(u32! [6 7]) 2) == #(i16! [5])
			v == #(i16! [6 7 6 7 5])
		]

	--test-- "CLEAR vector"
		--assert all [
			v: #(i8! [1 2])
			(clear v) == #(i8! [])
			empty? v
		]
===end-group===

~~~end-file~~~
