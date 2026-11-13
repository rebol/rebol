Rebol [
	title:  "File type identifier"
	name:    identify
	type:    module
	version: 1.0.0
	date:    29-Mar-2025
	author:  @oldes
	license: MIT
	exports: [identify]
	purpose: {Identify file type of file or binary data}
	home:    https://github.com/Oldes/Rebol-Identify/
	;;https://github.com/h2non/filetype
	;;https://pypi.org/project/filetype/
	;;https://docs.fileformat.com/
	;;https://www.iana.org/assignments/media-types/media-types.xhtml
]

identify: func [
	"Identifies file types using their magic numbers."
	file [file! url! binary!]
	;return: [none! word!] ;; this specification is available since version 3.18.5!
][
	unless binary? file [file: read/part/binary file 262]
	foreach [type rule] types [
		parse file [rule to end (return type)]
	]
	none
]

types: context [
	;- Audio files...
	midi: #{49546864}
	mp3: [
		#{494433}
		| #"^(FF)" [
		      #"^(E2)" ;MPEG 2.5 with error protection
            | #"^(E3)" ;MPEG 2.5 w/o error protection
            | #"^(F2)" ;MPEG 2 with error protection
            | #"^(F3)" ;MPEG 2 w/o error protection
            | #"^(FA)" ;MPEG 1 with error protection
            | #"^(FB)" ;MPEG 1 w/o error protection
		]
	]
	m4a:  [#{4D344120} | 4 skip #{667479704D3441}]
	ogg:   #{4F676753}
	flac:  #{664C6143}
	wav:  [#{52494646} 4 skip #{57415645}]
	amr:   #{2321414D520A}
	aac:  [#{FFF1} | #{FFF9}]
	aiff: [#{464F524D} 4 skip #{41494646}]
	;- Image files...
	jpg:   #{FFD8FF}
	jpx:  [#{0000000C} 12 skip #{667479706A707820}]
	jp2:  [#{0000000C} 12 skip #{667479706A703220}]
	jxl:  [#{FF0A} | #{0000000C4A584C200D870A}]
	apng: [#{89504e470d0a1a0a} to #{6163544C}] ;; simplified!
	png:   #{89504E47}
	gif:   #{474946}
	webp: [#{52494646} 4 skip #{574542505650}]
	cr2:  [[#{49492A00} | #{4D4D002A}] 4 skip #{4352}]
	tiff: [[#{49492A00} | #{4D4D002A}] 4 skip not #{4352}]
	bmp:   #{424D}
	jxr:   #{4949BC}
	psd:   #{38425053}
	ico:   #{00000100}
	heic: [
		isobmf
		[
			#{68656963} | ;heic
			[
				#{6D696631} | ;mif1
				#{6D736631}   ;msf1
			] 8 skip [#{68656963} | 4 skip #{68656963} | 8 skip #{68656963}]
		]
	]
	dcm:  [128 skip #{4449434D}]
	dwg:   #{41433130}
	xcf:   #{67696d70207863662076}
	avif: [
		isobmf
		[
			#{61766966} | ;avif
			#{61766973} | ;avis
			[
				#{6D696631} | ;mif1
				#{6D736631}   ;msf1
			] 8 skip [#{61766966} | 4 skip #{61766966} | 8 skip #{61766966}]
		]
	]
	qoi:   #{716F6966}
	dds:   #{44445320}
	rpm:   #{edabeedb}
	epub: [#{504B0304} 26 skip #{6D696D65747970656170706C69636174696F6E2F657075622B7A6970}]
	zip:  [#{504B} [#"^(03)" | #"^(05)" | #"^(07)"] [#"^(04)" | #"^(06)" | #"^(08)"]]
	rar:  [#{526172211A07} [#"^(00)" | #"^(01)"]]
	gz:    #{1F8B08}
	bz2:   #{425A68}
	sevenz: #{377ABCAF271C}
	pdf:  [opt #{EFBBBF} #{25504446}]
	exe:   #{4D5A}
	swf:  [#{465753} | #{435753} | #{5A5753}]
	rtf:   #{7B5C727466} ;= application/rtf
	nes:   #{4E45531A}   ;= application/x-nintendo-nes-rom
	crx:   #{43723234}   ;= application/x-google-chrome-extension
	cab:  [#{4D534346} | #{49536328}] ;= application/vnd.ms-cab-compressed
	;eot: [8 skip [#{020001} | #{010000} | #{020002}] 23 skip #{4C50}] ;= application/octet-stream
	ps:    #{2521}
	xz:    #{FD377A585A00}
	sqlite: #{53514C69}
	deb:   #{213C617263683E0A64656269616E2D62696E617279}
	ar:    #{213C617263683E}
	z:    [#{1FA0} | #{1F9D}]
	lzop:  #{894C5A4F000D0A1A}
	lz:    #{4C5A4950}
	elf:  [#{7F454C46} 48 skip]
	lz4:   #{04224D18}
	br:    #{ceb2cf81}
	zst:  [[#"^(22)" | #"^(23)" | #"^(24)" | #"^(25)" | #"^(26)" | #"^(27)" | #"^(28)"] #{B52FFD}] ;! only Zstandard frames

	mp4:  [isobmf [mp4_brand | 8 skip [mp4_brand | 4 skip mp4_brand | 8 skip mp4_brand] ]]
	m4v:   #{0000001C667479704D3456}
	mkv:  [#{1A45DFA3} to #{428288 6D6174726F736B61}]
	webm: [#{1A45DFA3} to #{428284 7765626D}]
	mov:  [isobmf #{71742020}]
	avi:   #{5249464641564920}
	wmv:   #{3026B2758E66CF11A6D9}
	flv:   #{464C5601}
	mpg:  [#{000001} mpg_ver]
	m3gp: [4 skip #{66747970336770}]

	wasm:  #{0061736d01000000}
	tar:  [257 skip #{7573746172}]
]

mpg_ver: charset [#"^(b0)" - #"^(bf)"]
mp4_brand: [#{6D703432} | #{6D703431} | #{69736F6D}]
isobmf: [4 skip #{66747970}]
