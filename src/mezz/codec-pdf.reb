REBOL [
	title:  "Codec: PDF"
	name:    pdf
	type:    module
	options: [delay]
	version: 0.1.0
	file:    https://raw.githubusercontent.com/Oldes/Rebol3/master/src/mezz/codec-pdf.reb
	author: "Oldes"
	history: [16-Sep-2021 "Oldes" {Initial version - raw PDF data de/encode}]
	notes: {
		The codec is so far able to decode PDF's Carousel Object Structure into Rebol structure.
		https://web.archive.org/web/20121205012515/http://jimpravetz.com/blog/2012/12/in-defense-of-cos/
		http://what-when-how.com/itext-5/understanding-the-carousel-object-system-itext-5/

		It's not yet much useful for document creation, but more for examination deep PDF file structures.

		The loading is not optimal. It tries to load all objects even these not referenced in cross-reference table.
		It can be changed in the future, but so far, purpose of the code is to learn some PDF implementation details.
		It is not designed for real life processing of large PDF documents.

		Useful link for manual tests: https://www.pdf-online.com/osa/validate.aspx
	}
]

system/options/log/pdf: 3

;- locals:
pdf: out: value: stream: obj-id: ref-id: c: n1: n2: o: g: t: x: s: e:
bytes: end: len: year: month: day: hour: minute: second: z: none

sec-pad: #{28BF4E5E4E758A4164004E56FFFA01082E2E00B6D0683E802F0CA9FE6453697A}

;- Rules
;; not using construction syntax for bitsets for higher backwards compatibility
rl_newline: [CRLF | LF | CR]
ch_number:        to bitset! #{000000000000FFC0}                    ;charset "0123456789"
ch_delimiter:     to bitset! #{0000000004C1000A0000001400000014}    ;charset "()<>[]{}/%"
ch_str-valid:     to bitset! [not #{00EC000000C0000000000008}]      ;complement charset "^/^M^-^H^L()\"
ch_sp:            to bitset! #{0040000080}                          ;charset " ^-"
ch_newline:       to bitset! #{0024}                                ;charset CRLF
ch_spnl:          to bitset! #{0064000080}                          ;charset " ^-^/^L^M"
ch_hex:           to bitset! #{000000000000FFC07FFFFFE07FFFFFE0}    ;charset [#"0" - #"9" #"a" - #"z" #"A" - #"Z"]
ch_hex-str:       to bitset! #{006400008000FFC07FFFFFE07FFFFFE0}    ;union union ch_hex sp ch_newline
ch_str:           to bitset! [not #{0000000000C0000000000008}]      ;complement charset "\()"
ch_str-esc:       to bitset! #{0000000000C0000000000008220228}      ;charset "nrtbf()\"
ch_not-hash:      to bitset! [not #{0000000010}]                    ;complement charset "#"
ch_not-newline:   to bitset! [not #{0024}]                          ;complement ch_newline
ch_not-delimiter: to bitset! [not #{0000000004C1000A0000001400000014}] ;complement ch_delimiter
ch_name:          to bitset! [not #{0064000084C1000A0000001400000014}] ;complement union ch_delimiter ch_spnl

rl_comment: [#"%" not #"%" copy value some ch_not-newline rl_newline]
rl_boolean: ["true" (value: true) | "false" (value: false)]
rl_null:    ["null" (value: none) ]
rl_number:  [
	copy value [
		opt [#"-" | #"+"]
		[ some ch_number opt [#"." any ch_number]
		| #"." some ch_number ]
	] (value: load value)
]
rl_name:    [#"/" copy value any ch_name         (
	value: to ref! dehex/escape value #"#"
	try [value: to word! value]
)]
rl_hex-str: [#"<" copy value any ch_hex-str #">" (value: debase value 16)]

rl_str: [
	some ch_str
	| #"(" while rl_str #")" ;@@ using `while` instead of `any` because `remove` would fail!
	| remove "\^/"
	| change [#"\" copy c 1 ch_str-esc] (
		select [
			#"n" #"^/"
			#"r" #"^M"
			#"t" #"^-"
			#"b" #"^H"
			#"f" #"^L"
			#"(" #"("
			#")" #")"
			#"\" #"\"
		] to char! c
	)
	| change [#"\" copy c 3 ch_number] (
		to char! (
		   ((to integer! c/1 - 48) << 6)
		 + ((to integer! c/2 - 48) << 3)
		 + ((to integer! c/3 - 48))) 
	)
	| change [#"\" copy c 2 ch_number] (
		to char! (
		   ((to integer! c/1 - 48) << 3)
		 + ((to integer! c/2 - 48))) 
	)
]
rl_date: [
	"(D:"
		copy year 4 ch_number
		(month: day: #"1" hour: minute: second: #"0")
		opt [copy month  2 ch_number]
		opt [copy day    2 ch_number]
		opt [copy hour   2 ch_number]
		opt [copy minute 2 ch_number]
		opt [copy second 2 ch_number]
		(
			value: to date! to string! rejoin [
				day #"-" month #"-" year #"/"
				hour #":" minute #":" second
			]
		)
		opt [
			#"Z"
			|
			(hour: minute: 0)
			copy z [#"+" | #"-"]
			copy hour   2 ch_number
			#"'"
			copy minute 2 ch_number
			#"'"
			(value/zone: to time! to string! rejoin [z hour #":" minute])
		]
	#")"
]
rl_string:  [
	#"("
	copy value while rl_str ;@@ using `while` instead of `any` by purpose mentioned in `rl_str`
	#")"
	(value: to string! value)
]
rl_ref-id: [
	copy n1 some ch_number some ch_sp
	copy n2 some ch_number
	(ref-id: as-pair n1: load n1 n2: load n2) ; ?? ref-id)
]
rl_reference: [
	rl_ref-id some ch_sp #"R" (value: ref-id)
]

*stack: copy []

rl_value: [
	  rl_name ;(try/with [value: to word! value][insert value #"/"])
	| rl_reference ;must be before number!
	| rl_number
	| rl_boolean
	| rl_date
	| rl_string
	| rl_hex-str
	| rl_dict
	| rl_array
	| rl_null
]

rl_dict: [
	"<<"
	(append/only *stack copy [])
	any ch_spnl
	any [
		rl_name (append last *stack value)
		any ch_spnl rl_value any ch_spnl
		(append/only last *stack value)
	]
	">>"
	(
		value: make map! take/last *stack
		len: any [value/length 0]
		if pair? len [len: any [pdf/objects/:len 0]]
		;? value
		;? len
	)
	opt [
		any ch_spnl
		"stream"
		rl_newline
		[
			copy bytes len skip ;t: (probe to-string copy/part t 15)
			rl_newline
			"endstream"
			|
			copy bytes to "^/endstream" 10 skip (
				log-warn 'PDF ["Length of the object" obj-id "stream is incorrect!" len "<>" length? bytes]
			)
		]
		rl_newline
		(
			stream: make object! [
				spec: value
				data: bytes
			]
			value: stream
			;? stream
		)
		any ch_spnl
	]
]
rl_array: [
	#"["
	(append/only *stack copy [])
	any ch_spnl
	any [
		rl_value any ch_spnl
		(append/only last *stack value)
	]
	#"]"
	(value: take/last *stack)
]

rl_obj: [
	rl_ref-id (obj-id: ref-id );? obj-id)
	some ch_sp
	"obj"
	any ch_spnl
		rl_value
	any ch_spnl
	"endobj"
	any ch_spnl
]

rl_xref: [
	"xref"
	rl_newline
	some [
		rl_ref-id
		rl_newline
		(log-trace 'PDF ["XREF" ref-id])
		n2 [
			copy o 10 ch_number #" "
			copy g  5 ch_number #" "
			copy t [#"n" | #"f"] any #" "
			rl_newline
			(
				o: load o
				g: load g
				t: to char! t
				repend pdf/referencies [o g t]
			)
		]
	]
]

rl_trailer: [
	"trailer"
	rl_newline
	rl_value
	any ch_spnl
	( pdf/trailer: value )
]

rl_startxref: [
	opt rl_trailer
	"startxref"
	rl_newline
	copy value some ch_number
	rl_newline
	(pdf/startxref: value: load value)
]

rl_import-object: [
	any ch_spnl
	rl_obj
	(
		;print ["=> obj " obj-id "==> " mold value]
		either all [
			object? value
			value/spec/Type = 'ObjStm
		][
			import-objstm value
		][
			put pdf/objects obj-id value ;@@ do warning when id is redefined?
		]
	)
]

rl_pdf: [
	"%PDF-" copy value [some ch_number #"." some ch_number] any ch_sp rl_newline
	(pdf/version: to string! value)
	any ch_spnl
	opt rl_comment
	rl_pdf_body
	any ch_spnl
	"%%EOF"
	any ch_spnl
	[
		end
		|
		opt [rl_pdf_body "%%EOF" to end]
	]
	
]
rl_pdf_body: [
	any ch_spnl
	any rl_import-object
	any ch_spnl
	any rl_xref
	any ch_spnl
	opt rl_startxref (
		log-trace 'PDF ["startxref offset:" value]
	)
	any ch_spnl
]


decompress-obj: func[obj [object!] /local p][
	try [
		switch obj/spec/Filter [
			FlateDecode [
				try [
					obj/data: decompress skip obj/data 2 'deflate
					obj/spec/Filter: none
					obj/spec/Length: length? obj/data 

					if all [
						p: obj/spec/DecodeParms
						p/Predictor = 12
						integer? p/Columns
					][
						obj/data: unfilter obj/data p/Columns
					]
				]
			]
		]
	]
]

import-objstm: function[obj [object!]][
	decompress-obj obj
	try/with [
		offsets: load to-string copy/part obj/data obj/spec/First
		;probe to-string obj/data
		obj-id: 0x0
		foreach [id ofs] offsets [
			;print [id ofs]
			obj-id/1: id
			if parse skip obj/data (ofs + obj/spec/First) [rl_value to end][
				;?? value
				put pdf/objects obj-id value
			]
		]
	][
		log-error 'PDF "Failed to unpack ObjStm"
		log-error 'PDF system/state/last-error
	]
]

form-ref: func[ref][
	ajoin [to integer! ref/1 #" " to integer! ref/2]
]
emit-block: func[val [block!]][
	out: insert out #"["
	forall val [ emit-val val/1 ] 
	out: insert out #"]"
]
emit-string: func[val [any-string!]][
	out: insert out #"("
	parse val [
		any [
			  s: some ch_str-valid e: (out: insert/part out s e)
			| 1 skip s: ( 
				out: insert out select #[
					#"^/" "\n"
					#"^M" "\r"
					#"^-" "\t"
					#"^H" "\b"
					#"^L" "\f"
					#"("  "\("
					#")"  "\)"
					#"\"  "\\"
				] s/-1
			)
		]
	]
	out: insert out #")"
]
emit-binary: func[val [binary!]][
	out: insert out ajoin [#"<" val #">"]
]
emit-indent: does [
	unless find " [" to char! out/-1 [out: insert out #" "]
]
emit-val: func[val][
	switch/default type?/word :val [
		ref!
		word!    [ out: insert insert out #"/" form val ]
		pair!    [
			emit-indent
			out: insert insert out form-ref val " R"
		]
		decimal!
		integer!
		logic!   [
			emit-indent
			out: insert out form val
		]
		block!   [ emit-block val ]
		map!     [ emit-obj val ]
		object!  [ emit-stream val ]
		string!
		email!   [ emit-string val ]
		binary!  [ emit-binary val ]
		date!    [ emit-string form-pdf-date val ]
	][
		ask rejoin ["Unnown value type!: " type? val copy/part mold val 10]
	]
]
emit-obj: func[obj][
	out: insert out "<<"
	foreach [key val] obj [
		unless none? val [
			out: insert insert out "/" form key
			emit-val val
		]
	]
	out: insert out ">>"
]

get-xref-count: function[xrefs n][
	xref: xrefs
	while [2 <= length? xref][
		i: xref/1/1
		either (i - n) <= 1 [
			n: i
			xref: skip xref 2
		][  break ]
	]
	to integer! n
]

emit-stream: func[obj [object!] /local data][
	unless find obj 'spec [
		put obj 'spec #[Length: 0]
	]
	data: any [obj/data #{}]
	unless any [           ; don't use compression 
		obj/spec/Filter    ; if there is already some filter
		300 > length? data ; if data are small enough
	][
		obj/spec/Filter: 'FlateDecode
		data: compress obj/data 'zlib
	]
	unless binary? data [
		; make sure that data are in binary, so the length is correct!
		data: to binary! data
	]
	obj/spec/Length: length? data
	emit-obj obj/spec
	out: insert insert insert out "stream^M^/" data "^M^/endstream"
]

rebol-version-str: rejoin ["Rebol/" system/product " Version " system/version]


remove-metadata: function [pdf [object!]][
	if all [
		objs: pdf/objects
		root: select objs pdf/trailer/Root
		meta: select objs id: root/Metadata
	][
		put objs id none
		root/Metadata: none
	]
]

form-pdf-date: function[d [date!]][
	z: any [d/zone 0:0]
	format/pad ["D:" -4 -2 -2 -2 -2 -2 1 -2 #"'" -2 #"'"] reduce [
		d/year d/month d/day d/hour d/minute d/second
		either z < 0:0 [#"-"][#"+"] z/hour z/minute
	] #"0"
]

build-pdf-tree: function[
	{Builds PDF-tree from a loaded PDF object}
	pdf [object!]
][
	unless all [
		map? trailer: select pdf 'trailer
		map? objects: select pdf 'objects
	][ return none ]
	;@@TODO...
]


decode-xref: func[value /local a b c inp index xref f1 f2 f3][
	if all [
		object? value
		value/spec/Type = 'XRef
	][
		decompress-obj value

		index: any [
			value/spec/Index
			reduce [0 value/spec/Size]
		]
		
		set [a b c] value/spec/W
		inp: value/data
		foreach [id sz] index [
			loop sz [
				f1: to integer! take/part inp a
				f2: to integer! take/part inp b
				f3: to integer! take/part inp c

				append pdf/referencies new-line reduce [
					as-pair id either f1 = 2 [0][f3]
					f1 f2
				] true
				++ id
			]
		]
		;? inp

		pdf/trailer: make map! reduce/no-set [
			Size: value/spec/Size
			Root: value/spec/Root
			Info: value/spec/Info
			ID:   value/spec/ID
		]
	]
]

register-codec [
	name:  'pdf
	type:  'application
	title: "Portable Document Format"
	suffixes: [%.pdf]

	decode: func [
		{Extract content of the PDF file}
		data  [binary! file! url!]
		/local tmp info keys obj1 obj2 enc md5 key
	][
		unless binary? data [ data: read data ]
		pdf: make object! [
			version:     none
			file-size:   length? data
			trailer:     none
			objects:     copy #[]
			referencies: copy  []
			startxref:   none
		]

		unless parse data rl_pdf [
			print "Failed to parse PDF!"
			return none
		]
		value: stream: obj-id: ref-id: none      ; release internal values
		new-line/all/skip pdf/referencies true 3 ; prettifies the block
		;? pdf halt
		keys: keys-of pdf/objects
		if all [
			;none? pdf/trailer
			obj1: pdf/objects/(keys/1)
			obj1/Linearized
		][
			;? obj1
			;clear pdf/objects
			if all [
				keys/2
				obj2: pdf/objects/(keys/2)
				obj2/spec/Type = 'XRef
			][
				decode-xref obj2
			]

			;probe to-string copy/part skip data obj1/T 20

			parse skip data obj1/T [
				any ch_spnl
				rl_obj 
				(
					;log-trace 'PDF  ["=> obj " obj-id "==> " mold value]
					pdf/trailer: value
					;?? value
					decode-xref value
					;?? pdf/trailer
					
				)
			]

			parse find/tail data to-binary "%%EOF" [
				any ch_spnl
				any rl_import-object
			]
		]
		if not empty? pdf/referencies [
			foreach [ofs id type] pdf/referencies [
				if type = #"n" [
					parse (skip data ofs - 1) rl_import-object
				]
			]
		]

		if all [
			pdf/trailer
			enc: pdf/trailer/Encrypt
			enc: pdf/objects/:enc
		][
			log-info 'PDF ["Encrypted using: ^[[m" enc/Filter "v:" enc/V "r:" enc/R]
			;@@ TODO...
			;? enc
			;if all [enc/Filter = 'Standard enc/V = 1][
			;	md5: open checksum:md5
			;	
			;	key: binary/write 64 [
			;		BYTES :sec-pad BYTES :enc/O ui32be :enc/P
			;	]
			;	? key
			;	binary/write key probe first pdf/trailer/ID
			;	? key
			;	md5: checksum key/buffer 'md5 
			;	? md5
			;	key: rc4/key copy/part md5 5
			;	? key
			;	probe rc4/stream key enc/U
			;]
		]
		if all [
			pdf/trailer
			system/options/log/pdf > 0
			map? info: try [pdf/objects/(pdf/trailer/info)]
		][
			if info/Author       [log-info 'PDF ["Author:  ^[[m" info/Author]]
			if info/Title        [log-info 'PDF ["Title:   ^[[m" info/Title]]
			if info/CreationDate [log-info 'PDF ["Created: ^[[m" info/CreationDate]]
			if info/ModDate      [log-info 'PDF ["Modified:^[[m" info/ModDate]]
			if info/Producer     [log-info 'PDF ["Producer:^[[m" info/Producer]]
			if info/Creator      [log-info 'PDF ["Creator: ^[[m" info/Creator]]
		]
		also pdf pdf: none ; return result and release the internal value
	]
	encode: func [
		pdf [object!]
		/local xref xref-pos i n last-obj-id version trailer objects info root
	][
		;@@ TODO!
		;-- This is just very simple encoder with not linearized output!
		;-- It does no input validity checks so user is responsible to provide
		;-- well formated pdf input object.

		;- validate minimal requirements...
		objects: select pdf 'objects
		unless any [map? objects block? objects][
			log-error 'PDF "Missing valid objects list!"
			return none
		]
		trailer: select pdf 'trailer
		unless trailer [
			put pdf 'trailer trailer: #[Info: #(none) Root: #(none)]
		]
		unless root: trailer/Root [
			log-trace 'PDF "Trying to locate `Catalog` in PDF objects."
			foreach [ref obj] pdf/objects [
				if all [map? obj obj/Type = 'Catalog][
					trailer/Root: ref
					break
				]
			]
		]
		unless root: trailer/Root [
			log-error 'PDF "Missing required `Catalog` object!"
			return none
		]
		if info: pick pdf/objects trailer/Info [
			unless info/CreationDate [info/CreationDate: now]
			if any [not info/Creator  info/Creator  = "Rebol"] [ info/Creator:  rebol-version-str ]
			if any [not info/Producer info/Producer = "Rebol"] [ info/Producer: rebol-version-str ]
			
			
			info/ModDate: now
		]

		unless version: select pdf 'version [ version: @1.3	]
		if decimal?  version [version: form version]
		unless parse version [some ch_number #"." some ch_number end][
			log-error 'PDF ["Invalid PDF version:" mold version]
			return none
		]

		;- File header..                                              
		out: make binary! any [select pdf 'file-size 60000]
		out: insert out ajoin ["%PDF-" version "^M%"]
		out: insert out #{E2E3CFD30D0A} ;= %âãÏÓ
		xref: copy []


		;- File body                                                  
		foreach [ref obj] pdf/objects [
			append xref reduce [ref -1 + index? out] 
			out: insert insert out form-ref ref " obj^M"
			emit-val obj

			out: insert out "^Mendobj^M"
		]

		;- Cross-Reference Table                                      
		xref-pos: out
		sort/skip xref 2
		i: 0
		n: get-xref-count xref i
		out: insert out ajoin [
			"xref^M^/0 " 1 + n "^M^/0000000000 65535 f^M^/"
		]
		for i 0 n - 1 1 [
			last-obj-id: to integer! xref/1/1
			out: insert out format/pad [-10 #" " -5 " n^M^/"] reduce [xref/2 to integer! xref/1/2] #"0"
			xref: skip xref 2
		]
		while [not tail? xref][
			i: to integer! xref/1/1
			n: get-xref-count xref i
			;print [i n]
			i: i - 1
			out: insert out ajoin [
				"" i + 1 #" " (n - i) "^M^/"
			]
			while [i < n] [
				last-obj-id: to integer! xref/1/1
				out: insert out format/pad [-10 #" " -5 " n^M^/"] reduce [xref/2 to integer! xref/1/2] #"0"
				xref: skip xref 2
				++ i
			]
		]
		;- File Trailer                                               
		out: insert out "trailer^M^/"
		emit-val trailer
		out: insert out ajoin ["^M^/startxref^M^/" -1 + index? xref-pos "^M^/%%EOF^M^/"]

		head out
	]

	identify: function [data [binary!]][
		parse data ["%PDF-" to end]
	]
]