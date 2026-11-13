Rebol [
	Title:    "Test MIDI port"
	Date:     5-Jun-2023
	Author:   "Oldes"
	File:     %test-midi.r3
	Version:  1.0.0
	Needs:    3.11.0
]

midi: query midi:// [devices-in devices-out]
unless block? midi [ print as-purple "No MIDI available!" quit]

print [as-yellow "Input devices: " length? midi/devices-in]
foreach dev midi/devices-in  [ print [tab mold dev] ]
print [as-yellow "Output devices:" length? midi/devices-out]
foreach dev midi/devices-out [ print [tab mold dev] ]


*MIDI_message: enum [
	note-off:          2#1000
	note-on:           2#1001
	poly-aftertouch:   2#1010
	control-change:    2#1011
	program-change:    2#1100
	channel-preasure:  2#1101
	pitch_bend-change: 2#1110
] "MIDI Voice message"

*MIDI_message_short: enum [
	Off:   2#1000
	On:    2#1001
	PoPr:  2#1010
	Par:   2#1011
	PrCh:  2#1100
	ChPr:  2#1101
	Pb:    2#1110
] "MIDI Voice message (short variant)"


process-midi: function [data [binary!]][
	bin: binary data
	while [(length? bin/buffer) >= 8][
		binary/read bin [
			status:  UB 4
			channel: UB 4
			byte-1:  SI8
			byte-2:  SI8
			byte-3:  SI8 ;???
			time:    UI32LE
		]
		op: *MIDI_message_short/name status 
		prin  ajoin [time SP op " ch=" channel SP]
		print ajoin switch/default op [
			Par     [["c=" byte-1 " v=" byte-2]]
			Pb ChPr [reduce ["v=" byte-1 + (byte-2 << 7)]]
			PrCh    [["p=" byte-1]]
		][
			["n=" byte-1 " v=" byte-2]
		]
	]
]

close-midi: does [
	try [close midi-out]
	try [close midi-inp]
	wait 0
]

either port? midi-out: try [
	open [
		scheme: 'midi
		device-out: "Microsoft GS Wavetable" ;; note that the name is not complete, but still accepted
	]
][
	;; Play some random modern piano music ;-)
	loop 50 [
		write midi-out rejoin [
			;;    NOTE            VOLUME
			#{90} random 127 50 + random 77 0
			#{90} random 127 50 + random 77 0
			#{90} random 127 50 + random 77 0
		]
		;; Random time between notes :)
		wait 0.5 + random 0.5
	]
	try [close midi-out]
	wait 0
][
	;; No SW synth...
	print as-purple "Not playing the great piano music, because no SW synth found!"
]

print as-yellow "You have 10 seconds to try your (first) MIDI device input!"
midi-inp: open midi:1
midi-inp/awake: function [event [event!]][
	switch event/type [
		read  [ process-midi read event/port    ]
		open  [ print ["MIDI port opened!" event/port/spec/ref] ]
		close [ print ["MIDI port closed!" event/port/spec/ref] ]
	]
	true
]
wait 10
close-midi


if system/options/script [ask "DONE"]