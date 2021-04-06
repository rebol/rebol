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
			samples: #[si16! [0 -1000 -2000 -1000 0 1000 2000 1000 0]]
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
			--assert data/2/2 = 646121705
			--assert block? codecs/zip/decode data/2/3

		--test-- "Decode ZIP using info"
			bin: read %units/files/test-lzma.zip
			--assert block? info: codecs/zip/decode/info bin
			--assert info/1   = %xJSFL.komodoproject
			--assert info/2/1 = 18-Aug-2012/5:20:28
			data: codecs/zip/decompress-file at bin info/2/2 reduce [info/2/5 info/2/3 info/2/4]
			--assert info/2/6 = checksum data 'crc32

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

if find codecs 'JSON [
	===start-group=== "JSON codec"
	--test-- "JSON encode/decode"
		data: #(a: 1 b: #(c: 2.0) d: "^/^-")
		str: encode 'JSON data
		--assert data = decode 'JSON str
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
	===end-group===
]

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

if find codecs 'html-entities [
	===start-group=== "HTML-entities codec"
	--test-- "decode html-entities"
		test: {Test: &spades; & &#162; &lt;a&gt;&#32;and &Delta;&delta; &frac34;}
		--assert "Test: ♠ & ¢ <a> and Δδ ¾" = decode 'html-entities test
		--assert "Test: ♠ & ¢ <a> and Δδ ¾" = decode 'html-entities to binary! test

	===end-group===
]

~~~end-file~~~