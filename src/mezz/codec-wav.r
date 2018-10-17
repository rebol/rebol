REBOL [
	title:  "REBOL 3 codec for WAV file format"
	name:   'codec-WAV
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

register-codec [
	name:  'WAV
	title: "Waveform Audio File Format"
	suffixes: [%.wav %.wave]

	decode: function [
		data [binary!]
	][
		if verbose > 0 [
			print ["^[[1;32mDecode WAV data^[[m (^[[1m" length? data "^[[mbytes )"]
			; count maximal bytes width (used for padding)
			w: 1 + length? form length? data
		]

		bin: binary data
		binary/read bin [
			chunkId:   BYTES 4
			chunkSize: UI32LE
			format:    BYTES 4
		]

		if all [
			chunkId = #{52494646} ;RIFF
			format  = #{57415645} ;WAVE
		][
			; looks like valid WAVE file
			data:   copy []
			chunks: copy []
			
			while [not tail? bin/buffer][
				binary/read bin [
					id:     BYTES 4
					size:   UI32LE
					starts: INDEX
				]
				ends: starts +  size
				chunk: any [ try [to tag! id]  id ]
				if verbose > 0 [
					printf [
						$32
						"CHUNK: " $1 7 $0
						"at: "    $1 w $0
						"bytes: " $1 w $0
					] reduce [mold chunk starts size]
				]
				append chunks chunk
				switch/default chunk  [
					<fmt > [
						format: binary/read bin [
							UI16LE        ; compression
							UI16LE        ; channels
							UI32LE        ; sampleRate
							UI32LE        ; bytesPerSec
							UI16LE        ; blockAlign
							UI16LE        ; bitsPerSample
						]
						if size > 16 [
							size: size - 16
							append format copy/part bin/buffer size
							bin/buffer: skip bin/buffer size
						]
						append/only chunks format
					]
					<data> [
						binary/read/into bin [BYTES :size] tail data
						append chunks size ; so one could reconstruct the data chunk in encoding
					]
					<smpl> [
						sampler: binary/read bin [
							UI32LE        ; Manufacturer
							UI32LE        ; Product
							UI32LE        ; Sample Period
							UI32LE        ; MIDI Unity Note
							UI32LE        ; MIDI Pitch Fraction
							UI32LE        ; SMPTE Format
							UI32LE        ; SMPTE Offset
							count: UI32LE ; Num Sample Loops
							UI32LE        ; Sampler Data
						]
						append/only sampler loops: copy []
						loop count [
							binary/read/into bin [
								UI32LE    ; Cue Point ID
								UI32LE    ; Type
								UI32LE    ; Start
								UI32LE    ; End
								UI32LE    ; Fraction
								UI32LE    ; Play Count
							] loops
						]
						append/only chunks sampler
					]
					<fact> [
						binary/read/into bin [BYTES :size] tail chunks
					]
					<cue > [
						count: binary/read bin 'UI32LE
						append chunks cues: copy []
						loop count [
							binary/read/into bin [
								UI32LE    ; id            - unique identification value
								UI32LE    ; Position      - play order position
								UI32LE    ; Data Chunk ID - RIFF ID of corresponding data chunk
								UI32LE    ; Chunk Start   - Byte Offset of Data Chunk *
								UI32LE    ; Block Start   - Byte Offset to sample of First Channel
								UI32LE    ; Sample Offset - Byte Offset to sample byte of First Channel
							] tail cues
						]
						new-line/skip cues true 6
					]
					<_PMX> [
						; Extensible Metadata Platform (XMP) data
						;@@ https://www.adobe.com/products/xmp.html
						binary/read bin [tmp: BYTES :size]
						try [tmp: to string! tmp]
						append chunks tmp
						if verbose > 1 [printf [$33 tmp $0] ""]  
					]
				][
					binary/read/into bin [BYTES :size] tail chunks
				]
				if ends <> index? bin/buffer [
					cause-error 'script 'bad-bad ["WAV decode" "invalid chunk end"]
				]
			]
		]
		either any [empty? chunks  none? format] [
			none
		][
			new-line/skip chunks true 2
			object compose/only [
				type:     'wave
				rate:     (format/3)
				channels: (format/2)
				bits:     (format/6)
				chunks:   (chunks)
				data:     (either empty? data [none][rejoin data])
			]
		]
	]
	identify: func [
		"Returns TRUE if binary looks like WAV data"
		data [binary!]
	][
		parse data [#{52494646} 4 skip #{57415645} to end]
	]

	verbose: 0
]