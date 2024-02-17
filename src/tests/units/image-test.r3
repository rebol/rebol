Rebol [
	Title:   "Rebol image test script"
	Author:  "Oldes"
	File: 	 %image-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Image"

===start-group=== "make image"
	--test-- "make image using pair"
		--assert image? img: make image! 2x2
	--test-- "make image using pair and RGB tuple"
		--assert image? img: make image! [2x2 255.0.0]
		--assert 255.0.0.255 = img/1
	--test-- "make image using pair and RGBA tuple"
		--assert image? img: make image! [2x2 255.0.0.10]
		--assert 255.0.0.10 = img/1
	--test-- "make image from gob"
	;@@ https://github.com/Oldes/Rebol-issues/issues/12
		--assert-er try [image? to image! make gob! []]
	--test-- "maximum limit"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1660
		--assert all [
			error? e: try [make image! 65536.0x1]
			e/id = 'size-limit
		]
		;@@ https://github.com/Oldes/Rebol-issues/issues/1567
		--assert all [
			error? e: try [make image! 65536x65536]
			e/id = 'size-limit
		]
	--test-- "to image from binary"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1048
		--assert all [error? e: try [to image! #{0000}]   e/id = 'bad-make-arg]
		--assert all [error? e: try [to image! #{000000}] e/id = 'bad-make-arg]
		--assert all [image? img: to image! #{00000000}          img/size = 1x1]
		--assert all [image? img: to image! #{0000000011}        img/size = 1x1]
		--assert all [image? img: to image! #{000000001111}      img/size = 1x1]
		--assert all [image? img: to image! #{0000000011111111}  img/size = 2x1]

===end-group===

===start-group=== "construct image"
	--test-- "construct image valid"
		--assert "make image! [1x1 #{FFFFFF}]" = mold #(image! 1x1)
		--assert "make image! [1x1 #{FFFFFF}]" = mold #(image! 1x1 #{FFFFFF})
		--assert "make image! [1x1 #{FFFFFF}]" = mold #(image! 1x1 #{FF})
		--assert "make image! [1x1 #{141414}]" = mold #(image! 1x1 20.20.20)
		--assert "make image! [1x1 #{141414} #{3C}]" = mold #(image! 1x1 20.20.20.60)
		--assert all [
			"make image! [1x1 #{FFFFFF} #{30}]" = mold img: #(image! 1x1 #{FFFFFF} #{30})
			#{30} = img/alpha
			#{FFFFFF} = img/rgb
			#{FFFFFF30} = img/rgba
		]
		--assert all [
			"make image! [1x1 #{}]" = mold img: #(image! 1x1 #{FFFFFF} 2)
			2 = index? img
			#{} = img/alpha
			#{} = img/rgba
			img: head img
			"make image! [1x1 #{FFFFFF}]" = mold img
			#{FF} = img/alpha
			#{FFFFFFFF} = img/rgba
		]
	--test-- "construct image invalid"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1034
		--assert error? try [load {#(image! x)}]
		--assert error? try [load {#(image! 1x-1)}]
		--assert error? try [load {#(image! 1x1 x)}]
		--assert error? try [load {#(image! 1x1 #{FF} x)}]
		--assert error? try [load {#(image! 1x1 20.20.20.60 x)}]
		--assert error? try [load {#(image! 1x1 #{FFFFFF} #{30} x)}]
		;@@ https://github.com/Oldes/Rebol-issues/issues/1037
		--assert all [error? e: try [make image! [3x2 #{000000000000000000000000000000000000} 1x0]] e/id = 'malconstruct]
		--assert all [error? e: try [load {#(image! 3x2 #{000000000000000000000000000000000000} 1x0)}] e/id = 'malconstruct]
		;@@ https://github.com/Oldes/Rebol-issues/issues/2508
		--assert datatype? try [load {#(image!)}]

===end-group===

===start-group=== "INDEX? / INDEXZ? / AT / ATZ"
	img: make image! 2x2
	--test-- "index? image!"
		--assert 1 = index? img
		--assert 2 = index? next img
		--assert 5 = index? tail img
		--assert 4 = index? skip tail img -1
	--test-- "index? at image!"
		--assert 1 = index? at img -1
		--assert 1 = index? at img 0
		--assert 1 = index? at img 1
		--assert 2 = index? at img 2
		--assert 5 = index? at img 6
		--assert 1 = index? skip at img 2 -1
	--test-- "index?/xy image!"
		--assert 1x1 = index?/xy img
		--assert 2x1 = index?/xy next img
		--assert 1x3 = index?/xy tail img
		--assert 2x2 = index?/xy skip tail img -1
	--test-- "index?/xy at image!"
		--assert 1x1 = index?/xy at img 1x1
		--assert 1x2 = index?/xy at img 1x2
		--assert 2x2 = index?/xy at img 2x2
		--assert 1x3 = index?/xy at img 20x2
		--assert 1x2 = index?/xy skip at img 2x2 -1x0
	--test-- "indexz? image!"
		--assert 0 = indexz? img
		--assert 1 = indexz? next img
		--assert 4 = indexz? tail img
		--assert 3 = indexz? skip tail img -1
	--test-- "indexz? atz image!"
		--assert 0 = indexz? atz img -1
		--assert 0 = indexz? atz img 0
		--assert 2 = indexz? atz img 2
		--assert 4 = indexz? atz img 6
		--assert 3 = indexz? skip atz img 6 -1
		--assert 1 = indexz? skip atz img 2 -1
	--test-- "indexz?/xy image!"
		--assert 0x0 = indexz?/xy img
		--assert 1x0 = indexz?/xy next img
		--assert 0x2 = indexz?/xy tail img
		--assert 1x1 = indexz?/xy skip tail img -1x0
	--test-- "indexz?/xy atz image!"
		--assert 0x0 = indexz?/xy atz img 0x0
		--assert 0x1 = indexz?/xy atz img 0x1
		--assert 1x1 = indexz?/xy atz img 1x1
		--assert 0x2 = indexz?/xy atz img 2x2
		--assert 0x2 = indexz?/xy atz img 20x2
		--assert 0x1 = indexz?/xy skip atz img 1x1 -1x0
	--test-- "mold/all image with modified index"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1037
		--assert 2x1 = index?/xy load mold/all next make image! 3x2

===end-group===

===start-group=== "FOREACH"
	--test-- "issue-1008"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1008
	--assert tuple? foreach x make image! 1x1 [break/return x]
	
	--test-- "issue-1479"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1479
	pix: copy []
	img: make image! [1x2 #{010203040506} #{0102}]
	foreach p img[ append pix p ]
	--assert img/1 = pix/1
	--assert img/2 = pix/2
	--assert img/1 = 1.2.3.1
	--assert img/2 = 4.5.6.2
===end-group===

===start-group=== "image range"
	img: make image! 2x2
	--test-- "image range (integer index)"
		--assert none = img/0
		--assert error? try [img/0: 1.2.3]
		--assert img/1 = 255.255.255.255
		--assert 1.2.3 = img/1: 1.2.3
		--assert img/2 = 255.255.255.255
		--assert 1.2.3 = img/2: 1.2.3
		--assert img/3 = 255.255.255.255
		--assert 1.2.3 = img/3: 1.2.3
		--assert img/4 = 255.255.255.255
		--assert 1.2.3 = img/4: 1.2.3
		--assert none = img/5
		--assert error? try [img/5: 1.2.3]
	img: make image! 2x2
	--test-- "image range (pair index)"
		--assert none = img/(0x0)
		--assert error? try [img/(0x0): 1.2.3]
		--assert none = img/(0x1)
		--assert error? try [img/(0x1): 1.2.3]
		--assert none = img/(1x0)
		--assert error? try [img/(1x0): 1.2.3]
		--assert img/(1x1) = 255.255.255.255
		--assert 1.2.3 = img/(1x1): 1.2.3
		--assert img/(1x2) = 255.255.255.255
		--assert 1.2.3 = img/(1x2): 1.2.3
		--assert img/(2x1) = 255.255.255.255
		--assert 1.2.3 = img/(2x1): 1.2.3
		--assert img/(2x2) = 255.255.255.255
		--assert 1.2.3 = img/(2x2): 1.2.3
		--assert none = img/(1x3)
		--assert error? try [img/(1x3): 1.2.3]
		;--assert none = img/(3x1)
		;--assert error? try [img/(3x1): 1.2.3]
		--assert none = img/(2x3)
		--assert error? try [img/(2x3): 1.2.3]
		--assert none = img/(3x2)
		--assert error? try [img/(3x2): 1.2.3]
===end-group===

===start-group=== "set alpha using path"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2343
	img: make image! 2x2
	--test-- "set alpha using path and integer"
		img/1: 0
		--assert img/1 = 255.255.255.0
		img/1: 127
		--assert img/1 = 255.255.255.127
		
	--test-- "set alpha using path and char"
		img/1: #"^@"
		--assert img/1 = 255.255.255.0
		img/1: to char! 127 
		--assert img/1 = 255.255.255.127

	--test-- "set alpha using path with out of range value"
		--assert error? try [img/1: -1]
		--assert error? try [img/1: 256]
		--assert error? try [img/1: #"^(260)"]
===end-group===

===start-group=== "adjusting tuple parts in images"
	;@@ https://github.com/Oldes/Rebol-issues/issues/505
	img: make image! 2x1
	--test-- "set R using path and integer"
		img/1/1: 10
		img/2/1: 20
		--assert 10 = img/1/1
		--assert 20 = img/2/1
	--test-- "set G using path and integer"
		img/1/2: 10
		img/2/2: 20
		--assert 10 = img/1/2
		--assert 20 = img/2/2
	--test-- "set B using path and integer"
		img/1/3: 10
		img/2/3: 20
		--assert 10 = img/1/3
		--assert 20 = img/2/3
	--test-- "set A using path and integer"
		img/1/4: 10
		img/2/4: 20
		--assert 10 = img/1/4
		--assert 20 = img/2/4
	--test-- "errors"
		--assert error? try [img/1/1/1: 10]
		--assert error? try [img/1/10:  10]
===end-group===

===start-group=== "raw image data getters"
	img: make image! 2x1 img/1: 1.2.3.100 img/2: 4.5.6.200
	tests: [
		rgba    #{01020364040506C8}
		rgbo    #{0102039B04050637}
		argb    #{64010203C8040506}
		orgb    #{9B01020337040506}
		bgra    #{03020164060504C8}
		bgro    #{0302019B06050437}
		abgr    #{64030201C8060504}
		obgr    #{9B03020137060504}
		opacity #{9B37}
		alpha   #{64C8}
	]
	foreach [format bin] tests [
		--test-- reform ["get raw image" format "data"]
			--assert img/:format = bin
	]
===end-group===

===start-group=== "raw image data setters"
	foreach [format bin] tests [
		--test-- reform ["set raw image" format "data"]
			img/1: 1.2.3
			img/:format: bin
			--assert img/1 = 1.2.3.100
	]
===end-group===


===start-group=== "image/color"

	--test-- "image/color getter"
		;-- counts average image color
		img: make image! 2x1
		--assert img/color = 255.255.255.255 ; default image color is white
		img/1: 0.0.0
		--assert img/color = 127.127.127.255
		img/2: 50.130.60.200
		--assert img/color = 25.65.30.227

	--test-- "image/color setter"
		;-- just for consistency, fills image as if /rgb would be used
		;-- alpha channel is ignored!
		; using image state from previous test..
		--assert img/rgba = #{000000FF32823CC8}
		img/color: 25.65.30.227
		--assert img/1 = 25.65.30.255
		--assert img/2 = 25.65.30.200

===end-group===

===start-group=== "change image"
	--test-- "change image by image"
		img: make image! 4x4
		change img make image! [2x2 0.0.0]
		--assert img/rgb = #{
000000000000FFFFFFFFFFFF
000000000000FFFFFFFFFFFF
FFFFFFFFFFFFFFFFFFFFFFFF
FFFFFFFFFFFFFFFFFFFFFFFF}
		change at img 2x2 make image! [2x2 220.22.22]
		--assert img/rgb = #{
000000000000FFFFFFFFFFFF
000000DC1616DC1616FFFFFF
FFFFFFDC1616DC1616FFFFFF
FFFFFFFFFFFFFFFFFFFFFFFF}
		change at img 3x3 make image! [3x3 33.33.33]
		--assert img/rgb = #{
000000000000FFFFFFFFFFFF
000000DC1616DC1616FFFFFF
FFFFFFDC1616212121212121
FFFFFFFFFFFF212121212121}
		change at img 1x4 make image! [4x4 66.166.66]
		--assert img/rgb = #{
000000000000FFFFFFFFFFFF
000000DC1616DC1616FFFFFF
FFFFFFDC1616212121212121
42A64242A64242A64242A642}

		change at img 4x1 make image! [2x1 #{AAAAAABBBBBB}]
		--assert img/rgb = #{
000000000000FFFFFFAAAAAA
000000DC1616DC1616FFFFFF
FFFFFFDC1616212121212121
42A64242A64242A64242A642}
===end-group===

===start-group=== "image pixel assignment validity"
	img: make image! 2x2
	--test-- "image pixel 3-tuple assignment"
		--assert 255.255.255 = img/1: 255.255.255
	--test-- "image pixel 4-tuple assignment"
		--assert 255.255.255.255 = img/2: 255.255.255.255
	--test-- "image pixel 5-tuple assignment"
		--assert 1.2.3.4.5 = img/3: 1.2.3.4.5
		--assert 1.2.3.4 = img/3
	--test-- "image pixel junk assignment"
		--assert error? try [img/1: "junk"]
		--assert error? try [img/1: []]
		--assert error? try [img/1: 3.14]
	--test-- "image pixel unaffected by junk assignments?"
		--assert 255.255.255.255 = img/1
===end-group===

===start-group=== "RGB - HSV conversions"
;@@ https://github.com/Oldes/Rebol-issues/issues/2342

--test-- "RGB-TO-HSV"
	--assert 36.235.134     = rgb-to-hsv 134.116.10
	--assert 36.235.134.100 = rgb-to-hsv 134.116.10.100
	--assert 0.0.134        = rgb-to-hsv 134.134.134
	--assert 42.253.134     = rgb-to-hsv 134.134.1
--test-- "HSV-TO-RGB"	
	--assert 5.9.10     = hsv-to-rgb 134.116.10
	--assert 5.9.10.100 = hsv-to-rgb 134.116.10.100
	--assert 63.123.134 = hsv-to-rgb 134.134.134
	--assert 0.1.2      = hsv-to-rgb 134.134.2

===end-group===

===start-group=== "RGB color distance"
--test-- "color-distance"
	--assert 764.83 = round/to color-distance 0.0.0 255.255.255 0.01
	--assert 569.97 = round/to color-distance 0.0.255 255.0.0 0.01
	--assert 42.04  = round/to color-distance 200.105.200 225.105.200 0.01
	--assert 41.3   = round/to color-distance 175.200.100 200.200.100 0.01
	--assert 0.0    = round/to color-distance 200.200.100 200.200.100 0.01
===end-group===

===start-group=== "Image difference"
--test-- "image-diff (same sizes)"
	i1: load %units/files/flower.png
	i2: load %units/files/flower.bmp
	;i3: load %units/files/flower.jpg ;; it looks that on macOS the raw image is different then on Windows
	i3: tint copy i2 128.128.128 50%  ;; so better use another difference
	--assert 0.00% =          image-diff i1 i2
	--assert 11.6% = round/to image-diff i1 i3 0.01%
--test-- "image-diff (different sizes)"
	i4: copy/part i3 10x10
	--assert 11.13% = round/to image-diff i1 i4 0.01%
	--assert 11.13% = round/to image-diff i4 i1 0.01%
--test-- "image-diff (min/max difference)"
	i1: make image! [2x2 0.0.0]
	i2: make image! [2x2 255.255.255]
	--assert   0% = image-diff i1 i1
	--assert 100% = image-diff i1 i2
--test-- "image-diff/part"
	i1/1: 255.255.255
	i1/2: 255.255.255
	--assert   0% = round/to image-diff/part i1 i2 0x0 1x1 1%
	--assert   0% = round/to image-diff/part i1 i2 0x0 1x3 1%
	--assert  50% = round/to image-diff/part i1 i2 0x0 1x2 1%
	--assert  50% = round/to image-diff/part i1 i2 1x0 1x2 1%
	--assert  50% = round/to image-diff/part i1 i2 2x2 -1x-2 1%
	--assert error? try [image-diff/part i1 i2 0x0 0x2] ;; size cannot have zero width or height
	--assert error? try [image-diff/part i1 i2 3x0 1x2] ;; offset out of range
	--assert error? try [image-diff/part i1 i2 0x2 1x2] 
===end-group===

===start-group=== "Tint color"

--test-- "Tint tuple"
	c: 100.200.255
	--assert 100.200.255 = tint c 128.128.128 0
	--assert 114.164.192 = tint c 128.128.128 50%
	--assert 128.128.128 = tint c 128.128.128 1.0
	--assert 178.228.255 = tint c 255.255.255 0.5
	--assert  50.100.128 = tint c 0.0.0 50%
	--assert 100.200.255 = c ;<- tuple value is not modified

--test-- "Tint image"
	i: make image! [2x1 100.200.255]
	--assert 114.164.192.255 = first  tint i 128.128.128 50% ;@@ image is being modified!
	--assert 121.146.160.255 = second tint i 128.128.128 50%

===end-group===


===start-group=== "PREMULTIPLY"

--test-- "premultiply image 1 (no alpha)"
	i: make image! [3x1 #{C864FFC864FFC864FF}]
	--assert image? premultiply i
	--assert i/rgb = #{C864FFC864FFC864FF}
--test-- "premultiply image 2"
	i: make image! [3x1 #{C864FFC864FFC864FF} #{0080FF}]
	--assert image? premultiply i ;@@ image is being modified!
	--assert i/rgb = #{000000643280C864FF}
--test-- "premultiply image 3 (input not at its head)"
	i: make image! [3x1 #{C864FFC864FFC864FF} #{0080FF}]
	--assert image? premultiply next i
	--assert i/rgb = #{000000643280C864FF}

===end-group===


===start-group=== "BLUR"
if value? 'blur [
--test--  "blur"
	i: load %units/files/flower.png
	c: checksum to binary! i 'crc32
	t: copy i
	--assert all [
		image? blur t 0
		c = checksum to binary! t 'crc32
	]
	--assert all [
		image? blur t 5
		-1700743341 = checksum to binary! t 'crc32
	]
	--assert all [
		image? blur t 5
		-583506697  = checksum to binary! t 'crc32
	]
	--assert all [
		image? blur i 100000
		1523895462  = checksum to binary! i 'crc32
	]
	t: i: none
]
===end-group===


===start-group=== "Save/load image"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2534
	if find codecs 'png [
		--test-- "save/load PNG"
			img1: make image! [2x2 255.0.0.10]
			save %units/files/test.png img1
			img2: load %units/files/test.png
			--assert not none? find [
				#{FF00000AFF00000AFF00000AFF00000A}
				#{0A00000A0A00000A0A00000A0A00000A} ;; premultiplied on macOS :/
			] to binary! img2
	]
	if find codecs 'bmp [
		--test-- "save/load BMP"
			img1: make image! [2x2 255.0.0.10]
			save %units/files/test.bmp img1
			img2: load %units/files/test.bmp
			--assert not none? find [
				#{FF00000AFF00000AFF00000AFF00000A}
				#{0A00000A0A00000A0A00000A0A00000A} ;; premultiplied on macOS :/
			] to binary! img2
	]

	if find codecs 'jpeg [
		--test-- "loading JPEG file with an unexpected property type"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2587
		--assert all [
			image? try [img: load %units/files/issue-2587.jpg] ;; no error!
			img/size = 105x150
		]
	]
===end-group===

===start-group=== "Image as a series"

--test-- "REPEAT on image"
;@@ https://github.com/Oldes/Rebol-issues/issues/996
	img: make image! 1x2
	repeat n img [n/1: 1.2.3 n/1: index? n]
	--assert img/1 = 1.2.3.1
	--assert img/2 = 1.2.3.2

--test-- "SINGLE? on image"
;@@ https://github.com/Oldes/Rebol-issues/issues/1000
	--assert     single? make image! 1x1
	--assert not single? make image! 1x2

--test-- "FIND tuple on image"
;@@ https://github.com/Oldes/Rebol-issues/issues/818
	img: make image! 2x2 img/2: 66.66.66
	--assert image? pos: find img 66.66.66
	--assert   2 = index? pos
	--assert 2x1 = index?/xy pos
	--assert image? pos: find/tail img 66.66.66
	--assert   3 = index? pos
	--assert 1x2 = index?/xy pos
	--assert   2 = index? find/only img 66.66.66
	--assert   2 = index? find/only img 66.66.66.22
	--assert none? find img 66.66.66.22
	--assert   1 = index? find/match img 255.255.255
	--assert   2 = index? find/match/tail img 255.255.255

--test-- "FIND integer on image (alpha)"
	img: make image! 2x2 img/2: 66.66.66.66
	--assert image? pos: find img 66
	--assert   2 = index? pos

--test-- "APPEND on image"
;@@ https://github.com/Oldes/Rebol-issues/issues/344
	img: make image! 2x0
	append img 170.170.170
	--assert 2x0 = img/size ; size not updated yet as the row is not full
	append img 187.187.187
	--assert 2x1 = img/size ; size was updated now
	--assert #{AAAAAABBBBBB} = img/rgb
	append img [1.1.1 2.2.2]
	--assert #{AAAAAABBBBBB010101020202} = img/rgb

--test-- "INSERT on image"
	img: make image! 2x0
	--assert 1 = index? img
	--assert 2 = index? insert img 170.170.170
	--assert 2 = index? insert img 187.187.187
	--assert 3 = index? tail img
	--assert #{BBBBBBAAAAAA} = img/rgb
	--assert 3 = index? insert img [1.1.1 2.2.2]
	--assert #{010101020202BBBBBBAAAAAA} = img/rgb

--test-- "INSERT/tail on image"
;@@ https://github.com/Oldes/Rebol-issues/issues/783
	img: make image! 2x0
	img: insert tail img 170.170.170
	--assert 2x0 = img/size ; size not updated yet as the row is not full
	--assert tail? img
	--assert empty? img/rgb
	img: insert img 187.187.187
	--assert 2x1 = img/size ; size was updated now
	--assert tail? img
	--assert empty? img/rgb
	img: head img
	--assert #{AAAAAABBBBBB} = img/rgb

--test-- "INSERT invalid"
;@@ https://github.com/Oldes/Rebol-issues/issues/817
	img: make image! 2x0
	--assert 0 = length? img
	--assert error? try [append img "a"]
	--assert 0 = length? img

--test-- "CHANGE on image"
	a: make image! [2x1 170.170.170]
	b: make image! [2x1 187.187.187]
	--assert a/rgb = #{AAAAAAAAAAAA}
	--assert b/rgb = #{BBBBBBBBBBBB}
	--assert not error? try [change a b]
	--assert a/rgb = #{BBBBBBBBBBBB}
	--assert not error? try [change at a 2 0.0.0]
	--assert a/rgb = #{BBBBBB000000}
	;change should return series just past the change
	--assert tail? change/dup a 200.200.200 2
	--assert a/rgb = #{C8C8C8C8C8C8}


===end-group===


~~~end-file~~~

;clean up
foreach file [
	%units/files/test.png
	%units/files/test.bmp
][	error? try [delete file] ]