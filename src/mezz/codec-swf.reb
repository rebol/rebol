REBOL [
	name:    swf
	type:    module
	options: [delay]
	version: 0.2.0
	title:  "SWF file codec"
	file:    https://raw.githubusercontent.com/Oldes/Rebol3/master/src/mezz/codec-swf.reb
	author: "Oldes"
	date:    25-Feb-2019
	history: [
		0.2.0 25-Feb-2019 "Oldes" {
			Basic set of SWF tag decoders.
		}
		0.1.0 11-Oct-2018 "Oldes" {
			Initial version with DECODE and IDENTIFY functions.
			Not all chunks are parsed.
		}
	]
]

spriteLevel: 0
tId: 0     ; shared tag's ID
bin: none  ; shared tag's binary

align: does [ binary/read bin 'align ]

readID: does [ binary/read bin 'UI16LE ]
readUsedID: does [ binary/read bin 'UI16LE ]
readSTRING: does [ binary/read bin 'STRING ]
readCount: has[count][
	count: binary/read bin 'UI8
	either 255 = count [ binary/read bin 'UI16LE ][ count ]
]

readRECT: has[n][
	binary/read bin [align n: UB 5]
	binary/read bin [SB :n SB :n SB :n SB :n align]
]

readPair: has[n][
	n: binary/read/with bin 'UB 5
	to pair! binary/read bin [FB :n FB :n]
]

readSBPair: has[n][
	n: binary/read/with bin 'UB 5
	to pair! binary/read bin [SB :n SB :n]
]

readMATRIX: does [
	also reduce [
		either binary/read bin 'bit [readPair][1x1] ;scale
		either binary/read bin 'bit [readPair][0x0] ;rotate
		readSBPair ;translate
	] align
]

readRGBA: does [ binary/read bin 'TUPLE4 ]
readRGB:  does [ binary/read bin 'TUPLE3 ]

readCXFORM: has [HasAddTerms? HasMultTerms? n tmp][
	binary/read bin [
		HasAddTerms?:  BIT
		HasMultTerms?: BIT
		n: UB 4
	]
	tmp: reduce [
		either HasMultTerms? [
			binary/read bin [SB :n SB :n SB :n] ;RGB
		][	[256 256 256] ]
		either HasAddTerms? [
			binary/read bin [SB :n SB :n SB :n] ;RGB
		][	none ]
	]
	binary/read bin 'align
	tmp
]

readCXFORMa: has [HasAddTerms? HasMultTerms? n tmp][
	binary/read bin [
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

readGRADIENT: func[type /local n gradients a b][
	binary/read bin [
		a: UB 2
		b: UB 2
		n: UB 4
	]
	reduce [
		a ;SpreadMode
		b ;InterpolationMode
		(
			gradients: make block! n
			loop n [
				;reading GRADRECORD
				append gradients reduce [
					binary/read bin 'UI8                   ; ratio
					either tId >= 32 [readRGBA][readRGB] ; color
				]
			]
			gradients
		)
		either all [type = 19 tId = 83][
			binary/read bin 'FIXED8                        ; FocalPoint
		][	none] 
	]
]

readCLIPACTIONS: does [
	;@@TODO: parse it once needed
	; So far it should be safe to read rest of all data bytes
	; as this record is always on end of tag
	binary/read bin 'bytes
]

readSHAPEWITHSTYLES: does [
	reduce [
		readFILLSTYLEARRAY 
		readLINESTYLEARRAY 
		readSHAPERECORD    
	]
]

readFILLSTYLEARRAY: has[n result][
	align
	n: binary/read bin 'UI8
	if n = 255 [ n: binary/read bin 'UI16LE ] ;FillStyleCountExtended
	result: make block! n
	loop n [
		append/only result readFILLSTYLE
	]
	result
]

readLINESTYLEARRAY: has[n result][
	n: binary/read bin 'UI8
	if n = 255 [ n: binary/read bin 'UI16LE ] ;LineStyleCountExtended
	result: make block! n
	loop n [
		append/only result readLINESTYLE
	]
	result
]

readFILLSTYLE: has[type][
	reduce [
		type: binary/read bin 'UI8 ;FillStyleType
		case [
			type = 0 [
				;solid fill
				binary/read bin case [
					find [46 84] tId [ [TUPLE4 TUPLE4] ] ;morph
					tId >= 32 ['TUPLE4]  ;rgba
					true ['TUPLE3] ;rgb
				]
			]
			any [
				type = 16 ;linear gradient fill
				type = 18 ;radial gradient fill
				type = 19 ;focal gradient fill (swf8)
			][
				;gradient
				reduce either find [46 84] tId [
					;morph
					[readMATRIX readMATRIX readGRADIENT type ]
				][	;shape
					[readMATRIX readGRADIENT type]
				]
			]
			type >= 64 [
				;bitmap
				reduce either find [46 84] tId [
					;morph
					[readUsedID readMATRIX readMATRIX]
				][	;shape
					[readUsedID readMATRIX]
				]
			]
		]
	]
]

readLINESTYLE: function[][
	reduce case [
		;DefineMorphShape
		tId = 46 [
			binary/read bin [
				UI16LE ;StartWidth
				UI16LE ;EndWidth
				TUPLE4
				TUPLE4
			]
		]
		;DefineShape4
		any [tId = 67 tId = 83][
			binary/read bin [
				width:       UI16LE
				capStyle:    UB 2
				joinStyle:   UB 2
				hasFill?:    BIT
				noHScale?:   BIT
				noVScale?:   BIT
				pixelHint?:  BIT
				             SKIPBITS 5 ; reserved
				noClose?:    BIT
				endCapStyle: UB 2
			]
			[
				width
				reduce [
					capStyle
					joinStyle
					hasFill?
					noHScale?
					noVScale?
					pixelHint?
					noClose?
					endCapStyle
				]
				either joinStyle = 2 [binary/read bin 'UI16LE][none] ;miterLimit
				either hasFill? [readFILLSTYLE][readRGBA]
			]
		]
		;DefineMorphShape2
		tId = 84 [
			binary/read bin [
				startWidth:  UI16LE
				endWidth:    UI16LE
				capStyle:    UB 2
				joinStyle:   UB 2
				hasFill?:    BIT
				noHScale?:   BIT
				noVScale?:   BIT
				pixelHint?:  BIT
				             SKIPBITS 5 ; reserved
				noClose?:    BIT
				endCapStyle: UB 2
			]
			[
				startWidth
				endWidth
				reduce [
					capStyle
					joinStyle
					hasFill?
					noHScale?
					noVScale?
					pixelHint?
					noClose?
					endCapStyle
				]
				either joinStyle = 2 [binary/read bin 'UI16LE][none] ;miterLimit
				either hasFill? [readFILLSTYLE][reduce [readRGBA readRGBA]]
			]
		]
		true [
			[
				binary/read bin 'UI16LE ;Width
				either tagId = 32 [readRGBA][readRGB]
			]
		]
		
	]
]

readSHAPERECORD: func[/local numFillBits numLineBits records nBits lineType states ][
	binary/read bin [
		numFillBits: UB 4
		numLineBits: UB 4
	]
	records: make block! 32
	until [
		either binary/read bin 'BIT [ ;edge?
			;print "edge"
			either binary/read bin 'BIT [;straightEdge?
				;print "line - "
				if lineType <> 'line [append records lineType: 'line]
				nBits: 2 + binary/read/with bin 'UB 4
				append records either binary/read bin 'BIT [
					;GeneralLine
					binary/read bin [
						SB :nBits ;deltaX
						SB :nBits ;deltaY
					]
				][
					reduce either binary/read bin 'BIT [
						;Vertical
						[0 binary/read/with bin 'SB :nBits]
					][	;Horizontal
						[binary/read/with bin 'SB :nBits 0]
					]
				]
			][
				;print "curve - "
				if lineType <> 'curve [append records lineType: 'curve]
				nBits: 2 + binary/read/with bin 'UB 4
				append records binary/read bin [
					SB :nBits ;controlDeltaX
					SB :nBits ;Y
					SB :nBits ;anchorDeltaX
					SB :nBits ;Y
				]
			]
			false
		][
			states: binary/read/with bin 'UB 5
			;print ["STATES:" states]
			either states = 0 [
				;EndShapeRecord
				true ;end
			][
				lineType: none
				;StyleChangeRecord
				;print ["SCR:" mold copy/part bin/buffer 5 (enbase to-binary to-char states 2)]
				append records 'style
				insert/only tail records reduce [
					either 0 < (states and 1 ) [ readSBPair ][none] ;move
					either 0 < (states and 2 ) [ binary/read/with bin 'UB :numFillBits][none] ;fillStyle0
					either 0 < (states and 4 ) [ binary/read/with bin 'UB :numFillBits][none] ;fillStyle1
					either 0 < (states and 8 ) [ binary/read/with bin 'UB :numLineBits][none] ;lineStyle
					either 0 < (states and 16) [
						reduce [
							readFILLSTYLEARRAY
							readLINESTYLEARRAY
							binary/read bin [
								numFillBits: UB 4 ;Number of fill index bits for new styles
								numLineBits: UB 4 ;...line...
							]
						]
					][none] ;NewStyles
				]
				false ;continue
			]		
		]
	]
	align
	records	
]

read-SWFTags: func[/local result tag tagLength tagData][
	++ spriteLevel
	result: make block! 64

	while [not tail? bin/buffer][
		tag: binary/read bin 'UI16LE
		tId: (65472 and tag) >> 6
		tagLength: tag and 63
		if tagLength = 63 [tagLength: binary/read bin 'UI32LE]
		either tagLength > 0 [
			binary/read bin [tagData: BYTES :tagLength]
		][	tagData: none ]
		repend result [tId tagData]
	]
	-- spriteLevel
	new-line/all/skip result true 2 
	result
]

;@@ NOTE: to simplify processing, first 6 values returned from PlaceObject* are always:
;@@ [Depth move/place ClassName CharacterId Matrix Cxform]
read-PlaceObject: has[id depth][
	binary/read bin [
		id:    UI16LE   ;ID of character to place
		depth: UI16LE   ;Depth of character
	]
	reduce [
		depth
		none        ; always place
		none        ; always without ClassName
		id          ; CharacterId
		readMATRIX  ; Transform matrix data
		either tail? bin/buffer [none][readCXFORM]   ;Color transform data
	]
]

read-PlaceObject2: func[/local flags][
	flags: binary/read bin 'BITSET8
	reduce [
		binary/read bin 'UI16LE                         ;depth
		flags/7                                         ;TRUE = move, else place
		none                                            ;ClassName (not exists)
		either flags/6 [binary/read bin 'UI16LE ][none] ;CharacterId
		either flags/5 [readMATRIX              ][none] ;HasMatrix
		either flags/4 [readCXFORMa             ][none] ;HasCxform
		either flags/3 [binary/read bin 'UI16LE ][none] ;HasRatio
		either flags/2 [binary/read bin 'STRING ][none] ;HasName
		either flags/1 [binary/read bin 'UI16LE ][none] ;HasClipDepth
		either flags/0 [readCLIPACTIONS         ][none] ;HasClipActions
	]
]

read-PlaceObject3: func[/local flags][
	flags: binary/read bin 'BITSET16
	reduce [
		binary/read bin 'UI16LE                          ;depth
		flags/7                                          ;TRUE = move, else place
		either flags/12 [binary/read bin 'STRING ][none] ;ClassName
		either flags/6  [binary/read bin 'UI16LE ][none] ;CharacterId
		either flags/5  [readMATRIX              ][none] ;HasMatrix
		either flags/4  [readCXFORMa             ][none] ;HasCxform
		either flags/3  [binary/read bin 'UI16LE ][none] ;HasRatio
		either flags/2  [binary/read bin 'STRING ][none] ;HasName
		either flags/1  [binary/read bin 'UI16LE ][none] ;HasClipDepth
		either flags/15 [readFILTERS             ][none] ;SurfaceFilterList
		either flags/14 [binary/read bin 'UI8    ][none] ;HasBlendMode
		either flags/13 [binary/read bin 'UI8    ][none] ;BitmapCaching
		either flags/10 [binary/read bin 'UI8    ][none] ;0 = Place invisible, 1 = Place visible
		either flags/9  [binary/read bin 'TUPLE4 ][none] ;Background Color
		either flags/0  [readCLIPACTIONS         ][none] ;HasClipActions
	]
]

read-RemoveObject: does [
	binary/read bin [
		UI16LE ;charID
		UI16LE ;depth
	]
]
read-RemoveObject2: does [
	binary/read bin 'UI16LE ;depth
]

read-DefineSprite: func[][
	reduce [
		binary/read bin 'UI16LE ;ID
		binary/read bin 'UI16LE ;frameCount
		read-SWFTags
	]
]

read-DefineShape: func[/local result][
	result: reduce [
		binary/read bin 'UI16LE ;shapeID
		readRECT                ;Bounds of the shape
		either tId >= 67 [
			reduce [
				readRECT ;edgeBounds
				(
				 binary/read bin [UB 6]  ;reserved
				 binary/read bin 'BIT    ;usesNonScalingStrokes
				)
				binary/read bin 'BIT     ;usesScalingStrokes
			]
		][	none ]
		readSHAPEWITHSTYLES
	]
]

read-SceneAndFrameLabelData: has[n scenes labels][
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

read-ExportAssets:
read-SymbolClass: has[count symbols][
	count: binary/read bin 'UI16LE
	symbols: make block! (2 * count)
	loop count [
		append symbols binary/read bin [
			UI16LE ;id
			STRING ;name
		]
	]
	symbols
]

read-SWT-CharacterName: does [
	binary/read bin [
		UI16LE ;ID
		STRING
	]
]
read-ImportAssets: has[result][
	reduce [
		binary/read bin 'STRING ;URL
		none
		read-SymbolClass
	]
]
read-ImportAssets2: has[result][
	reduce [
		binary/read bin 'STRING ;URL
		binary/read bin [UI8 UI8]
		read-SymbolClass
	]
]

read-SetTabIndex:
read-ScriptLimits: does [
	binary/read bin [
		UI16LE ;Depth    | MaxRecursionDepth
		UI16LE ;TabIndex | ScriptTimeoutSeconds
	]
]

read-FileAttributes: does [
	binary/read bin [
		BITSET8
		UI24LE ;reserved, must be 0
	]
]

read-DefineScalingGrid: does [reduce[
	readUsedID ;sprite or button to use with
	readRECT
]]


;-------------------------------------------------------------------------
;-- Images:                                                               
;-------------------------------------------------------------------------

read-DefineBitsLossless: has[
	BitmapID BitmapFormat BitmapWidth BitmapHeight ColorTableSize
][
	binary/read bin [
		BitmapID:     UI16LE
		BitmapFormat: UI8
		BitmapWidth:  UI16LE
		BitmapHeight: UI16LE
	]
	ColorTableSize: either BitmapFormat = 3 [binary/read bin 'UI8][none]
	reduce [
		BitmapID
		BitmapFormat
		BitmapWidth
		BitmapHeight
		ColorTableSize
		copy bin/buffer ; ZlibBitmapData
	]
]

read-DefineBitsJPEG2: does [
	binary/read bin [
		UI16LE ; BitmapID
		BYTES  ; JPEGData
	]
]
read-DefineBitsJPEG3: has [AlphaDataOffset][
	binary/read bin [
		UI16LE ; BitmapID
		AlphaDataOffset: UI32LE
		BYTES :AlphaDataOffset ; JPEGData
		BYTES  ; BitmapAlphaData
	]
]
read-DefineBitsJPEG4: has[AlphaDataOffset] [
	binary/read bin [
		UI16LE ; BitmapID
		AlphaDataOffset: UI32LE
		FIXED8 ; DeblockParam
		BYTES :AlphaDataOffset ; JPEGData
		BYTES  ; BitmapAlphaData
	]
]


;-------------------------------------------------------------------------
;-- Sound:                                                                
;-------------------------------------------------------------------------

read-DefineSound: does [
	binary/read bin [
		UI16LE ;soundid
		UB 4   ;SoundFormat
		UB 2   ;SoundRate
		BIT    ;SoundSize: FALSE = 8bit, TRUE = 16bit
		BIT    ;SoundType: FALSE = mono, TRUE = stereo
		UI32LE ;SoundSampleCount
		BYTES  ;SoundData
	]
]

read-StartSound: does [
	reduce [
		readUsedID
		readSOUNDINFO
	]
]

read-StartSound2: does [
	reduce [
		binary/read bin 'STRING ;SoundClassName
		readSOUNDINFO
	]
]

read-SoundStreamHead: has[result StreamSoundCompression] [
	result: binary/read bin [
		UB 4 ;reserved
		UB 2 ;PlaybackSoundRate
		Bit  ;PlaybackSoundSize
		Bit  ;PlaybackSoundType
		StreamSoundCompression: UB 4
		UB 2 ;StreamSoundRate
		Bit  ;StreamSoundSize
		Bit  ;StreamSoundType
		UI16LE ;StreamSoundSampleCount
	]
	append result either StreamSoundCompression = 2 [binary/read bin 'SI16LE][none]
	result
]

readSOUNDINFO: has[flags][
	flags: binary/read bin 'BITSET8
	reduce [
		flags
		either flags/7 [binary/read bin 'UI32LE][none] ;InPoint
		either flags/6 [binary/read bin 'UI32LE][none] ;OutPoint
		either flags/5 [binary/read bin 'UI16LE][none] ;Loops
		either flags/4 [readSOUNDENVELOPE][none]
	]
]

readSOUNDENVELOPE: has[count result][
	count: binary/read bin 'UI8
	result: make block! (3 * count)
	loop count [
		append result binary/read bin [
			UI32LE ;Pos44
			UI16LE ;LeftLevel
			UI16LE ;RightLevel
		]
	]
	result
]


;-------------------------------------------------------------------------
;-- Button:                                                               
;-------------------------------------------------------------------------

readBUTTONRECORDs: has[records flags id depth] [
	records: copy []
	while [#{00} <> flags: binary/read/with bin 'BYTES 1][
		repend/only records [
			flags: to bitset! flags
			binary/read bin 'UI16LE ;ID
			binary/read bin 'UI16LE	;PlaceDepth
			readMATRIX
			either tId = 34 [readCXFORMa][none]
			either all [flags/3 tId = 34][readFILTERS][none] ;FilterList
			either all [flags/2 tId = 34][binary/read bin 'UI8][none] ;BlendMode
		]
	]
	new-line/all records true
]

readBUTTONCONDACTIONs: has [actions length CondActionSize][
	actions: copy []
	until [
		CondActionSize: binary/read bin 'UI16LE
		repend actions [
			binary/read bin 'BITSET16
			readACTIONRECORDs
		]
		CondActionSize = 0 
	]
	new-line/all actions true
]

read-DefineButton: does [
	reduce [
		binary/read bin 'UI16LE ;id
		readBUTTONRECORDs
		readACTIONRECORDs
	]
]

read-DefineButton2: has[pos] [
	reduce [
		binary/read bin 'UI16LE  ;id
		binary/read bin 'BITSET8 ;flags
		(
			pos: (index? bin/buffer) + binary/read bin 'UI16LE
			readBUTTONRECORDs
		)
		(
			binary/read/with bin 'AT :pos
			readBUTTONCONDACTIONs
		)
	]
]

read-DefineButtonCxform: does [
	reduce [
		binary/read bin 'UI16LE  ;id
		readCXFORM
	]
]

read-DefineButtonSound: has[id] [
	reduce [
		readUsedID ;ButtonId
		;ButtonSoundChar0,ButtonSoundInfo0 (OverUpToIdle)
		either 0 < id: readUsedID [reduce [id readSOUNDINFO]][none]
		;ButtonSoundChar1,ButtonSoundInfo1 (IdleToOverUp)
		either 0 < id: readUsedID [reduce [id readSOUNDINFO]][none]
		;ButtonSoundChar2,ButtonSoundInfo2 (OverUpToOverDown)
		either 0 < id: readUsedID [reduce [id readSOUNDINFO]][none]
		;ButtonSoundChar3,ButtonSoundInfo3 (OverDownToOverUp)
		either 0 < id: readUsedID [reduce [id readSOUNDINFO]][none]
	]
]


;-------------------------------------------------------------------------
;-- Video:                                                                
;-------------------------------------------------------------------------

read-DefineVideoStream: does [
	binary/read bin [
		UI16LE ;ID
		UI16LE ;NumFrames
		UI16LE ;Width
		UI16LE ;Height
		UB 4 ;VideoFlagsReserved
		UB 3 ;VideoFlagsDeblocking
		BIT  ;VideoFlagsSmoothing
		UI8  ;CodecID
	]
]

read-VideoFrame: does [
	binary/read bin [
		UI16LE ;StreamID
		UI16LE ;FrameNum
		BYTES  ;VideoData
	]
]

;-------------------------------------------------------------------------
;-- Actions:                                                              
;-------------------------------------------------------------------------

readACTIONRECORDs:
read-DoAction: has[records index ActionCode length] [
	records: copy []
	until [
		binary/read bin [
			index: INDEX
			ActionCode: BYTES 1
		]
		length: either (to integer! ActionCode) > 127 [binary/read bin 'UI16LE][0]
		append records reduce [
			index - 1
			ActionCode
			binary/read/with bin 'BYTES :length
		]
		ActionCode = #{00} ;end?
	]
	new-line/skip records true 3
	records
]

read-DoInitAction: does [reduce[
	readUsedID	;Sprite ID
	readACTIONRECORDs
]]

read-DoABC: func[][
	reduce [
		either tId = 72 [none][binary/read/with bin 'BYTES 4] ; flags
		either tId = 72 [none][binary/read bin 'STRING]       ; name
		binary/read bin 'BYTES                                ; ABCData
	]
]

;-------------------------------------------------------------------------
;-- Text:                                                                 
;-------------------------------------------------------------------------

read-DefineFont: has[id OffsetTable GlyphShapeTable last-ofs][
	binary/read bin [
		UI16LE ;fontID
		BYTES
	]
]

read-DefineFont3: has[
	fontID flags langCode fontName numGlyphs OffsetTable ofs CodeTableOffset GlyphShapeTable tmp n
][
	binary/read bin [
		fontID:    UI16LE
		flags:     BITSET8
		langCode:  UI8
		n:         UI8
		fontName:  BYTES :n
		numGlyphs: UI16LE
	]
	either numGlyphs = 0 [
		reduce [
			fontID flags langCode
			to string! fontName
			none none none
		]
	][
		reduce [
			fontID flags langCode
			to string! fontName
			(
				ofs: either flags/4 ['UI32LE]['UI16LE]

				;OffsetTable: make block! numGlyphs
				;loop numGlyphs [
				;	append OffsetTable binary/read bin :ofs
				;]
				;new-line/all OffsetTable true

				binary/read/with bin 'SKIP (numGlyphs * either flags/4 [4][2])

				CodeTableOffset: binary/read bin :ofs

				GlyphShapeTable: make block! numGlyphs
				loop NumGlyphs [
					append/only GlyphShapeTable readSHAPERECORD
				]
				new-line/all GlyphShapeTable true
			)
			to string! binary/read/with bin 'BYTES (
				numGlyphs * either tId = 48 [
					either flags/4 [2][1]
				][	either flags/4 [4][2] ]
			) ;CodeTable
			either flags/0 [;FontFlagsHasLayout
				reduce [
					binary/read bin 'SI16LE ;FontAscent
					binary/read bin 'SI16LE ;FontDescent
					binary/read bin 'SI16LE ;FontLeading
					(
						tmp: make block! numGlyphs
						loop NumGlyphs [append tmp binary/read bin 'SI16LE]
						tmp ;FontAdvanceTable
					)
					(
						clear tmp  
						loop NumGlyphs [append tmp readRECT]
						tmp ;FontBoundsTable
					)
					(
						binary/read bin 'BYTES ;was: readKERNINGRECORDs WideOffsets?
					)
				]
			][	none ]
		]
	]
]

read-DefineFont4: does[
	binary/read bin [
		UI16LE  ;fontID
		BITSET8 ;flags
		STRING  ;fontName
		BYTES   ;FontData (OpenType CFF font)
	]
]

read-DefineText: does [ ;same for DefineText2
	reduce [
		readUsedID ;charId
		readRECT   ;TextBounds
		readMATRIX ;TextMatrix
		readTEXTRECORD
	]
]

read-DefineEditText: has[flags][
	reduce [
		binary/read bin 'UI16LE ;ID
		readRECT                ;Bounds
		flags: binary/read bin 'BITSET16
		either flags/7  [ binary/read bin 'UI16LE ][none] ;FontID
		either flags/8  [ binary/read bin 'STRING ][none] ;FontClass
		either flags/7  [ binary/read bin 'UI16LE ][none] ;FontHeight
		either flags/5  [ binary/read bin 'TUPLE4 ][none] ;TextColor
		either flags/6  [ binary/read bin 'UI16LE ][none] ;MaxLength
		either flags/10 [
			binary/read bin [
				UI8    ;align
				UI16LE ;LeftMargin	
				UI16LE ;RightMargin
				UI16LE ;Indent
				UI16LE ;Leading
			]
		][none] ;Layout
		binary/read bin 'STRING ;VariableName
		either flags/0 [ binary/read bin 'STRING ][none] ;InitialText
	]
]

readTEXTRECORD: func[/local GlyphBits AdvanceBits flags records GlyphCount glyphs][
	binary/read bin [
		align
		GlyphBits:   UI8
		AdvanceBits: UI8
	]
	records: copy []
	while [0 <> bin/buffer/1][
		flags: binary/read bin 'BITSET8
		append records reduce [
			either flags/4 [readUsedID][none] ;fontID
			either flags/5 [either tId = 11 [readRGB][readRGBA]][none]
			either flags/7 [binary/read bin 'SI16LE  ][none] ;XOffset
			either flags/6 [binary/read bin 'SI16LE  ][none] ;YOffset
			either flags/4 [binary/read bin 'UI16LE  ][none] ;TextHeight
			(
				GlyphCount: binary/read bin 'UI8
				glyphs: make block! 2 * GlyphCount
				loop GlyphCount [
					append glyphs binary/read bin [
						UB :GlyphBits   ;GlyphIndex
						SB :AdvanceBits ;GlyphAdvance
					]
				]
				glyphs
			)
		]
		align
	]
	records
]

readKERNINGRECORDs: func[wide? /local result][
	result: copy []
	either wide? [
		loop readUI16 [
			append result reduce [
				readUI16 ;FontKerningCode1
				readUI16 ;FontKerningCode2
				readSI16 ;FontKerningAdjustment
			]
		]
	][
		loop readUI16 [
			append result reduce [
				readUI8  ;FontKerningCode1
				readUI8  ;FontKerningCode2
				readSI16 ;FontKerningAdjustment
			]
		]
	]
	result
]

read-DefineFontInfo: has[flags][
	reduce [
		binary/read bin 'UI16LE                        ;fontID
		to string! binary/read bin 'UI8BYTES           ;FontName
		binary/read bin 'BITSET8                       ;flags
		either tId = 13 [none][ binary/read bin 'UI8 ] ;LangCode
		binary/read bin 'BYTES                         ;CodeTable
	]
]

read-DefineAlignZones: does [
	reduce [
		binary/read bin 'UI16LE    ;fontID
		binary/read/with bin 'UB 2 ;csmTableHint
		binary/read/with bin 'UB 6 ;reserved
		readALIGNZONERECORDs
	]
]

readALIGNZONERECORDs: has[records numZoneData zoneData][
	records: copy []
	while [not tail? bin/buffer][
		repend/only records [
			(
				numZoneData: binary/read bin 'UI8
			 	zoneData: make block! numZoneData
				loop numZoneData [
					append zoneData binary/read bin [FLOAT16 FLOAT16]
				]
				zoneData
			)
			binary/read bin 'UI8 ;zoneMask
		]
	]
	records
]

read-CSMTextSettings: does [
	binary/read bin [
		UI16LE ;textID
		UB 2   ;styleFlagsUseSaffron
		UB 3   ;gridFitType
		UB 3   ;reserved
		UI32LE ;thickness
		UI32LE ;sharpness
		UI8    ;reserved
	]
]

read-DefineFontName: does [
	binary/read bin [
		UI16LE ;fontID
		STRING ;fontName
		STRING ;copyright
	]
]


;-------------------------------------------------------------------------
;-- Morphing:                                                             
;-------------------------------------------------------------------------

readMORPHFILLSTYLEARRAY: has[count FillStyles][
	FillStyles: make block! count: readCount ; count can have 1 or 2 bytes!
	loop count [ ;FillStyleCount
		append/only FillStyles readMORHFILLSTYLE
	]
	FillStyles
]

readMORPHLINESTYLEARRAY: has[count LineStyles][
	LineStyles:  make block! count: readCount ; count can have 1 or 2 bytes!
	loop count [ ;LineStyleCount
		append/only LineStyles either tId = 46 [
			binary/read bin [
				UI16LE ;StartWidth
				UI16LE ;EndWidth
				RGBA
				RGBA
			] ;<= readMORPHLINESTYLE
		][ readMORPHLINESTYLE2 ]
	]
	LineStyles
]

readMORPHLINESTYLE2: has [joinStyle hasFill?][
	reduce [
		binary/read bin 'UI16LE ;StartWidth
		binary/read bin 'UI16LE ;EndWidth
		binary/read bin [
			UB 2            ;start_cap_style
			joinStyle: UB 2 ;join_style
			hasFill?:  BIT  ;has_fill
			BIT             ;no_hscale
			BIT             ;no_vscale
			BIT             ;pixel_hinting
			UB 5            ;reserved
			BIT             ;no_close
			UB 2            ;end_cap_style
		]
		either joinStyle = 2 [binary/read bin 'UI16LE][none] ;miterLimit
		either hasFill? [readMORHFILLSTYLE][binary/read bin [TUPLE4 TUPLE4]]
	]
]

readMORHFILLSTYLE: has[type][
	reduce [
		type: binary/read bin 'UI8 ;FillStyleType
		reduce case [
			type = 0 [;solid fill
				binary/read bin [TUPLE4 TUPLE4]
			]
			any [
				type = 16 ;linear gradient fill
				type = 18 ;radial gradient fill
				type = 19 ;focal gradient fill (swf8)
			][;gradient
				[readMATRIX readMATRIX readMORPHGRADIENT]
			]
			type >= 64 [;bitmap
				[readUsedID readMATRIX readMATRIX]
			]
		]
	]
]

readMORPHGRADIENT: has[count gradients][
	count: binary/read bin 'UI8
	gradients: make block! count
	loop count [
		append/only gradients binary/read bin [;readMORPHGRADRECORD
			UI8    ;Start ratio
			TUPLE4 ;Start color
			UI8    ;End ratio
			TUPLE4 ;End color
		]
	]
	gradients
]

read-DefineMorphShape:  does [
	reduce [
		binary/read bin 'UI16LE ;ID
		readRECT                ;StartBounds
		readRECT                ;EndBounds
		binary/read bin 'UI32LE ;Offset
		readMORPHFILLSTYLEARRAY
		readMORPHLINESTYLEARRAY
		readSHAPERECORD         ;StartEdges
		readSHAPERECORD         ;EndEdges
	]
]

read-DefineMorphShape2:  does [
	reduce [
		binary/read bin 'UI16LE ;ID
		readRECT                ;StartBounds
		readRECT                ;EndBounds
		readRECT                ;StartEdgeBounds
		readRECT                ;EndEdgeBounds
		(
			binary/read/with bin 'UB 6 ;reserved
			binary/read bin 'BIT ;usesNonScalingStrokes
		)
		binary/read bin 'BIT     ;usesScalingStrokes
		binary/read bin 'UI32LE  ;Offset
		readMORPHFILLSTYLEARRAY
		readMORPHLINESTYLEARRAY
		readSHAPERECORD          ;StartEdges
		readSHAPERECORD          ;EndEdges
	]
]


;-------------------------------------------------------------------------
;-------------------------------------------------------------------------

tag-decoders: make map! reduce [
	2  :read-DefineShape
	4  :read-PlaceObject
	5  :read-RemoveObject
	6  :read-DefineBitsJPEG2
	7  :read-DefineButton
	9  :readRGB
	10 :read-DefineFont
	11 :read-DefineText
	12 :read-DoAction
	13 :read-DefineFontInfo ;v1
	14 :read-DefineSound
	15 :read-StartSound
	17 :read-DefineButtonSound
	18 :read-SoundStreamHead
	;19 SoundStreamBlock ;= just binary data
	20 :read-DefineBitsLossless
	21 :read-DefineBitsJPEG2
	22 :read-DefineShape
	23 :read-DefineButtonCxform
	;24 none ;Protected file!
	26 :read-PlaceObject2
	28 :read-RemoveObject2
	32 :read-DefineShape
	33 :read-DefineText
	34 :read-DefineButton2
	35 :read-DefineBitsJPEG3
	36 :read-DefineBitsLossless
	37 :read-DefineEditText
	;38 DefineVideo ;= not officially supported!
	39 :read-DefineSprite
	40 :read-SWT-CharacterName
	;41 SerialNumber
	;42 DefineTextFormat ;= not officially supported!
	43 :readSTRING ;FrameLabel
	45 :read-SoundStreamHead
	46 :read-DefineMorphShape
	48 :read-DefineFont3 ;DefineFont2
	56 :read-ExportAssets
	57 :read-ImportAssets
	;58 EnableDebugger
	59 :read-DoInitAction
	60 :read-DefineVideoStream
	61 :read-VideoFrame
	62 :read-DefineFontInfo ;v2
	;63 DebugID
	;64 EnableDebugger2
	65 :read-ScriptLimits
	66 :read-SetTabIndex
	67 :read-DefineShape ;DefineShape4
	69 :read-FileAttributes
	70 :read-PlaceObject3
	71 :read-ImportAssets2
	72 :read-DoABC ;define
	73 :read-DefineAlignZones
	74 :read-CSMTextSettings
	75 :read-DefineFont3
	76 :read-SymbolClass
	77 :readSTRING ;MetaData
	78 :read-DefineScalingGrid
	82 :read-DoABC
	83 :read-DefineShape
	84 :read-DefineMorphShape2
	86 :read-SceneAndFrameLabelData
	;87 DefineBinaryData ;= just binary data
	88 :read-DefineFontName
	89 :read-StartSound2
	90 :read-DefineBitsJPEG4
	91 :read-DefineFont4
]

register-codec [
	name:  'swf
	type:  'application
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
		fileSize: (binary/read bin 'UI32LE) - 8
		if verbose > 0 [
			print [
				"SWF file version:" version
				select [
					67 "compressed using deflate"
					70 "uncompressed"
					90 "compressed using LZMA"
				] compression
				lf
				"Data size:" fileSize "bytes"
			]
		]
		
		switch compression [
			67 [
				binary/init bin decompress/size bin/buffer 'zlib fileSize
			]
			90 [
				packed: binary/read bin 'UI32LE
				binary/init bin decompress/size bin/buffer 'lzma fileSize
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

	decode-tag: func [id [integer!] data [binary! none!] ][
		tid: id
		bin: binary data
		any [tag-decoders/:id data]
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