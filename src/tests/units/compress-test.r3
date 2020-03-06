Rebol [
	Title:   "Rebol compress/decompress test script"
	Author:  "Oldes"
	File: 	 %compress-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Compress/Decompress"

data: "test test test"

===start-group=== "ZLIB compression / decompression"

	--test-- "basic compress/decompress"
		--assert  #{789C03000000000100000000} = compress ""
		--assert  data = to string! decompress compress data

	--test-- "basic compress/decompress while specifing level of compression"
		--assert (compress/level ""   0) = #{7801010000FFFF0000000100000000}
		--assert (compress/level data 0) = #{7801010E00F1FF7465737420746573742074657374293905810E000000}
		--assert  data = to string! decompress compress/level data 0
		--assert  data = to string! decompress compress/level data 1
		--assert  data = to string! decompress compress/level data 2
		--assert  data = to string! decompress compress/level data 3
		--assert  data = to string! decompress compress/level data 4
		--assert  data = to string! decompress compress/level data 5
		--assert  data = to string! decompress compress/level data 6
		--assert  data = to string! decompress compress/level data 7
		--assert  data = to string! decompress compress/level data 8
		--assert  data = to string! decompress compress/level data 9

	--test-- "basic decompression with specified uncompressed size"
		bin: compress data
		--assert  #{74657374} = decompress/size bin 4

	--test-- "compression when input is limited"
		--assert  #{74657374} = decompress compress/part data 4
		--assert  #{74657374} = decompress compress/part skip data 5 4
		--assert  #{74657374} = decompress compress/part tail data -4

===end-group===

===start-group=== "GZIP compression / decompression"

	--test-- "DEFLATE decompress"
		--assert #{74657374} = decompress/deflate #{2B492D2E01000C7E7FD804}

	--test-- "GZIP compress/decompress"
		--assert  data = to string! decompress/gzip compress/gzip data

	--test-- "GZIP compress/decompress while specifing level of compression"
		--assert (skip compress/gzip/level ""   0 10) =
			#{010000FFFF0000000000000000}
		--assert (skip compress/gzip/level data 0 10) =
			#{010E00F1FF7465737420746573742074657374026A5B230E000000}
		--assert  data = to string! decompress/gzip compress/gzip/level data 0
		--assert  data = to string! decompress/gzip compress/gzip/level data 1
		--assert  data = to string! decompress/gzip compress/gzip/level data 2
		--assert  data = to string! decompress/gzip compress/gzip/level data 3
		--assert  data = to string! decompress/gzip compress/gzip/level data 4
		--assert  data = to string! decompress/gzip compress/gzip/level data 5
		--assert  data = to string! decompress/gzip compress/gzip/level data 6
		--assert  data = to string! decompress/gzip compress/gzip/level data 7
		--assert  data = to string! decompress/gzip compress/gzip/level data 8
		--assert  data = to string! decompress/gzip compress/gzip/level data 9

	--test-- "GZIP decompression with specified uncompressed size"
		bin: compress/gzip data
		--assert  #{74657374} = decompress/gzip/size bin 4

	--test-- "GZIP compression when input is limited"
		--assert  #{74657374} = decompress/gzip compress/gzip/part data 4
		--assert  #{74657374} = decompress/gzip compress/gzip/part skip data 5 4
		--assert  #{74657374} = decompress/gzip compress/gzip/part tail data -4

	--test-- "GZIP codec"
		--assert  binary? alice: load %units/files/alice29.txt.gz
		--assert  #{37A087D23C8709E97AA45ECE662FAF3D07006A58} = checksum/method alice 'sha1
		--assert  file? save %units/files/tmp.gz alice
		--assert  binary? alice: load %units/files/tmp.gz
		--assert  #{37A087D23C8709E97AA45ECE662FAF3D07006A58} = checksum/method alice 'sha1
		delete %units/files/tmp.gz
		
===end-group===

===start-group=== "LZMA compression / decompression"
	--test-- "LZMA compress/decompress"
	either error? e: try [compress/lzma "test"][
		;-- LZMA compression is not available in current build
		--assert  'feature-na = e/id
	][	
		--assert  data = to string! decompress/lzma compress/lzma data

		--test-- "LZMA compress/decompress while specifing level of compression"
			--assert (compress/lzma/level ""   0) =
			#{5D00400000000000000000000000}
			--assert (compress/lzma/level data 0) =
			#{5D00400000003A194ACE1CFB1CD99000000E000000}
			--assert  data = to string! decompress/lzma compress/lzma/level data 0
			--assert  data = to string! decompress/lzma compress/lzma/level data 1
			--assert  data = to string! decompress/lzma compress/lzma/level data 2
			--assert  data = to string! decompress/lzma compress/lzma/level data 3
			--assert  data = to string! decompress/lzma compress/lzma/level data 4
			--assert  data = to string! decompress/lzma compress/lzma/level data 5
			--assert  data = to string! decompress/lzma compress/lzma/level data 6
			--assert  data = to string! decompress/lzma compress/lzma/level data 7
			--assert  data = to string! decompress/lzma compress/lzma/level data 8
			--assert  data = to string! decompress/lzma compress/lzma/level data 9

		--test-- "LZMA decompression with specified uncompressed size"
			bin: compress/lzma data
			--assert  #{74657374} = decompress/lzma/size bin 4

		--test-- "LZMA compression when input is limited"
			--assert  #{74657374} = decompress/lzma compress/lzma/part data 4
			--assert  #{74657374} = decompress/lzma compress/lzma/part skip data 5 4
			--assert  #{74657374} = decompress/lzma compress/lzma/part tail data -4
	]
===end-group===

===start-group=== "ENCLOAK/DECLOAK"
	--test-- "issue-48"
	;@@ https://github.com/Oldes/Rebol-issues/issues/48
	--assert (a: compress  "a") = #{789C4B04000062006201000000}
	--assert (b: encloak a "a") = #{2DD7F778DDDD45E040016E2B5E}
	--assert (c: decloak b "a") = #{789C4B04000062006201000000}
	--assert (d: decompress c ) = #{61}

===end-group===



~~~end-file~~~