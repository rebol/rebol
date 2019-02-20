REBOL [
	title:  "REBOL 3 codec for SWF file format"
	name:   'codec-SWF
	author: "Oldes"
	version: 0.1.0
	date:    11-Oct-2018
	history: [
		0.1.0 11-Oct-2018 "Oldes" {
			Initial version with DECODE and IDENTIFY functions.
			Not all chunks are parsed.
		}
	]
]
#ifdef <USE_SWF_CODEC>
import module [
	Title: "SWF file format related functions"
	Name:  SWF
	Version: 0.1.0
	;Exports: []
][
	spriteLevel: 0

	align: func[bin][binary/read bin 'align]

	;readRECTObj: func[bin /local n rect][
	;	binary/read bin [align n: UB 5]
	;	rect: make object! [xMin: xMax: yMin: yMax: 0]
	;	set rect binary/read bin [SB :n SB :n SB :n SB :n align]
	;	rect 
	;]
	readRECT: func[bin /local n rect][
		binary/read bin [align n: UB 5]
		binary/read bin [SB :n SB :n SB :n SB :n align]
	]
	readPair: func[bin /local n][
		binary/read bin [n: UB 5]
		to-pair binary/read bin [FB :n FB :n]
	]
	readSBPair: func[bin /local n][
		binary/read bin [n: UB 5]
		to-pair binary/read bin [SB :n SB :n]
	]
	readMatrix: func[bin][
		binary/read bin 'align
		reduce [
			either binary/read bin 'bit [readPair bin][1x1] ;scale
			either binary/read bin 'bit [readPair bin][0x0] ;rotate
			readSBPair bin ;translate
		]
	]
	readCXFORMa: func[bin /local HasAddTerms? HasMultTerms? n tmp][
		binary/read bin [
			align
			HasAddTerms?:  BIT
			HasMultTerms?: BIT
			n: UB 4
		]
		tmp: reduce [
			either HasMultTerms? [
				binary/read bin [SB :n SB :n SB :n SB :n] ;RGBA
			][	[256 256 256 256] ]
			either HasAddTerms? [
				binary/read bin [SB :n SB :n SB :n SB :n] ;RGBA
			][	none ]
		]
		binary/read bin 'align
		tmp
	]
	readCLIPACTIONS: func[bin][
		;@@TODO: parse it once needed
		; So far it should be safe to read rest of all data bytes
		; as this record is always on end of tag
		binary/read bin 'bytes
	]
	readSHAPEWITHSTYLES: func[bin][
		;@@TODO: parse it once needed
		; So far it should be safe to read rest of all data bytes
		; as this record is always on end of tag
		binary/read bin 'bytes
	]
	

	read-SWFTags: func[bin /local result tag tagId tagLength tagData][
		++ spriteLevel
		result: make block! 64

		while [not tail? bin/buffer][
			tag: binary/read bin 'UI16LE
			tagId: (65472 and tag) >> 6
			tagLength: tag and 63
			if tagLength = 63 [tagLength: binary/read bin 'UI32LE]
			either tagLength > 0 [
				binary/read bin [tagData: BYTES :tagLength]
			][	tagData: none ]
			repend result [tagId tagData]
		]
		-- spriteLevel
		new-line/all/skip result true 2 
		result
	]

	read-PlaceObject2: func[bin /local flags][
		flags: binary/read bin 'BITSET8
		reduce [
			binary/read bin 'ui16LE                              ;depth
			flags/7                                              ;move or place
			either flags/6 [binary/read bin 'ui16LE      ][none] ;CharacterId
			either flags/5 [readMatrix bin               ][none] ;HasMatrix
			either flags/4 [align readCXFORMa bin        ][none] ;HasCxform
			either flags/3 [align binary/read bin 'ui16LE][none] ;HasRatio
			either flags/2 [align binary/read bin 'STRING][none] ;HasName
			either flags/1 [align binary/read bin 'ui16LE][none] ;HasClipDepth
			either flags/0 [align readCLIPACTIONS bin    ][none] ;HasClipActions
		]
	]

	read-DefineSprite: func[bin][
		reduce [
			binary/read bin 'UI16LE ;ID
			binary/read bin 'UI16LE ;frameCount
			read-SWFTags bin
		]
	]

	read-DefineShape: func[bin tagId [integer!] /local result][
		result: reduce [
			binary/read bin 'UI16LE ;shapeID
			readRECT bin            ;Bounds of the shape
			either tagId >= 67 [
				reduce [
					readRECT bin ;edgeBounds
					(
					 binary/read bin [UB 6]  ;reserved
					 binary/read bin 'BIT    ;usesNonScalingStrokes
					)
					binary/read bin 'BIT     ;usesScalingStrokes
				]
			][	none ]
			readSHAPEWITHSTYLES bin
		]
	]

	read-SceneAndFrameLabelData: func[bin /local n scenes labels][
		n: binary/read bin 'EncodedU32
		scenes: make block! 2 * n
		loop n [
			repend scenes binary/read bin [
				EncodedU32 ;offset
				STRING     ;name of scene
			]
		]
		n: binary/read bin 'EncodedU32
		labels: make block! 2 * n
		loop n [
			repend labels binary/read bin [
				EncodedU32 ;frame number
				STRING     ;name of frame
			]
		]
		reduce [scenes labels]
	]

	register-codec [
		name:  'SWF
		title: "ShockWave Flash"
		suffixes: [%.swf]

		decode: function [
			data [binary!]
		][
			if verbose > 0 [
				print ["^[[1;32mDecode SWF data^[[m (^[[1m" length? data "^[[mbytes )"]
			]

			swf: make object! [
				header: none
				tags:   none
			]

			bin: binary data ; initializes Bincode streams
			binary/read bin [
				compression: UI8
				signature:   UI16
				version:     UI8
			]
			unless all [
				signature = 22355           ; "WS"
				find [67 70 90] compression ; C F Z
			][
				; not a SWF file
				return none
			]
			fileSize: binary/read bin 'UI32LE
			if verbose > 0 [
				print [
					"SWF file version:" version
					select [
						67 "compressed using deflate"
						70 "uncompressed"
						90 "compressed using LZMA"
					] compression
					lf
					"Data size:" fileSize - 8 "bytes"
				]
			]
			
			switch compression [
				67 [
					binary/init bin decompress/zlib/size bin/buffer fileSize - 8
				]
				90 [
					packed: binary/read bin 'UI32LE
					binary/init bin decompress/lzma/size bin/buffer fileSize - 8
				]
			]

			binary/read bin [n: UB 5]
			frame-size: binary/read bin [SB :n SB :n SB :n SB :n ALIGN]
			;? bin
			binary/read bin [frame-rate: UI16 frame-count: UI16LE]
			if verbose > 0 [	
				print ["^[[32mframe-size:  ^[[0;1m" frame-size "^[[m"]
				print ["^[[32mframe-rate:  ^[[0;1m" frame-rate "^[[m"]
				print ["^[[32mframe-count: ^[[0;1m" frame-count "^[[m"]
			]
			swf/header: object compose [
				version: (version)
				bounds:  (frame-size)
				rate:    (frame-rate)
				frames:  (frame-count)
			]
			swf/tags: make block! (2 * frame-count) + 100
			while [not tail? bin/buffer][
				tag: binary/read bin 'UI16LE
				tagId: (65472 and tag) >> 6
				tagLength:  tag and 63
				if tagLength = 63 [tagLength: binary/read bin 'UI32LE]
				either tagLength > 0 [
					binary/read bin [tagData: BYTES :tagLength]
				][	tagData: none ]
				repend swf/tags [tagId tagData]
				if verbose > 0 [
					printf [-4 #" " $33 27 $0] reduce [
						tagId
						select swf-tags tagId
						either tagData [
							either verbose > 2 [
								mold tagData
							][
								tmp: mold copy/part tagData 32
								if 32 < length? tagData[
									change back tail tmp "..."
								]
								tmp
							]
						][""]
					]
				]
				if all [tagData verbose > 1] [
					if tagData <> tmp: decode-tag tagId tagData [
						prin #"^-" probe tmp
					]
				] 
			]
			
			new-line/all/skip swf/tags true 2 
			return swf
		]

		decode-tag: function[id [integer!] data [binary!]][
			bin: binary data
			switch/default id [
				26 [ read-PlaceObject2 bin ]
				39 [ read-DefineSprite bin ]
				83 [ read-DefineShape bin 83 ]
				86 [ read-SceneAndFrameLabelData bin ]
			][ data ] ; return original data if not able to decode
		]

		identify: func [
			"Returns TRUE if binary looks like SWF data"
			data [binary!]
		][
			parse/case data [[#"C" | #"F" | #"Z"] "WS" to end]
		]

		verbose: 2

		swf-tags: make map! [
			0  <End>
			1  <ShowFrame>
			2  <DefineShape>
			3  <FreeCharacter>
			4  <PlaceObject>
			5  <RemoveObject>
			6  <DefineBitsJPEG>
			7  <DefineButton>
			8  <JPEGTables>
			9  <SetBackgroundColor>
			10 <DefineFont>
			11 <DefineText>
			12 <DoAction>
			13 <DefineFontInfo>
			14 <DefineSound>
			15 <StartSound>
			18 <SoundStreamHead>
			17 <DefineButtonSound>
			19 <SoundStreamBlock>
			20 <DefineBitsLossless>
			21 <DefineBitsJPEG2>
			22 <DefineShape2>
			23 <DefineButtonCxform>
			24 <Protect>
			25 <PathsArePostscript>  ; https://www.m2osw.com/swf_tag_pathsarepostscript
			26 <PlaceObject2>
			28 <RemoveObject2>
			29 <SyncFrame>           ; https://www.m2osw.com/swf_tag_syncframe
			31 <FreeAll>             ; https://www.m2osw.com/swf_tag_freeall
			32 <DefineShape3>
			33 <DefineText2>
			34 <DefineButton2>
			35 <DefineBitsJPEG3>
			36 <DefineBitsLossless2>
			37 <DefineEditText>
			38 <DefineVideo>
			39 <DefineSprite>
			40 <NameCharacter>       ; https://www.m2osw.com/swf_tag_namecharacter
			41 <ProductInfo>         ; https://www.m2osw.com/swf_tag_productinfo
			42 <DefineTextFormat>
			43 <FrameLabel>
			45 <SoundStreamHead2>
			46 <DefineMorphShape>
			47 <GenerateFrame?>      ; https://www.m2osw.com/swf_tag_generateframe
			48 <DefineFont2>
			49 <GeneratorCommand>    ; https://www.m2osw.com/swf_tag_generatorcommand
			50 <DefineCommandObject> ; https://www.m2osw.com/swf_tag_definecommandobject
			51 <CharacterSet>        ; https://www.m2osw.com/swf_tag_characterset
			52 <ExternalFont>        ; https://www.m2osw.com/swf_tag_externalfont
			56 <ExportAssets>
			57 <ImportAssets>
			58 <EnableDebugger>
			59 <DoInitAction>
			60 <DefineVideoStream>
			61 <VideoFrame>
			62 <DefineFontInfo2>
			63 <DebugID>
			64 <ProtectDebug2>
			65 <ScriptLimits>
			66 <SetTabIndex>
			69 <FileAttributes>
			70 <PlaceObject3>
			71 <Import2>
			72 <DoABCDefine>
			73 <DefineFontAlignZones>
			74 <CSMTextSettings>
			75 <DefineFont3>
			76 <SymbolClass>
			77 <MetaData>
			78 <DefineScalingGrid>
			82 <DoABC>
			83 <DefineShape4>
			84 <DefineMorphShape2>
			86 <SceneAndFrameLabelData>
			87 <DefineBinaryData>
			88 <DefineFontName>
			89 <StartSound2>
			90 <DefineBitsJPEG4>
			91 <DefineFont4>
			93 <Telemetry>
			1023 <DefineBitsPtr>
		]
	]
]

#endif ; USE_SWF_CODEC