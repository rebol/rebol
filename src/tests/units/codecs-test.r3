Rebol [
	Title:   "Rebol codecs test script"
	Author:  "Oldes"
	File: 	 %codecs-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Codecs"

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

~~~end-file~~~