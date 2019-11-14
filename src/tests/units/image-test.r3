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
	;@@ https://github.com/rebol/rebol-issues/issues/2343
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
		change at img 1x1 make image! [2x2 220.22.22]
		--assert img/rgb = #{
000000000000FFFFFFFFFFFF
000000DC1616DC1616FFFFFF
FFFFFFDC1616DC1616FFFFFF
FFFFFFFFFFFFFFFFFFFFFFFF}
		change at img 2x2 make image! [3x3 33.33.33]
		--assert img/rgb = #{
000000000000FFFFFFFFFFFF
000000DC1616DC1616FFFFFF
FFFFFFDC1616212121212121
FFFFFFFFFFFF212121212121}
		change at img 0x3 make image! [4x4 66.166.66]
		--assert img/rgb = #{
000000000000FFFFFFFFFFFF
000000DC1616DC1616FFFFFF
FFFFFFDC1616212121212121
42A64242A64242A64242A642}

		change at img 3x0 make image! [2x1 #{AAAAAABBBBBB}]
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
;@@ https://github.com/rebol/rebol-issues/issues/2342

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

===start-group=== "Save/load image"
	if find codecs 'png [
		--test-- "save/load PNG"
			img1: make image! [2x2 255.0.0.10]
			save %units/files/test.png img1
			img2: load %units/files/test.png
			--assert #{FF00000AFF00000AFF00000AFF00000A} = to binary! img2
	]
	if find codecs 'bmp [
		--test-- "save/load BMP"
			img1: make image! [2x2 255.0.0.10]
			save %units/files/test.bmp img1
			img2: load %units/files/test.bmp
			--assert #{FF00000AFF00000AFF00000AFF00000A} = to binary! img2
	]

~~~end-file~~~

;clean up
foreach file [
	%units/files/test.png
	%units/files/test.bmp
][	error? try [delete file] ]