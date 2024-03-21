Rebol [
	Title:    "Test Audio port"
	Date:     5-Jul-2023
	Author:   "Oldes"
	File:     %test-audio.r3
	Version:  1.0.0
]

;- This is just a prototype... it will be probably changed.

on-sound-event: func[event][print event/type true]

try/with [
	print "^/Simply loading a WAV and playing it once"
	sound: open audio:units/files/drumloop.wav
	sound/awake: :on-sound-event
	insert sound on

	wait [sound 3]
	wait 0.2

	print "^/Loading a WAV and playing it as a loop"
	sound: open [
		scheme: 'audio
		source: %units/files/drumloop.wav
		loop-count: -1 ;; infinite loop!
	]
	sound/awake: :on-sound-event

	insert sound on
	n: 1
	while [port? wait [sound 5]][
		if ++ n = 5 [prin LF break]
	]
	insert sound off

] :print



if system/options/script [ask "DONE"]