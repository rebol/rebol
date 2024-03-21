REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Boot Sys: Port and Scheme Functions"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Context: sys
	Note: {
		The boot binding of this module is SYS then LIB deep.
		Any non-local words not found in those contexts WILL BE
		UNBOUND and will error out at runtime!
	}
]

append/only system/schemes [
	title: "Audio"
	name: 'audio
	spec: system/standard/port-spec-audio
	init: func [port /local spec source wav] [
		spec: port/spec
		if all [
			url? spec/ref
			parse spec/ref ["audio:" copy source to end]
		][
			spec/source: as file! source
		]
		; make spec to be only with audio related keys
		spec: make copy system/standard/port-spec-audio spec
		if file? spec/source [
			try/except [
				wav: decode 'wav read spec/source
				spec/channels: wav/channels
				spec/rate: wav/rate
				spec/bits: wav/bits
				port/data: wav/data
			][ return false ]
		]
		port/spec: :spec
		;probe spec
		;protect/words port/spec ; protect spec object keys of modification
		true
	]
]
