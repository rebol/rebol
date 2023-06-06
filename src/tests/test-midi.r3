Rebol [
	Title:    "Test MIDI port"
	Date:     5-Jun-2023
	Author:   "Oldes"
	File:     %test-midi.r3
	Version:  1.0.0
	Requires: 3.11.0
]

midi: query midi://
unless object? midi [ print as-purple "No MIDI available!" quit]

print [as-yellow "Input devices: " length? midi/devices-in]
foreach dev midi/devices-in  [ print [tab mold dev] ]
print [as-yellow "Output devices:" length? midi/devices-out]
foreach dev midi/devices-out [ print [tab mold dev] ]


*MIDI_message: enum [
	note-off:          2#{0000 1000}
	note-on:           2#{0000 1001}
	poly-aftertouch:   2#{0000 1010}
	control-change:    2#{0000 1011}
	program-change:    2#{0000 1100}
	channel-preasure:  2#{0000 1101}
	pitch_bend-change: 2#{0000 1110}
] "MIDI Voice message"

*MIDI_message_short: enum [
	Off:   2#{0000 1000}
	On:    2#{0000 1001}
	PoPr:  2#{0000 1010}
	Par:   2#{0000 1011}
	PrCh:  2#{0000 1100}
	ChPr:  2#{0000 1101}
	Pb:    2#{0000 1110}
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
		prin  ajoin [time #" " op " ch=" channel #" "]
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

midi-inp: open midi:1
midi-out: open [scheme: 'midi device-out: 1]

midi-out/awake:
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
halt
;; Play some random modern piano music ;-)
;; MIDI input should be printed in the console.
loop 50 [
	write midi-out rejoin [
		#{90} random 127 50 + random 77 0
		#{90} random 127 50 + random 77 0
		#{90} random 127 50 + random 77 0
	]
	wait 0.5 + random 0.5
]

close-midi

if system/options/script [ask "DONE"]