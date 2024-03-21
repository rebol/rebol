Rebol [
	Title:   "Rebol codecs test script"
	Author:  "Oldes"
	File: 	 %codecs-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Codecs"

===start-group=== "Codec's identify"
	--test-- "encoding?"
		--assert 'text = encoding? #{}
		--assert 'text = encoding? #{6162}
		--assert do-codec codecs/text/entry 'identify #{}
		if all [
			find codecs 'png
			select codecs/png 'entry ; original native codec
		][
			--assert not do-codec codecs/png/entry 'identify #{01}
		]
		if all [
			find codecs 'jpeg
			select codecs/jpeg 'entry ; original native codec
		][
			--assert not do-codec codecs/jpeg/entry 'identify #{01}
			--assert not do-codec codecs/jpeg/entry 'identify #{010203}
			bin: insert/dup make binary! 126 #{00} 126
			--assert not do-codec codecs/jpeg/entry 'identify bin
		]
		if all [
			find codecs 'gif
			select codecs/gif 'entry ; original native codec
		][
			--assert not do-codec codecs/gif/entry 'identify #{01}
		]
===end-group===

===start-group=== "TEXT codec"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2028
	--test-- "ENCODE text"
		--assert "1 2" = encode 'text [1 2]
		--assert "1 2" = encode 'text #{312032}
	--test-- "SAVE %test.txt"
		--assert "%a %b" = load save %temp.txt [%a %b] 
		--assert [1 2]   = load save %temp.bin [1 2]
		--assert "1 2"   = load save %temp.txt [1 2]     ;-- note that result is STRING
		--assert "1 2^/" = read/string %temp.bin ;@@ should be there the newline char?!
		--assert "1 2"   = read/string %temp.txt

		--assert #{312032} = load save %temp.bin #{312032}
		--assert "#{312032}^/" = read/string %temp.bin ;@@ should be there the newline char?!
		delete %temp.bin
		delete %temp.txt

===end-group===

===start-group=== "SAVE function"
	--test-- "SAVE to binary!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1909
		--assert "1"     = load save #{} "1"
		--assert #{CAFE} = load save #{} #{CAFE}
		--assert [1 2]   = load save #{} [1 2]

	--test-- "invalid image SAVE"	
		if in codecs 'png [
			--assert error? try [save %temp.png [1 2]]
			--assert error? try [save %temp.png "foo"]
			--assert error? try [save %temp.png #{00}]
		]
		if in codecs 'bmp [
			--assert error? try [save %temp.bmp [1 2]]
			--assert error? try [save %temp.bmp "foo"]
			--assert error? try [save %temp.bmp #{00}]
		]
		if in codecs 'jpg [
			--assert error? try [save %temp.jpg [1 2]]
			--assert error? try [save %temp.jpg "foo"]
			--assert error? try [save %temp.jpg #{00}]
		]
	--test-- "SAVE/compress"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1466
		--assert all [
			binary? b: try [save/compress none [print "Hello World!"] true]
			b = #{
5245424F4C205B0A202020206F7074696F6E733A205B636F6D70726573735D0A
5D0A789C2B28CACC2B5150F248CDC9C95708CF2FCA495154E20200526B06D9}
			(load b) = [print "Hello World!"]
			object? first load/header b
		]
		--assert all [
			binary? b: try [save/compress none [print "Hello World!"] 'script]
			b = #{
5245424F4C205B0A202020206F7074696F6E733A205B636F6D70726573735D0A
5D0A3634237B654A77724B4D724D4B314651386B6A4E79636C58434D3876796B
6C52564F494341464A7242746B3D7D}
			(load b) = [print "Hello World!"]
			object? first load/header b
		]
		--assert all [
			binary? b: try [save/compress none [print "Hello World!"] false]
			b = #{789C2B28CACC2B5150F248CDC9C95708CF2FCA495154E20200526B06D9}
			(load decompress b 'zlib) = [print "Hello World!"]
		]

===end-group===


===start-group=== "TEXT codec"
;- loading *.txt file does also conversion CRLF to LF
;- it is actually same like using read/string
;@@ https://github.com/Oldes/Rebol-issues/issues/2424
--test-- "load UCS16-LE txt"
	--assert all [
		string? try [str: load %units/files/issue-2186-UTF16-LE.txt]
		11709824 = checksum str 'crc24
	]
--test-- "load UCS16-BE txt"
	--assert all [
		string? try [str: load %units/files/issue-2186-UTF16-BE.txt]
		11709824 = checksum str 'crc24
	]
--test-- "load UCS32-LE txt"
	--assert all [
		string? try [str: load %units/files/issue-2186-UTF32-LE.txt]
		11709824 = checksum str 'crc24
	]
--test-- "load UCS32-BE txt"
	--assert all [
		string? try [str: load %units/files/issue-2186-UTF32-BE.txt]
		11709824 = checksum str 'crc24
	]
--test-- "load/save issue! as .txt"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1937
	;- notice that result is a string! and not an issue!
	--assert "#00000002" = load save %tmp.txt #00000002
	--assert  #00000002  = load save %tmp.reb #00000002
--test-- "load/save block! as .txt"
	--assert {1 "aha"} = load save %tmp.txt [1 "aha"]
	--assert [1 "aha"] = load save %tmp.reb [1 "aha"]
--test-- "load/save binary! as .txt"
	--assert    "12" = load save %tmp.txt #{3132}
	--assert #{3132} = load save %tmp.reb #{3132}

--test-- "loading a binary longer than 32262 bytes"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1677
	--assert 32733 = length? do %units/files/issue-1677.txt
	--assert 32733 = length? do bin: read %units/files/issue-1677.txt
	write %tmp.reb bin
	--assert 32733 = length? do %tmp.reb


	delete %tmp.txt
	delete %tmp.reb

===end-group===

if find codecs 'wav [
	codecs/wav/verbose: 3
	===start-group=== "WAV codec"
		
		--test-- "Load WAV file"
			--assert object? snd: load %units/files/drumloop.wav
			--assert   'wave = snd/type
			--assert   44100 = snd/rate
			--assert       1 = snd/channels
			--assert      16 = snd/bits
			--assert 3097828 = checksum to-binary snd/data  'crc24
			snd: none
		--test-- "Decode WAV data"
			--assert binary? bin: read %units/files/zblunk_02.wav
			--assert object? snd: decode 'WAV bin
			--assert 4283614 = checksum to-binary snd/data 'crc24
			snd: none
			bin: none
			
		--test-- "Encode WAV"
			samples: #(i16! [0 -1000 -2000 -1000 0 1000 2000 1000 0])
			--assert binary? bin: encode 'wav :samples
			--assert object? snd: decode 'wav :bin
			--assert   'wave = snd/type
			--assert   44100 = snd/rate
			--assert       1 = snd/channels
			--assert      16 = snd/bits
			--assert samples = snd/data

	===end-group===
	codecs/wav/verbose: 0
]

if find codecs 'der [
	codecs/der/verbose: 2
	===start-group=== "DER codec"
		
		--test-- "Load DER file"
			--assert block? pfx: load %units/files/test.pfx
			--assert binary? try [a: pfx/sequence/sequence/4/2]
			--assert block? b: decode 'DER a
			--assert binary? try [c: b/sequence/sequence/4/2]
			--assert block? d: decode 'DER c

		--test-- "form-OID"
			--assert "2.5.29.17" = codecs/der/form-oid #{551D11}
			--assert "1.2.840.10045.4.3.2" = codecs/der/form-oid #{2A8648CE3D040302}

		--test-- "decode-OID"
			--assert 'subjectAltName = codecs/der/decode-oid #{551D11}
			--assert 'ecdsa-with-SHA256 = codecs/der/decode-oid #{2A8648CE3D040302}

	===end-group===
	codecs/der/verbose: 0
]

if find codecs 'crt [
	codecs/crt/verbose: 3
	===start-group=== "CRT codec"
		
		--test-- "Load CRT file"
			--assert object? cert: load %units/files/google.crt
			--assert "Google Internet Authority G3" = try [cert/issuer/commonName]
			--assert block? try [key: cert/public-key/rsaEncryption]
			--assert #{010001} = try [key/2]
			
		--test-- "Load Docker's CRT file"
			--assert object? cert: load %units/files/apiserver.crt
			--assert "kubernetes" = try [cert/issuer/commonName]
			--assert block? try [key: cert/public-key/rsaEncryption]
			--assert #{010001} = try [key/2]
	===end-group===
	codecs/crt/verbose: 0
]

import 'swf
if find codecs 'swf [
	codecs/swf/verbose: 1
	===start-group=== "SWF codec"
		
		--test-- "Load SWF file"
			--assert object? swf1: load %units/files/test1-deflate.swf
			--assert object? swf2: load %units/files/test2-lzma.swf
			--assert swf1/tags = swf2/tags
			--assert swf1/header/frames = 25

		codecs/swf/verbose: 3
		--test-- "Load SWF file with decoding tags"
			--assert not error? try [swf1: load %units/files/test3.swf]
			--assert not error? try [swf2: load %units/files/test4-as2btn.swf]

	===end-group===
	codecs/swf/verbose: 0
]

if find codecs 'zip [
	v: system/options/log/zip
	system/options/log/zip: 3
	===start-group=== "ZIP codec"
		
		--test-- "Load ZIP file"
			--assert block? load %units/files/test-lzma.zip
			--assert block? load %units/files/test-stored.zip
			--assert block? load %units/files/test-deflate.zip

		--test-- "Decode ZIP using the codec directly"
			--assert block? data: codecs/zip/decode/only %units/files/test.aar [%classes.jar]
			--assert data/2/3 = 646121705
			--assert block? codecs/zip/decode data/2/2

		--test-- "Decode ZIP using info"
			bin: read %units/files/test-lzma.zip
			--assert block? info: codecs/zip/decode/info bin
			--assert info/1   = %xJSFL.komodoproject
			--assert info/2/1 = 18-Aug-2012/5:20:28
			data: codecs/zip/decompress-file at bin info/2/2 reduce [info/2/5 info/2/3 info/2/4]
			--assert info/2/6 = checksum data 'crc32

		--test-- "Encode ZIP using encode"
			--assert binary? try [bin: encode 'ZIP [
				%empty-folder/ none
				%file-1 "Hello!"
				%file-2 "Hello, Hello, Hello, Hello, Hello!"
				%file-3 #{000102030400010203040001020304}
				%folder/file-4 [1-Jan-2021 "This file is with date!"]
				%file-5 ["Uncompressed" store] ; this file will be included uncompressed
				%file-6 ["This file have a comment" comment: "This file is not important."]
				%file-7 ["File with attributes" att-int: 1 att-ext: 2175008768]
			]]
			data: decode 'ZIP bin
			--assert all [
				data/1 = %empty-folder/
				data/3 = %file-1
				data/4/2 = #{48656C6C6F21}
				data/5 = %file-2
				data/9 = %folder/file-4
				data/10/1 = 1-Jan-2021/0:00
				"Uncompressed" = to string! second select data %file-5
				"This file is not important." = to string! select select data %file-6 'comment
				         1 = select select data %file-7 'att-int
				2175008768 = select select data %file-7 'att-ext
			]

		--test-- "Encode ZIP using directory"
			--assert not error? try [save %ico.zip %units/files/ico/]
			data: load %ico.zip
			--assert 30 = length? data ; 14 files and 1 directory
			--assert %ico/ = data/1
			; the order of files is not same across all systems, so next assert is not used
			; --assert %ico/icon_128.png = data/3
			--assert block? select data %ico/icon_128.png
			delete %ico.zip

		--test-- "Saving file to zip"
			;@@ https://github.com/Oldes/Rebol-issues/issues/2588
			write %temp.txt "Ahoj!"
			--assert all [
				not error? try [save %temp.zip %temp.txt]
				block? data: load %temp.zip
				data/1 = %temp.txt
				data/2/2 = #{41686F6A21}
			]
			delete %temp.zip
			delete %temp.txt

			make-dir %temp/
			write %temp/temp.txt "Ahoj!"
			--assert all [
				not error? try [save %temp.zip %temp/]
				block? data: load %temp.zip
				data/1 = %temp/
				none? data/2/2
				data/3 = %temp/temp.txt
				data/4/2 = #{41686F6A21}
			]
			delete %temp.zip
			delete-dir %temp/


		--test-- "Encode ZIP using wildcard"
			--assert not error? try [save %temp.zip %units/files/issue-2186*.txt]
			data: load %temp.zip
			--assert 8 = length? data ; 4 files
			--assert %issue-2186-UTF16-BE.txt = data/1
			--assert %issue-2186-UTF16-LE.txt = data/3
			delete %temp.zip


	===end-group===
	system/options/log/zip: v
]

if find codecs 'tar [
	codecs/zip/verbose: 3
	===start-group=== "TAR codec"
		
		--test-- "Load TAR file"
			--assert block? load %units/files/test.tar

		--test-- "Decode TAR using the codec directly"
			tar-decode: :codecs/tar/decode
			--assert block? data: tar-decode/only %units/files/test.tar %test.txt
			--assert data/2/1 = #{7465737474657374}

	===end-group===
	codecs/tar/verbose: 1
]

if find codecs 'unixtime [
	===start-group=== "unixtime codec"
		date: 18-Sep-2019/8:52:31+2:00
		--test-- "encode 32bit unixtime"
			--assert 1568789551  = encode 'unixtime date
			--assert 1568789551  = codecs/unixtime/encode date
			--assert  "5D81D42F" = encode/as 'unixtime date string!
			--assert #{5D81D42F} = encode/as 'unixtime date binary!
			--assert 1568789551  = encode/as 'unixtime date integer!
			--assert error? try  [ encode/as 'unixtime date url! ]

		--test-- "decode 32bit unixtime"
			--assert date = decode 'unixtime 1568789551
			--assert date = decode 'unixtime  "5D81D42F"
			--assert date = decode 'unixtime #{5D81D42F}
			--assert date = codecs/unixtime/decode 1568789551

		date: 1-1-2056/1:2:3
		--test-- "encode 64bit unixtime"
			--assert 2713914123 = encode 'unixtime date
			--assert "A1C30B0B" = encode/as 'unixtime date string!

		--test-- "decode 64bit unixtime"
			--assert date = decode 'unixtime 2713914123
			--assert date = decode 'unixtime "A1C30B0B"
	===end-group===
]

if all [
	find codecs 'ICO
	find codecs 'PNG
][
	===start-group=== "ICO codec"
	--test-- "ICO encode"
		--assert all [
			binary? bin: try [codecs/ico/encode sort wildcard %units/files/ico/ %*.png]
			#{0E7368623AD1DBD1BD94FC55B174778C} = checksum bin 'md5
		]
	--test-- "ICO decode"
		--assert all [
			block? ico: try [codecs/ico/decode %units/files/test.ico]
			ico/1/1  = 128 ico/1/2  = 32  binary? ico/1/3
			ico/2/1  = 16  ico/2/2  = 32  binary? ico/2/3
			ico/3/1  = 20  ico/3/2  = 32  binary? ico/3/3
			ico/4/1  = 24  ico/4/2  = 32  binary? ico/4/3
			ico/5/1  = 30  ico/5/2  = 32  binary? ico/5/3
			ico/6/1  = 32  ico/6/2  = 32  binary? ico/6/3
			ico/7/1  = 36  ico/7/2  = 32  binary? ico/7/3
			ico/8/1  = 40  ico/8/2  = 32  binary? ico/8/3
			ico/9/1  = 48  ico/9/2  = 32  binary? ico/9/3
			ico/10/1 = 60  ico/10/2 = 32  binary? ico/10/3
			ico/11/1 = 64  ico/11/2 = 32  binary? ico/11/3
			ico/12/1 = 72  ico/12/2 = 32  binary? ico/12/3
			ico/13/1 = 80  ico/13/2 = 32  binary? ico/13/3
			ico/14/1 = 96  ico/14/2 = 32  binary? ico/14/3
		]
		--assert all [
			image? img: try [decode 'png ico/2/3]
			16x16 = img/size
		]
	===end-group===
]

try [import 'json]
if find codecs 'JSON [
	===start-group=== "JSON codec"
	--test-- "JSON encode/decode"
		data: #[a: 1 b: #[c: 2.0] d: "^/^-"]
		str: encode 'JSON data
		--assert data = decode 'JSON str
		; Github is using "+1" and "-1" keys in the `reactions` data now
		--assert ["+1" 1] = to block! decode 'JSON {{"+1": 1}}
	--test-- "Decode unicode escaped char"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2546
		--assert [test: "a"] = to block! decode 'json {{"test": "\u0061"}}

	===end-group===
]

if find codecs 'PNG [
	system/options/log/png: 3
	===start-group=== "PNG codec"
	--test-- "png/size?"
		--assert 24x24 = codecs/png/size? read %units/files/r3.png
		--assert none?   codecs/png/size? read %units/files/test.aar

	--test-- "png/chunks"
		--assert all [
			; read PNG file as a block of chunks...
			block? blk: try [codecs/png/chunks %units/files/png-from-photoshop.png]
			10 = length? blk
			"IHDR" = to string! blk/1    13 = length? blk/2
			"pHYs" = to string! blk/3     9 = length? blk/4
			"iTXt" = to string! blk/5  2661 = length? blk/6
			"IDAT" = to string! blk/7   264 = length? blk/8
			"IEND" = to string! blk/9     0 = length? blk/10
		]
		--assert all [
			; read PNG with only specified set of chunks...
			block? blk: try [codecs/png/chunks/only %units/files/png-from-photoshop.png ["IHDR" "IDAT" "PLTE"]]
			4 = length? blk
			"IHDR" = to string! blk/1
			"IDAT" = to string! blk/3
			; the source image does not use palette, so there is no PLTE chunk
			; write filtered set of chunks back as a PNG file...
			file? try [write %new.png codecs/png/chunks blk]
			; validate, if the new PNG is loadeable
			image? img: try [load %new.png]
			img/size = 72x72
			; compare sizes of the original and the reduced version
			321  = size? %new.png
			3015 = size? %units/files/png-from-photoshop.png
		]
		try [delete %new.png]

	--test-- "png rgb image"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2495
		--assert all [
			image? img: try [load %units/files/rbgw.png]
			img/1 = 200.0.0.255
			img/2 = 0.0.200.255
			img/3 = 0.200.0.255
			img/rgb = #{C800000000C800C800FFFFFF}
		]
		--assert all [
			image? img: try [load save %new.png make image! [2x2 #{C800000000C800C800FFFFFF}]]
			img/rgb = #{C800000000C800C800FFFFFF}
		]
		try [delete %new.png]

	--test-- "encode/decode PNG"
		--assert all [
			binary? try [b: encode 'png make image! 1x1] 
			image?  try [i: decode 'png b]
			i/rgb == #{FFFFFF}
		]
		;@@ https://github.com/Oldes/Rebol-issues/issues/2503
		--assert error? try [decode 'png #{}]
	===end-group===
]

if find codecs 'QOI [
	===start-group=== "QOI codec"
	;@@ https://github.com/Oldes/Rebol3/issues/39
	--test-- "save qoi"
		img: make image! 256x256
		--assert file? try [save %test.qoi img]
	--test-- "load qoi"
		--assert equal? img try [load %test.qoi]
	--test-- "qoi/size?"
		--assert 256x256 = codecs/qoi/size? %test.qoi
		try [delete %test.qoi]
	--test-- "encode/decode QOI"
		--assert all [
			binary? try [b: encode 'qoi make image! 1x1] 
			image?  try [i: decode 'qoi b]
			i/rgb == #{FFFFFF}
		]
		;@@ https://github.com/Oldes/Rebol-issues/issues/2503
		--assert error? try [decode 'qoi #{}]
	===end-group===
]

if find codecs 'JPEG [
	===start-group=== "JPEG codec"
	--test-- "load jpeg"
		;@@ https://github.com/Oldes/Rebol-issues/issues/678
		--assert image? load %units/files/flower.jpg
		--assert image? load %units/files/flower-from-photoshop.jpg
		--assert image? load %units/files/flower-tiny.jpg
	--test-- "jpeg/size?"
		--assert 256x256 = codecs/jpeg/size? %units/files/flower.jpg
		--assert 256x256 = codecs/jpeg/size? %units/files/flower-from-photoshop.jpg
		--assert 256x256 = codecs/jpeg/size? %units/files/flower-tiny.jpg
		--assert none?     codecs/jpeg/size? %units/files/test.aar

	--test-- "encode/decode JPEG"
		--assert all [
			binary? try [b: encode 'jpeg make image! 1x1] 
			image?  try [i: decode 'jpeg b]
			i/rgb == #{FFFFFF}
		]
		;@@ https://github.com/Oldes/Rebol-issues/issues/2503
		--assert error? try [decode 'jpeg #{}]
	===end-group===
]

if find codecs 'GIF [
	===start-group=== "GIF codec"
	--test-- "gif/size?"
		--assert 256x256 = codecs/gif/size? %units/files/flower.gif
		--assert none?     codecs/gif/size? %units/files/test.aar

	--test-- "encode/decode GIF"
		--assert all [
			binary? try [b: encode 'gif make image! 1x1] 
			image?  try [i: decode 'gif b]
			i/rgb == #{FFFFFF}
		]
		;@@ https://github.com/Oldes/Rebol-issues/issues/2503
		--assert error? try [decode 'gif #{}]

	--test-- "gif rgb image"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2495
		--assert all [
			image? img: try [load %units/files/rbgw.gif]
			img/1 = 200.0.0.255
			img/2 = 0.0.200.255
			img/3 = 0.200.0.255
			img/rgb = #{C800000000C800C800FFFFFF}
		]
		--assert all [
			image? img: try [load save %new.gif make image! [2x2 #{C800000000C800C800FFFFFF}]]
			img/rgb = #{C800000000C800C800FFFFFF}
		]
		try [delete %new.gif]

if native? :image [
	--test-- "animated gif"
		--assert true == try [all [
			i1: image/load/frame %units/files/animation.gif 1
			i2: image/load/frame %units/files/animation.gif 2
			i3: image/load/frame %units/files/animation.gif 3
			#{4D99990699791F57F238C7195ABB0DE7} == checksum i1/rgba 'md5
			#{878BACEFB949C6435702F87D5B62F9FA} == checksum i2/rgba 'md5
			#{1414A649C4CF2E2D9DE3A8502A4425AA} == checksum i3/rgba 'md5
		]]
]
	===end-group===
]

if find codecs 'BMP [
	===start-group=== "BMP codec"
	--test-- "bmp/size?"
		--assert 256x256 = codecs/bmp/size? %units/files/flower.bmp
		--assert none?     codecs/bmp/size? %units/files/test.aar
	--test-- "encode/decode BMP"
		--assert all [
			binary? try [b: encode 'bmp make image! 1x1] 
			image?  try [i: decode 'bmp b]
			i/rgb == #{FFFFFF}
		]
		;@@ https://github.com/Oldes/Rebol-issues/issues/2503
		--assert error? try [decode 'bmp #{}]

	===end-group===
]

if find codecs 'DDS [
	===start-group=== "DDS codec"
	--test-- "dds/size?"
		--assert 256x256 = codecs/dds/size? %units/files/flower.dds
		--assert none?     codecs/dds/size? %units/files/test.aar
	===end-group===
]

try [import 'xml]
if find codecs 'XML [
	===start-group=== "XML codec"
	--test-- "XML decode test1"
		--assert block? data: load %units/files/test1.xml
		--assert none? data/document/version
		--assert none? data/document/encoding
		--assert none? data/document/standalone
		--assert none? data/document/pubid
		--assert none? data/document/subset
		--assert data/document/doctype = "document"
		--assert data/document/sysid = "subjects.dtd"
		--assert 1 = length? data/3
		--assert data/3/1/1 = "document"
		--assert 17 = length? data/3/1/3

	--test-- "XML decode test2"
		--assert block? data: load %units/files/test2.xml
		--assert data/document/version = "1.0"
		--assert none? data/document/encoding
		--assert data/document/standalone = "no"
		--assert data/document/doctype = "HTML"
		--assert data/document/pubid = "-//W3C//DTD HTML 4.0 Transitional//EN"
		--assert data/document/sysid = "http://www.w3.org/TR/REC-html40/loose.dtd"
		--assert none? data/document/subset
		--assert 1 = length? data/3
		--assert data/3/1/1 = "HTML"
		--assert 5 = length? data/3/1/3


	===end-group===
]

try [import 'html-entities]
if find codecs 'html-entities [
	===start-group=== "HTML-entities codec"
	--test-- "decode html-entities"
		test: {Test: &spades; & &#162; &lt;a&gt;&#32;and &Delta;&delta; &frac34;}
		--assert "Test: ♠ & ¢ <a> and Δδ ¾" = decode 'html-entities test
		--assert "Test: ♠ & ¢ <a> and Δδ ¾" = decode 'html-entities to binary! test

	===end-group===
]

try [import 'quoted-printable]
if find codecs 'quoted-printable [
	was-length: :codecs/quoted-printable/max-line-length
	qp-encode: :codecs/quoted-printable/encode
	qp-decode: :codecs/quoted-printable/decode
	===start-group=== "Quoted-Printable codec"
	--test-- "encode Quoted-Printable"
		--assert (encode 'quoted-printable "Holčička") == "Hol=C4=8Di=C4=8Dka"
		--assert (encode 'quoted-printable @Holčička ) == @Hol=C4=8Di=C4=8Dka
		--assert (encode 'quoted-printable %Holčička ) == %Hol=C4=8Di=C4=8Dka
		--assert (encode 'quoted-printable #{486F6CC48D69C48D6B61}) == #{486F6C3D43343D3844693D43343D38446B61}
	--test-- "decode Quoted-Printable"
		--assert (decode 'quoted-printable "Hol=C4=8Di=C4=8Dka") == "Holčička"
		--assert (decode 'quoted-printable @Hol=C4=8Di=C4=8Dka ) == @Holčička
		--assert (decode 'quoted-printable %Hol=C4=8Di=C4=8Dka ) == %Holčička
		--assert (decode 'quoted-printable #{486F6C3D43343D3844693D43343D38446B61}) == #{486F6CC48D69C48D6B61}

	--test-- "multiline Quoted-Printable"
		codecs/quoted-printable/max-line-length: 76
		text-encoded: {J'interdis aux marchands de vanter trop leurs marchandises. Car ils se font=^M
 vite p=C3=A9dagogues et t'enseignent comme but ce qui n'est par essence qu=^M
'un moyen, et te trompant ainsi sur la route =C3=A0 suivre les voil=C3=^M
=A0 bient=C3=B4t qui te d=C3=A9gradent, car si leur musique est vulgaire il=^M
s te fabriquent pour te la vendre une =C3=A2me vulgaire.^M
   =E2=80=94=E2=80=89Antoine de Saint-Exup=C3=A9ry, Citadelle (1948)}
   		text-expected: {J'interdis aux marchands de vanter trop leurs marchandises. Car ils se font vite pédagogues et t'enseignent comme but ce qui n'est par essence qu'un moyen, et te trompant ainsi sur la route à suivre les voilà bientôt qui te dégradent, car si leur musique est vulgaire ils te fabriquent pour te la vendre une âme vulgaire.^M
   — Antoine de Saint-Exupéry, Citadelle (1948)}

		--assert text-expected == qp-decode text-encoded  
		--assert text-expected == qp-decode qp-encode text-expected

	--test-- "encode with line length limit"
		codecs/quoted-printable/max-line-length: 4
		--assert "a" = qp-encode "a"
		--assert "ab" = qp-encode "ab"
		--assert "abc" = qp-encode "abc"
		--assert "abcd" = qp-encode "abcd"
		--assert "abc=^M^/de" = qp-encode "abcde"
		--assert "abc=^M^/def" = qp-encode "abcdef"
		--assert "abc=^M^/defg" = qp-encode "abcdefg"

	--test-- "quoted-encode with spaces"
		--assert "a b" = qp-encode "a b"
		--assert "a_b" = qp-encode/no-space "a b"
		--assert "a_b" = qp-decode "a_b"
		--assert "a b" = qp-decode/space "a_b"

	===end-group===
	codecs/quoted-printable/max-line-length: :was-length
	unset 'qp-encode
	unset 'qp-decode
]


try [import 'plist]
if find codecs 'plist [
	===start-group=== "PLIST codec"		
		--test-- "Load PLIST file (XML version)"
			--assert map? data: load %units/files/Some.plist
			--assert data/AppIDName = "Test Application"
			--assert data/UUID      = "bba91992-3a72-46b3-bc5f-f7b59aa49236"
	
		--test-- "Load mobileprovision file"
			--assert all [
				map? data: load %units/files/Some.mobileprovision
				data/AppIDName = "Samorost 1"
				data/UUID      = "be387546-d90d-40cd-83e6-95eb6f5f0861"
				block? data/ProvisionedDevices
				block? data/DeveloperCertificates
				object? decode 'crt data/DeveloperCertificates/1
			]
	===end-group===
]

try [import 'mime-field]
if find codecs 'mime-field [
	===start-group=== "MIME-field codec"		
		--test-- "decode/encode mime-field"
foreach str [
	"Labels: =?UTF-8?Q?Doru=C4=8Den=C3=A9,Nep=C5=99e=C4=8Dten=C3=A9?="
	"=?UTF-8?B?d2ViIHBybyDFvmFyb8WhaWNl?="
	"=?ISO-8859-1?Q?Patrik_F=E4ltstr=F6m?= <paf@nada.kth.se>"
	"From: =?US-ASCII?Q?Keith_Moore?= <moore@cs.utk.edu>"
	"=?ISO-8859-1?B?SWYgeW91IGNhbiByZWFkIHRoaXMgeW8=?==?ISO-8859-2?B?dSB1bmRlcnN0YW5kIHRoZSBleGFtcGxlLg==?="
	{Subject: =?UTF-8?B?W0plIHZ5xb5hZG92w6FuYSBha2NlXSBVcGdyYWR1anRlLCBhYnlzdGUgbW9obGkgemHEjQ==?=
	=?UTF-8?B?w610IHBvdcW+w612YXQgc2x1xb5idSBHb29nbGUgV29ya3NwYWNl?=}
	{Subject: =?UTF-8?B?8J+OgiBLYXRlxZlpbmEgUnVzxYg=?=
 =?UTF-8?B?w6Frb3bDoSwgQ2FzYSBCZWw=?=
 =?UTF-8?B?bGEgRmF1eCBGaW5pc2hl?=
 =?UTF-8?B?cyBhIEdhYnJpZWxhIEty?=
 =?UTF-8?B?YXNvdsOhIFBldHJvdmnEhyA=?=
 =?UTF-8?B?bWFqw60gZG5lc2thIG5hcg==?=
 =?UTF-8?B?b3plbmlueQ==?=}
	{X-Gmail-Labels: =?UTF-8?Q?Archivov=C3=A1no,Kategorie:_Soci=C3=A1?=
 =?UTF-8?Q?ln=C3=AD_s=C3=ADt=C4=9B,Nep=C5=99e=C4=8Dten=C3=A9,Facebook?=}
	{Subject: =?UTF-8?Q?=C4=8D_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx?=
	=?UTF-8?Q?xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx?=
	=?UTF-8?Q?xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx?=}
	{Subject: Fwd: Cron <root@forum> test -x /usr/sbin/anacron || ( cd / &&
  run-parts --report /etc/cron.weekly )}
  {xxxxx^M^/ =?utf-8?Q?yyyy?=}
  {xxxxx^M^/ yyyy}
	"(=?ISO-8859-1?Q?a?= =?ISO-8859-1?Q?b?=)" ;"(ab)"
	"(=?ISO-8859-1?Q?a?=  =?ISO-8859-1?Q?b?=)" ;<== should be "(ab)"
	"(=?ISO-8859-1?Q?a?=^M^/ =?ISO-8859-1?Q?b?=)" ;"(ab)"
	"(=?ISO-8859-1?Q?a_b?=)" ;<= "(a b)"
	"(=?ISO-8859-1?Q?a?= =?ISO-8859-2?Q?_b?=)" ; "(a b)"
	"(=?ISO-8859-1?Q?a?= b)" ;"(a b)"
	{Subject: =?UTF-8?Q?Fwd=3A_objedn=C3=A1vka?=}
	"Subject: Hello Mail"
][
	x: decode 'mime-field :str
	y: encode 'mime-field :x
	z: decode 'mime-field :y
	--assert z = x
]
	===end-group===
]


if find codecs 'safe [
	;- using environmental variable to avoid interactive password input using `ask`
	temp: get-env "REBOL_SAFE_PASS"
	set-env "REBOL_SAFE_PASS" "my-pass"
	user: system/user/name ;; store existing user
	===start-group=== "SAFE codec"		
		--test-- "Save/Load SAFE file"
			foreach data [
				#[key: "Hello"]
				43
				#{DEADBEEF}
				[key: "aaa" value: 12]
			][
				--assert equal? data load save %temp.safe data
				delete %temp.safe
			]
		--test-- "Set-user which does not exists"
			;@@ https://github.com/Oldes/Rebol-issues/issues/2547
			--assert not error? try [set-user not-existing-user]
		--test-- "Initialise new user"
			--assert not error? try [set-user/n/p temp-user "passw"]
			--assert system/user/name = @temp-user
			--assert 'file = exists? try [system/user/data/spec/ref]
			--assert "hello" = put system/user/data 'key "hello"  ;; store some data...
			--assert "hello" = user's key               ;; resolve the data
			--assert not error? try [su]                ;; release user
			--assert none? system/user/name
			--assert not error? try [set-user/p temp-user "passw"]
			--assert "hello" = user's key               ;; resolve the data
			--assert not error? try [su #(none)]        ;; release user using none value
			--assert none? system/user/name

			try [delete system/user/data/spec/ref]

	===end-group===
	set-env "REBOL_SAFE_PASS" :temp
	set-user :user
]


;@@ PDF codec test is in: codecs-test-pdf.r3

~~~end-file~~~