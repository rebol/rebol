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
		--assert  #{37A087D23C8709E97AA45ECE662FAF3D07006A58} = checksum alice 'sha1
		--assert  file? save %units/files/tmp.gz alice
		--assert  binary? alice: load %units/files/tmp.gz
		--assert  #{37A087D23C8709E97AA45ECE662FAF3D07006A58} = checksum alice 'sha1
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

===start-group=== "PNG Pre-compression"
	bin: #{01020304050102030405}
	--test-- "FILTER 2"
		--assert #{0101030105FC02010401} = filter bin 2 'sub
		--assert #{0102020202FDFD020202} = filter bin 2 'up
		--assert #{0102030204FD00020302} = filter bin 2 'average
		--assert #{0101020102FCFD020201} = filter bin 2 'paeth
	--test-- "FILTER 5"
		--assert #{01010101010101010101} = filter bin 5 'sub
		--assert #{01020304050000000000} = filter bin 5 'up
		--assert #{01020203030101010101} = filter bin 5 'average
		--assert #{01010101010000000000} = filter bin 5 'paeth
	--test-- "FILTER 10"
		--assert #{0101010101FC01010101} = filter bin 10 'sub
		--assert #{01020304050102030405} = filter bin 10 'up
		--assert #{0102020303FF02020303} = filter bin 10 'average
		--assert #{0101010101FC01010101} = filter bin 10 'paeth
	--test-- "UNFILTER/AS 2"
		--assert bin = unfilter/as #{0101030105FC02010401} 2 'sub
		--assert bin = unfilter/as #{0102020202FDFD020202} 2 'up
		--assert bin = unfilter/as #{0102030204FD00020302} 2 'average
		--assert bin = unfilter/as #{0101020102FCFD020201} 2 'paeth
	--test-- "UNFILTER/AS 5"
		--assert bin = unfilter/as #{01010101010101010101} 5 'sub
		--assert bin = unfilter/as #{01020304050000000000} 5 'up
		--assert bin = unfilter/as #{01020203030101010101} 5 'average
		--assert bin = unfilter/as #{01010101010000000000} 5 'paeth
	--test-- "UNFILTER/AS 10"
		--assert bin = unfilter/as #{0101010101FC01010101} 10 'sub
		--assert bin = unfilter/as #{01020304050102030405} 10 'up
		--assert bin = unfilter/as #{0102020303FF02020303} 10 'average
		--assert bin = unfilter/as #{0101010101FC01010101} 10 'paeth
	--test-- "UNFILTER 2"
		--assert bin = unfilter #{01 0101 01 0301 01 05FC 01 0201 01 0401} 2
		--assert bin = unfilter #{02 0102 02 0202 02 02FD 02 FD02 02 0202} 2
		--assert bin = unfilter #{03 0102 03 0302 03 04FD 03 0002 03 0302} 2
		--assert bin = unfilter #{04 0101 04 0201 04 02FC 04 FD02 04 0201} 2
	--test-- "UNFILTER 5"
		--assert bin = unfilter #{01 0101010101 01 0101010101} 5
		--assert bin = unfilter #{02 0102030405 02 0000000000} 5
		--assert bin = unfilter #{03 0102020303 03 0101010101} 5
		--assert bin = unfilter #{04 0101010101 04 0000000000} 5
	--test-- "UNFILTER 10"
		--assert bin = unfilter #{01 0101010101FC01010101} 10
		--assert bin = unfilter #{02 01020304050102030405} 10
		--assert bin = unfilter #{03 0102020303FF02020303} 10
		--assert bin = unfilter #{04 0101010101FC01010101} 10

	bin: #{010203FF010203FF 020304FF030405FF}
	--test-- "FILTER/SKIP"
		--assert #{010203FF00000000020304FF01010100} = b1: filter/skip bin 8 'sub 4
		--assert #{010203FF010203FF0101010002020200} = b2: filter/skip bin 8 'up 4
		--assert #{010203FF010102800202038002020200} = b3: filter/skip bin 8 'average 4
		--assert #{010203FF000000000101010001010100} = b4: filter/skip bin 8 'paeth 4
	--test-- "FILTER/AS/SKIP"
		--assert bin = unfilter/as/skip :b1 8 'sub 4
		--assert bin = unfilter/as/skip :b2 8 'up 4
		--assert bin = unfilter/as/skip :b3 8 'average 4 
		--assert bin = unfilter/as/skip :b4 8 'paeth 4
===end-group===


~~~end-file~~~