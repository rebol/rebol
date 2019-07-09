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
		--assert do-codec system/codecs/text/entry 'identify #{}
		if find system/codecs 'png [
			--assert not do-codec system/codecs/png/entry 'identify #{01}
		]
		if find system/codecs 'jpeg [
			--assert not do-codec system/codecs/jpeg/entry 'identify #{01}
			--assert not do-codec system/codecs/jpeg/entry 'identify #{010203}
			bin: insert/dup make binary! 126 #{00} 126
			--assert not do-codec system/codecs/jpeg/entry 'identify bin
		]
		if find system/codecs 'gif [
			--assert not do-codec system/codecs/gif/entry 'identify #{01}
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

===start-group=== "Invalid SAVE"
	--test-- "invalid image SAVE"
		--assert error? try [save %temp.bmp [1 2]]
		--assert error? try [save %temp.png [1 2]]
		--assert error? try [save %temp.jpg [1 2]]
		--assert error? try [save %temp.bmp "foo"]
		--assert error? try [save %temp.png "foo"]
		--assert error? try [save %temp.jpg "foo"]
		--assert error? try [save %temp.bmp #{00}]
		--assert error? try [save %temp.png #{00}]
		--assert error? try [save %temp.jpg #{00}]
===end-group===

if find system/codecs 'wav [
	system/codecs/wav/verbose: 3
	===start-group=== "WAV codec"
		
		--test-- "Load WAV file"
			--assert object? snd: load %units/files/drumloop.wav
			--assert   'wave = snd/type
			--assert   44100 = snd/rate
			--assert       1 = snd/channels
			--assert      16 = snd/bits
			--assert 3097828 = checksum snd/data
			snd: none
		--test-- "Decode WAV data"
			--assert binary? bin: read %units/files/zblunk_02.wav
			--assert object? snd: decode 'WAV bin
			--assert 4283614 = checksum snd/data
			snd: none
			bin: none

	===end-group===
	system/codecs/wav/verbose: 0
]

if find system/codecs 'der [
	system/codecs/der/verbose: 2
	===start-group=== "DER codec"
		
		--test-- "Load DER file"
			--assert block? pfx: load %units/files/test.pfx
			--assert binary? try [a: pfx/sequence/sequence/4/2]
			--assert block? b: decode 'DER a
			--assert binary? try [c: b/sequence/sequence/4/2]
			--assert block? d: decode 'DER c

	===end-group===
	system/codecs/der/verbose: 0
]

if find system/codecs 'crt [
	system/codecs/crt/verbose: 3
	===start-group=== "CRT codec"
		
		--test-- "Load CRT file"
			--assert object? cert: load %units/files/google.crt
			--assert "Google Internet Authority G3" = try [cert/issuer/commonName]
			--assert block? try [key: cert/public-key/rsaEncryption]
			--assert #{010001} = try [key/2]
	===end-group===
	system/codecs/crt/verbose: 0
]

if find system/codecs 'swf [
	system/codecs/swf/verbose: 1
	===start-group=== "SWF codec"
		
		--test-- "Load SWF file"
			--assert object? swf1: load %units/files/test1-deflate.swf
			--assert object? swf2: load %units/files/test2-lzma.swf
			--assert swf1/tags = swf2/tags
			--assert swf1/header/frames = 25

		system/codecs/swf/verbose: 3
		--test-- "Load SWF file with decoding tags"
			--assert not error? try [swf1: load %units/files/test3.swf]
			--assert not error? try [swf2: load %units/files/test4-as2btn.swf]

	===end-group===
	system/codecs/swf/verbose: 0
]

if find system/codecs 'zip [
	system/codecs/zip/verbose: 3
	===start-group=== "ZIP codec"
		
		--test-- "Load ZIP file"
			--assert block? load %units/files/test-lzma.zip
			--assert block? load %units/files/test-stored.zip
			--assert block? load %units/files/test-deflate.zip

		--test-- "Decode ZIP using the codec directly"
			--assert block? data: system/codecs/zip/decode/only %units/files/test.aar [%classes.jar]
			--assert data/2/2 = 646121705
			--assert block? system/codecs/zip/decode data/2/3

	===end-group===
	system/codecs/zip/verbose: 1
]

if find system/codecs 'tar [
	system/codecs/zip/verbose: 3
	===start-group=== "TAR codec"
		
		--test-- "Load TAR file"
			--assert block? load %units/files/test.tar

		--test-- "Decode TAR using the codec directly"
			tar-decode: :system/codecs/tar/decode
			--assert block? data: tar-decode/only %units/files/test.tar %test.txt
			--assert data/2/1 = #{7465737474657374}

	===end-group===
	system/codecs/tar/verbose: 1
]

~~~end-file~~~