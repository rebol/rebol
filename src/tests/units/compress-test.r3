Rebol [
	Title:   "Rebol compress/decompress test script"
	Author:  "Oldes"
	File: 	 %compress-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Compress/Decompress"

data: "test test test"
text: {Lorem ipsum dolor sit amet, consectetur adipisici elit, sed eiusmod tempor incidunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquid ex ea commodi consequat. Quis aute iure reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint obcaecat cupiditat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.}

===start-group=== "ZLIB compression / decompression"

	--test-- "basic compress/decompress"
		--assert  #{789C030000000001} = compress "" 'zlib
		--assert  data = to string! decompress compress data 'zlib 'zlib

	--test-- "basic compress/decompress while specifing level of compression"
		--assert (compress/level ""   'zlib 0) = #{7801010000FFFF00000001}
		--assert (compress/level data 'zlib 0) = #{7801010E00F1FF746573742074657374207465737429390581}
		--assert  text = to string! decompress compress/level text 'zlib 0 'zlib
		--assert  text = to string! decompress compress/level text 'zlib 1 'zlib
		--assert  text = to string! decompress compress/level text 'zlib 2 'zlib
		--assert  text = to string! decompress compress/level text 'zlib 3 'zlib
		--assert  text = to string! decompress compress/level text 'zlib 4 'zlib
		--assert  text = to string! decompress compress/level text 'zlib 5 'zlib
		--assert  text = to string! decompress compress/level text 'zlib 6 'zlib
		--assert  text = to string! decompress compress/level text 'zlib 7 'zlib
		--assert  text = to string! decompress compress/level text 'zlib 8 'zlib
		--assert  text = to string! decompress compress/level text 'zlib 9 'zlib

	--test-- "basic decompression with specified uncompressed size"
		bin: compress data 'zlib
		--assert  #{74657374} = decompress/size bin 'zlib 4

	--test-- "compression when input is limited"
		--assert  #{74657374} = decompress compress/part      data   'zlib  4 'zlib
		--assert  #{74657374} = decompress compress/part skip data 5 'zlib  4 'zlib
		--assert  #{74657374} = decompress compress/part tail data   'zlib -4 'zlib

	--test-- "decompress when not at head"
		--assert data = to string! decompress next join #{00} compress data 'zlib 'zlib

===end-group===

===start-group=== "DEFLATE compression / decompression"

	--test-- "basic compress/decompress"
		--assert  #{0300} = compress "" 'deflate
		--assert  data = to string! decompress compress data 'deflate 'deflate

	--test-- "basic compress/decompress while specifing level of compression"
		--assert (compress/level ""   'deflate 0) = #{010000FFFF}
		--assert (compress/level data 'deflate 0) = #{010E00F1FF7465737420746573742074657374}
		--assert  text = to string! decompress compress/level text 'deflate 0 'deflate
		--assert  text = to string! decompress compress/level text 'deflate 1 'deflate
		--assert  text = to string! decompress compress/level text 'deflate 2 'deflate
		--assert  text = to string! decompress compress/level text 'deflate 3 'deflate
		--assert  text = to string! decompress compress/level text 'deflate 4 'deflate
		--assert  text = to string! decompress compress/level text 'deflate 5 'deflate
		--assert  text = to string! decompress compress/level text 'deflate 6 'deflate
		--assert  text = to string! decompress compress/level text 'deflate 7 'deflate
		--assert  text = to string! decompress compress/level text 'deflate 8 'deflate
		--assert  text = to string! decompress compress/level text 'deflate 9 'deflate

	--test-- "basic decompression with specified uncompressed size"
		bin: compress data 'deflate
		--assert  #{74657374} = decompress/size bin 'deflate 4

	--test-- "compression when input is limited"
		--assert  #{74657374} = decompress compress/part      data   'deflate  4 'deflate
		--assert  #{74657374} = decompress compress/part skip data 5 'deflate  4 'deflate
		--assert  #{74657374} = decompress compress/part tail data   'deflate -4 'deflate

	--test-- "decompress when not at head"
		--assert data = to string! decompress next join #{00} compress data 'deflate 'deflate

===end-group===

===start-group=== "GZIP compression / decompression"

	--test-- "GZIP compress/decompress"
		--assert  data = to string! decompress compress data 'gzip 'gzip

	--test-- "GZIP decompress when not at head"
		--assert data = to string! decompress next join #{00} compress data 'gzip 'gzip

	--test-- "GZIP compress/decompress while specifing level of compression"
		--assert (skip compress/level  ""  'gzip 0 10) =
			#{010000FFFF0000000000000000}
		--assert (skip compress/level data 'gzip 0 10) =
			#{010E00F1FF7465737420746573742074657374026A5B230E000000}
		--assert  text = to string! decompress compress/level text 'gzip 0 'gzip
		--assert  text = to string! decompress compress/level text 'gzip 1 'gzip
		--assert  text = to string! decompress compress/level text 'gzip 2 'gzip
		--assert  text = to string! decompress compress/level text 'gzip 3 'gzip
		--assert  text = to string! decompress compress/level text 'gzip 4 'gzip
		--assert  text = to string! decompress compress/level text 'gzip 5 'gzip
		--assert  text = to string! decompress compress/level text 'gzip 6 'gzip
		--assert  text = to string! decompress compress/level text 'gzip 7 'gzip
		--assert  text = to string! decompress compress/level text 'gzip 8 'gzip
		--assert  text = to string! decompress compress/level text 'gzip 9 'gzip

	--test-- "GZIP decompression with specified uncompressed size"
		bin: compress data 'gzip
		--assert  #{74657374} = decompress/size bin 'gzip 4

	--test-- "GZIP compression when input is limited"
		--assert  #{74657374} = decompress compress/part      data   'gzip  4 'gzip
		--assert  #{74657374} = decompress compress/part skip data 5 'gzip  4 'gzip
		--assert  #{74657374} = decompress compress/part tail data   'gzip -4 'gzip

	--test-- "GZIP codec"
		--assert  binary? alice: load %units/files/alice29.txt.gz
		--assert  #{37A087D23C8709E97AA45ECE662FAF3D07006A58} = checksum alice 'sha1
		--assert  file? save %units/files/tmp.gz alice
		--assert  binary? alice: load %units/files/tmp.gz
		--assert  #{37A087D23C8709E97AA45ECE662FAF3D07006A58} = checksum alice 'sha1
		delete %units/files/tmp.gz
		
===end-group===

===start-group=== "Brotli compression / decompression"
	--test-- "Brotli compress/decompress"
	either error? e: try [compress "test" 'brotli][
		;-- Brotli compression is not available in current build
		--assert  'feature-na = e/id
	][	
		--assert  data = to string! decompress compress data 'brotli 'brotli

		--test-- "Brotli compress/decompress while specifing level of compression"
			--assert (compress/level "" 'brotli 0) =
			#{3B}
			--assert (compress/level data 'brotli 0) =
			#{8B0680746573742074657374207465737403}
			--assert  text = to string! decompress compress/level text 'brotli 0 'brotli
			--assert  text = to string! decompress compress/level text 'brotli 1 'brotli
			--assert  text = to string! decompress compress/level text 'brotli 2 'brotli
			--assert  text = to string! decompress compress/level text 'brotli 3 'brotli
			--assert  text = to string! decompress compress/level text 'brotli 4 'brotli
			--assert  text = to string! decompress compress/level text 'brotli 5 'brotli
			--assert  text = to string! decompress compress/level text 'brotli 6 'brotli
			--assert  text = to string! decompress compress/level text 'brotli 7 'brotli
			--assert  text = to string! decompress compress/level text 'brotli 8 'brotli
			--assert  text = to string! decompress compress/level text 'brotli 9 'brotli
			--assert  text = to string! decompress compress/level text 'brotli 10 'brotli
			--assert  text = to string! decompress compress/level text 'brotli 11 'brotli

		--test-- "Brotli decompression with specified uncompressed size"
			bin: compress data 'brotli
			--assert  #{74657374} = decompress/size bin 'brotli 4

		--test-- "Brotli compression when input is limited"
			--assert  #{74657374} = decompress compress/part      data   'brotli  4 'brotli
			--assert  #{74657374} = decompress compress/part skip data 5 'brotli  4 'brotli
			--assert  #{74657374} = decompress compress/part tail data   'brotli -4 'brotli

		--test-- "Brotli decompress when not at head"
			--assert data = to string! decompress next join #{00} compress data 'brotli 'brotli
	]
===end-group===

===start-group=== "LZMA compression / decompression"
	--test-- "LZMA compress/decompress"
	either error? e: try [compress "test" 'lzma][
		;-- LZMA compression is not available in current build
		--assert  'feature-na = e/id
	][	
		--assert  data = to string! decompress compress data 'lzma 'lzma

		--test-- "LZMA compress/decompress while specifing level of compression"
			--assert (compress/level "" 'lzma 0) =
			#{5D00400000000000000000000000}
			--assert (compress/level data 'lzma 0) =
			#{5D00400000003A194ACE1CFB1CD99000000E000000}
			--assert  text = to string! decompress compress/level text 'lzma 0 'lzma
			--assert  text = to string! decompress compress/level text 'lzma 1 'lzma
			--assert  text = to string! decompress compress/level text 'lzma 2 'lzma
			--assert  text = to string! decompress compress/level text 'lzma 3 'lzma
			--assert  text = to string! decompress compress/level text 'lzma 4 'lzma
			--assert  text = to string! decompress compress/level text 'lzma 5 'lzma
			--assert  text = to string! decompress compress/level text 'lzma 6 'lzma
			--assert  text = to string! decompress compress/level text 'lzma 7 'lzma
			--assert  text = to string! decompress compress/level text 'lzma 8 'lzma
			--assert  text = to string! decompress compress/level text 'lzma 9 'lzma

		--test-- "LZMA decompression with specified uncompressed size"
			bin: compress data 'lzma
			--assert  #{74657374} = decompress/size bin 'lzma 4

		--test-- "LZMA compression when input is limited"
			--assert  #{74657374} = decompress compress/part      data   'lzma  4 'lzma
			--assert  #{74657374} = decompress compress/part skip data 5 'lzma  4 'lzma
			--assert  #{74657374} = decompress compress/part tail data   'lzma -4 'lzma

		--test-- "LZMA decompress when not at head"
			--assert data = to string! decompress next join #{00} compress data 'lzma 'lzma
	]
===end-group===

===start-group=== "LZW compression / decompression"
	--test-- "LZW compress/decompress"
	either error? e: try [compress "test" 'lzw][
		;-- LZW compression is not available in current build
		--assert  'feature-na = e/id
	][	
		--assert  data = to string! decompress compress data 'lzw 'lzw

		--test-- "LZW compress/decompress while specifing level of compression"
			--assert (compress/level "" 'lzw 0) =
			#{00FF01}
			--assert (compress/level data 'lzw 0) =
			#{007465737420FDFAFBE347F71F}
			--assert  text = to string! decompress compress/level text 'lzw 0 'lzw
			--assert  text = to string! decompress compress/level text 'lzw 1 'lzw
			--assert  text = to string! decompress compress/level text 'lzw 2 'lzw
			--assert  text = to string! decompress compress/level text 'lzw 3 'lzw
			--assert  text = to string! decompress compress/level text 'lzw 4 'lzw
			--assert  text = to string! decompress compress/level text 'lzw 5 'lzw
			--assert  text = to string! decompress compress/level text 'lzw 6 'lzw
			--assert  text = to string! decompress compress/level text 'lzw 7 'lzw
			--assert  text = to string! decompress compress/level text 'lzw 8 'lzw
			--assert  text = to string! decompress compress/level text 'lzw 9 'lzw

		--test-- "LZW decompression with specified uncompressed size"
			bin: compress data 'lzw
			--assert  #{74657374} = decompress/size bin 'lzw 4

		--test-- "LZW compression when input is limited"
			--assert  #{74657374} = decompress compress/part      data   'lzw  4 'lzw
			--assert  #{74657374} = decompress compress/part skip data 5 'lzw  4 'lzw
			--assert  #{74657374} = decompress compress/part tail data   'lzw -4 'lzw

		--test-- "LZW decompress when not at head"
			--assert data = to string! decompress next join #{00} compress data 'lzw 'lzw
	]
===end-group===

===start-group=== "CRUSH compression / decompression"
	--test-- "CRUSH compress/decompress"
	either error? e: try [compress "test" 'crush][
		;-- CRUSH compression is not available in current build
		--assert  'feature-na = e/id
	][	
		--assert  data = to string! decompress compress data 'crush 'crush

		--test-- "CRUSH compress/decompress while specifing level of compression"
			--assert (compress/level ""   'crush 0) = #{00000000}
			--assert (compress/level data 'crush 0) = #{0E000000E894994307A40201}
			--assert  text = to string! decompress compress/level text 'crush 0 'crush
			--assert  text = to string! decompress compress/level text 'crush 1 'crush
			--assert  text = to string! decompress compress/level text 'crush 2 'crush

		--test-- "CRUSH decompression with specified uncompressed size"
			bin: compress data 'crush
			--assert  #{74657374} = decompress/size bin 'crush 4

		--test-- "CRUSH compression when input is limited"
			--assert  #{74657374} = decompress compress/part      data   'crush  4 'crush
			--assert  #{74657374} = decompress compress/part skip data 5 'crush  4 'crush
			--assert  #{74657374} = decompress compress/part tail data   'crush -4 'crush

		--test-- "CRUSH decompress when not at head"
			--assert data = to string! decompress next join #{00} compress data 'crush 'crush

		--test-- "CRUSH with bad compression ratio"
			; CRUSH algorithm does not work well if there is no repeatable pattern
			; These asserts also validates, that there is no memory coruption as internal extension is used
			--assert #{0400000062C8984103} = compress "1234" 'crush
			--assert #{0800000062C89841A3868D1B38} = compress "12345678" 'crush
			--assert #{0D00000062C89841A3868D1B38720411328408} = compress "123456789ABCD" 'crush
			--assert #{1100000062C89841A3868D1B387204113284481123479000} = compress "123456789ABCDEFGH" 'crush
			--assert #{1500000062C89841A3868D1B38720411328448112347902451B28409} = compress "123456789ABCDEFGHIJKL" 'crush
	]
===end-group===

===start-group=== "ENCLOAK/DECLOAK"
	--test-- "issue-48"
	;@@ https://github.com/Oldes/Rebol-issues/issues/48
	--assert (a: compress  "a" 'zlib) = #{789C4B040000620062}
	--assert (b: encloak a "a")       = #{2CD6F679DCDC44E141}
	--assert (c: decloak b "a")       = #{789C4B040000620062}
	--assert (d: decompress c 'zlib)  = #{61}

===end-group===

if all [native? :filter native? :unfilter][
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
]

~~~end-file~~~