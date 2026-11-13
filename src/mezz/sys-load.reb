Rebol [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Boot Sys: Load, Import, Modules"

	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2023 Rebol Open Source Contributors
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

		These functions are kept in a single file because they
		are inter-related.
	}

	Comment: {
		BASICS:
		Code gets loaded in two ways:

		 1. As user code/data - residing in user context
		 2. As module code/data - residing in its own context

		Module loading can be delayed. This allows special modules like CGI, protocols,
		or HTML formatters to be available, but not require extra space.

		The system/modules list holds modules for fully init'd modules, otherwise it
		holds their headers, along with the binary or block that will be used to init them.
	}
]

intern: function [
	"Imports (internalizes) words/values from the lib into the user context."

	context [block! any-word!]
	"Word or block of words to be added (deeply)"
][
	index: 1 + length? user-context: system/contexts/user
	; for optimization below (index for resolve)

	context: bind/new :context user-context
	; Extend the user context with new words

	resolve/only user-context lib index
	; Copy only the new values into the user context

	:context
]

bind-lib: func [
	"Bind only the top words of the block to the lib context (used to load mezzanines)."

	body [block!]
][
	bind/only/set body lib
	; Note: not bind/new !

	bind body lib

	body
]

export-words: func [
	"Exports the words of a context into both the system lib and user contexts."

	context [module! object!]
	"Module context"

	words [block! none!]
	"The exports words block of the module"
][
	if words [
		resolve/extend/only lib context words
		; words already set in lib are not overriden

		resolve/extend/only system/contexts/user lib words
		; lib, because of above
	]
]

mixin?: func [
	"Returns TRUE if module is a mixin with exports."

	header [module! object!]
	"Module or spec header"
][
	; Note: Unnamed modules DO NOT default to being mixins.

	if module? header [
		header: spec-of header
		; Get the header object
	]

	did all [
		find select header 'options 'private

		block? select header 'exports
		; If there are no exports, there's no difference

		not empty? select header 'exports
	]
]

load-header: function/with [
	"Loads script header object and body binary (not loaded)."

	source [binary! string!]
	"Source code (string! will be UTF-8 encoded)"

	/only
	"Only process header, don't decompress or checksum body"

	/required
	"Script header is required"
][
	; This function decodes the script header from the script body.
	; It checks the header 'checksum and 'compress and 'content options,
	; and supports length-specified or script-in-a-block embedding.
	;
	; It will set the 'content field to the binary source if 'content is true.
	; The 'content will be set to the source at the position of the beginning
	; of the script header, skipping anything before it. For multi-scripts it
	; doesn't copy the portion of the content that relates to the current
	; script, or at all, so be careful with the source data you get.
	;
	; If the 'compress option is set then the body will be decompressed.
	; Binary vs. script encoded compression will be autodetected. The
	; header 'checksum is compared to the checksum of the decompressed binary.
	;
	; Normally, returns the header object, the body text (as binary), and the
	; the end of the script or script-in-a-block. The end position can be used
	; to determine where to stop decoding the body text. After the end is the
	; rest of the binary data, which can contain anything you like. This can
	; support multiple scripts in the same binary data, multi-scripts.
	;
	; If not /only and the script is embedded in a block and not compressed
	; then the body text will be a decoded block instead of binary, to avoid
	; the overhead of decoding the body twice.
	;
	; Syntax errors are returned as words:
	;    no-header
	;    bad-header
	;    bad-checksum
	;    bad-compress
	;
	; Note: set/any and :var used - prevent malicious code errors.
	; Commented assert statements are for documentation and testing.
	;
	case/all [
		binary? source [
			parse source [
				; utf-16 & utf-32
				#{0000FEFF}
				source:
				(source: iconv/to source 'utf-32BE 'utf8)
				|
				#{FFFE0000}
				source:
				(source: iconv/to source 'utf-32LE 'utf8)
				|
				#{FEFF}
				source:
				(source: iconv/to source 'utf-16BE 'utf8)
				|
				#{FFFE}
				source:
				(source: iconv/to source 'utf-16LE 'utf8)
				|
				; utf-8 (skip the BOM if found)
				opt [
					#{EFBBBF}
					source:
				]
				(source: assert-utf8 source)
			]
		]

		string? source [
			source: to binary! source
		]

		not start: script? source [
			; no script header found

			return either required [
				'no-header
			][
				reduce [
					_ source tail source
				]
			]
		]

		set/any [keyword: mark: line:] transcode/only/line start 1 _
		; get 'rebol keyword

		set/any [header: mark: line:] transcode/next/error/line mark :line _
		; get header block

		any [
			not block? :header
			; header block is incomplete

			not object? header: try [
				construct/with :header system/standard/header
			]
		][
			return 'bad-header
		]

		word? :header/options [
			header/options: reduce [
				header/options
			]
		]

		not any [
			block? :header/options
			none? :header/options
		][
			return 'bad-header
		]

		not any [
			binary? :header/checksum
			none? :header/checksum
		][
			return 'bad-checksum
		]

		not tuple? :header/version [
			header/version: _
			; could exceptions be made for integers and decimals here?
		]

		find header/options 'content [
			repend header [
				'content start
				; as of start of header
			]
		]

		13 = mark/1 [
			mark: next mark
			; skip CR
		]

		10 = mark/1 [
			mark: next mark
			; skip LF

			++ line
		]

		integer? length: select header 'length [
			remaining: skip mark length
		]

		not remaining [
			remaining: tail start
		]

		only [
			return reduce [
				header mark remaining line
			]
		]

		hash: header/checksum _
		; [print hash]
		; decompress and checksum not done
		; none saved to simplify later code

		:keyword = 'rebol [
			; regular script, binary or script encoded compression supported

			case [
				find header/options 'compress [
					mark: any [
						find mark non-ws
						mark
						; skip whitespace after header
					]

					unless mark: any [
						; automatic detection of compression type
						; @@note: can sniff for ZLIB here
						; typically ZLIB header begins: 78 [01 | 5E | 9C | DA]
						; https://stackoverflow.com/a/54915442/292969

						attempt [
							decompress/part mark 'zlib remaining
						]
						; binary compression

						attempt [
							decompress first transcode/next mark 'zlib
						]
						; script encoded
					][
						return 'bad-compress
					]

					if all [
						hash
						hash != checksum mark 'sha1
					][
						return 'bad-checksum
					]
				]

				; else assumed not compressed

				all [
					hash
					hash != checksum/part mark 'sha1 remaining
				][
					return 'bad-checksum
				]
			]
		]

		; assert/type [mark [binary!]] _

		:keyword != 'rebol [
			; block-embedded script, only script compression, header/length ignored

			set [body: remaining:] transcode/next start
			; decode embedded script

			body: skip body 2
			; we know from SCRIPT? that we have a valid `Rebol []` combo

			remaining: back remaining
			; reset remaining before the closing bracket

			case [
				find header/options 'compress [
					; script encoded only

					if error? body: try [
						decompress first body 'zlib
					][
						return 'bad-compress
					]

					if all [
						hash
						hash != checksum next remaining 'sha1
					][
						return 'bad-checksum
					]
				]

				all [
					hash
					hash != checksum/part mark 'sha1 remaining
				][
					return 'bad-checksum
				]
			]
		]

		; assert/type [body [block! binary!]] _
	]

	; assert/type [header object! mark [binary! block!] remaining binary!]
	; assert/type [header/checksum [binary! none!] header/options [block! none!]]

	reduce [
		header mark remaining line
	]
][
	non-ws: make bitset! [
		not 1 - 32
	]
]

load-ext-module: function [
	"Loads an extension module from an extension object."

	extension [object!]
	"Extension object (from LOAD-EXTENSION, modified)"

	; /local -- don't care if cmd-index and command are defined local
][
	; for ext obj: help system/standard/extensions
	assert/type [
		extension/lib-base handle!
		extension/lib-boot binary!
		; Just in case
	]

	if word? set [header: code:] load-header/required extension/lib-boot [
		cause-error 'syntax header extension
		; word returned is error code
	]

	; assert/type [header object! header/options [block! none!] code [binary! block!]]

	log/debug 'REBOL [
		"Extension:" select header 'title
	]

	unless header/options [
		header/options: make block! 1
	]

	append header/options 'extension
	; So make module! special cases it

	header/type: 'module
	; So load and do special case it

	extension/lib-boot: _
	; So it doesn't show up in the source

	body: body-of extension
	; Special extension words

	; Define default extension initialization if needed:
	; It is overridden when extension provides it's own COMMAND func.
	;
	unless :extension/command [
		append body [
			cmd-index: 0

			command: func [
				"Define a new command for an extension."
				args [integer! block!]
			][
				; (contains module-local variables)

				make command! reduce [
					args
					self
					++ cmd-index
				]
			]

			protect/hide/words [
				cmd-index command
			]
		]
	]

	; Convert the code to a block if not already:
	;
	unless block? code [
		code: make block! code
	]

	insert code body
	; Extension object fields and values must be first!

	reduce [
		header code
		; ready for make module!
	]
]

load-boot-exts: function [
	"INIT: Load boot-based extensions."
][
	log/debug 'REBOL "Loading boot extensions..."

	extensions: []

	foreach [spec caller] boot-exts [
		append extensions load-extension/dispatch spec caller
	]

	foreach extension extensions [
		case/all [
			word? set [header: body:] load-header/only/required extension/lib-boot [
				cause-error 'syntax header extension
				; word returned is error code
			]

			not word? :header/name [
				header/name: _
			]

			not any [
				header/name find header/options 'private
			][
				header/options: append any [
					header/options
					make block! 1
				] 'private
			]

			delay: did all [
				header/name
				find header/options 'delay
			][
				module: reduce [
					header extension
				]
				; load it later
			]

			not delay [
				header: spec-of module: make module! load-ext-module extension
			]

			; NOTE: This will error out if the code contains commands but
			; no extension dispatcher (call) has been provided.
			;
			header/name [
				repend system/modules [
					header/name module
				]
			]
		]

		case [
			not module? module _

			not block? select header 'exports _

			empty? header/exports _

			find header/options 'private [
				resolve/extend/only system/contexts/user module header/exports
				; full export to user
			]

			'else [
				export-words module header/exports
			]
		]
	]

	set 'boot-exts 'done

	set 'load-boot-exts 'done
	; only once
]

read-decode: function [
	"Reads code/data from source or DLL, decodes it, returns result (binary, block, image,...)."

	source [file! url!]
	"Source or block of sources?"

	type [word! none!]
	"File type, or NONE for binary raw data"
][
	either 'extension = type [
		; DLL-based extension
		; Try to load it (will fail if source is a url)
		;
		content: load-extension source
		; returns an object or throws an error
	][
		content: read source
		; can be string, binary, block

		if find system/catalog/file-types type [
			; e.g. not 'unbound

			content: decode type :content
		]
	]

	content
]

load: function [
	"Loads code or data from a file, URL, string, or binary."

	source [file! url! string! binary! block!]
	"Source or block of sources"

	/header
	"Result includes Rebol header object (preempts /all)"

	/all
	"Load all values (does not evaluate Rebol header)"

	/as
	"Override default file-type; use NONE to always load as code"

	type [word! none!]
	"E.g. text, markup, jpeg, unbound, etc."
][
	; WATCH OUT: for ALL and NEXT words! They are local.

	; NOTES:
	; Note that code/data can be embedded in other datatypes, including
	; not just text, but any binary data, including images, etc. The type
	; argument can be used to control how the raw source is converted.
	; Pass a /as of none or 'unbound if you want embedded code or data.
	; Scripts are normally bound to the user context, but no binding will
	; happen for a module or if the /as is 'unbound. This allows the result
	; to be handled properly by DO (keeping it out of user context.)
	; Extensions will still be loaded properly if /type is 'unbound.
	; Note that IMPORT has its own loader, and does not use LOAD directly.
	; /as with anything other than 'extension disables extension loading.

	assert/type [
		local none!
		; easiest way to protect against /local hacks
	]

	case/all [
		header [
			all: _
		]

		; -- Load multiple sources?
		;
		block? source [
			return map-each item source [
				load/:header/:all/:as :item type
			]
		]

		; -- What type of file? Decode it too:
		;
		any [
			file? source
			url? source
		][
			detected-type: file-type? source

			type: case [
				lib/all [
					'unbound = type
					'extension = detected-type
				][
					detected-type
				]

				as [
					type
				]

				'else [
					detected-type
				]
			]

			body: read-decode source type

			if not find [0 extension unbound] any [type 0] [
				return body
			]
		]

		none? body [
			body: source
		]

		; -- Is it not source code? Then return it now:
		;
		any [
			block? body

			not find [0 extension unbound] any [type 0]
			; due to make-boot issue with #[none]
		][
			unless type [
				return body
			]

			try [
				return decode type to binary! body
			]

			cause-error 'access 'no-codec type
		]

		; -- Try to load the HEADER, handle error:
		;
		not all [
			set [script: body: remaining: line:] either object? body [
				load-ext-module body
			][
				load-header body
			]

			if word? script [
				cause-error 'syntax script source
			]

			unless tail? remaining [
				body: copy/part body remaining
			]
		]

		; BODY is binary or block, SCRIPT is object or none

		; -- Convert code to block, insert header if requested:
		;
		not block? body [
			body: transcode/line body any [
				line 1
			]
		]

		header [
			insert body script
		]

		; -- Bind code to user context:
		;
		not any [
			'unbound = type
			'module = select script 'type
			find select script 'options 'unbound
		][
			body: intern body
		]

		; -- If appropriate and possible, return singular BODY value:
		not any [
			all
			header
			empty? body
			1 < length? body
		][
			set/any 'body first body
		]
	]

	:body
]

do-needs: function [
	"Process the NEEDS block of a program header. Returns unapplied mixins."

	needs [block! object! tuple! none!]
	"Needs block, header or version"

	/no-share
	"Force module to use its own non-shared global namespace"

	/no-lib
	"Don't export to the runtime library"

	/no-user
	"Don't export to the user context (mixins returned)"

	/block
	"Return all the imported modules in a block, instead"
][
	; NOTES:
	; This is a low-level function and its use and return values reflect that.
	; In user mode, the mixins are applied by IMPORT, so they don't need to
	; be returned. In /no-user mode the mixins are collected into an object
	; and returned, if the object isn't empty. This object can then be passed
	; to MAKE module! to be applied there. The /block option returns a block
	; of all the modules imported, not any mixins - this is for when IMPORT
	; is called with a Needs block.

	case/all [
		; If it's a header object:
		;
		object? needs [
			set/any 'needs select needs 'needs
			; (protected)
		]

		none? needs [
			return _
		]

		; If simple version number check:
		;
		tuple? :needs [
			case [
				needs > system/version [
					cause-error 'syntax 'needs reduce [
						'core needs
					]
				]

				3 >= length? needs _
				; no platform id check needed

				(needs and 0.0.0.255.255) != (system/version and 0.0.0.255.255) [
					; must match

					cause-error 'syntax 'needs reduce [
						'core needs
					]
				]
			]

			return _
		]

		; If it's an inline value, put it in a block:
		;
		not block? :needs [
			needs: reduce [
				:needs
			]
		]

		empty? needs [
			return _
		]
	]

	; Parse the needs dialect [source |version| |checksum-hash|]
	;
	modules: make block! length? needs

	name:
	version:
	hash: _

	unless parse needs [
		mark:

		opt [
			opt 'core
			set version tuple!
			(do-needs version)
		]

		any [
			mark:

			set name [word! | file! | url!]
			set version opt tuple!
			set hash opt binary!

			(
				repend modules [
					name version hash
				]
			)
		]
	][
		cause-error 'script 'invalid-arg mark
	]

	; Temporary object to collect exports of "mixins" (private modules).
	; Don't bother if returning all the modules in a block, or if in user mode.
	;
	if all [
		no-user
		not block
	][
		mixins: make object! 0
		; Minimal length since it may persist later
	]

	; Import the modules:
	;
	modules: map-each [name version hash] modules [
		; Import the module

		module: apply :import [
			name
			did version
			version
			did hash
			hash
			no-share
			no-lib
			no-user
		]

		; Collect any mixins into the object (if we are doing that)
		;
		if all [
			mixins
			mixin? module
		][
			resolve/extend/only mixins module select spec-of module 'exports
		]

		module
	]

	case [
		block [
			modules
			; /block: return block of modules
		]

		not empty? mixins [
			mixins
			; else return mixins, if any
		]
	]
]

load-module: function [
	"Loads a module (from a file, URL, binary, etc.) and inserts it into the system module list."

	source [word! file! url! string! binary! module! block!]
	"Source or block of sources"

	/version

	needs [tuple!]
	"Module must be this version or greater"

	/check

	hash [binary!]
	"Match SHA1 checksum (must be set in header)"

	/no-share
	"Force module to use its own non-shared global namespace"

	/no-lib
	"Don't export to the runtime library (lib)"

	/import
	"Do module import now, overriding /delay and 'delay option"

	/as

	name [word!]
	"New name for the module (not valid for reloads)"

	/delay
	"Delay module init until later (ignored if source is module!)"
][
	; NOTES:
	; This is a variation of LOAD that is used by IMPORT. Unlike LOAD, the module init
	; may be delayed. The module may be stored as binary or as an unbound block, then
	; init'd later, as needed.
	;
	; The checksum applies to the uncompressed binary source of the body, and
	; is calculated in LOAD-HEADER if the 'checksum header field is set.
	; A copy of the checksum is saved in the system modules list for security.
	; /no-share and /delay are ignored for module! source because it's too late.
	; A name is required for all imported modules, delayed or not; /as can be
	; specified for unnamed modules. If you don't want to name it, don't import.
	; If source is a module that is loaded already, /as name is an error.
	;
	; Returns block of name, and either built module or none if delayed.
	; Returns none if source is word and no module of that name is loaded.
	; Returns none if source is file/url and read or load-extension fails.
	; sys/log/info 'REBOL ["load-module:" source]

	assert/type [
		local none!
		; easiest way to protect against /local hacks
	]

	if import [
		delay: _
		; /import overrides /delay
	]

	; Process the source, based on its type
	;
	case [
		word? source [
			; loading the preloaded

			case/all [
				as [
					cause-error 'script 'bad-refine /as
					; no renaming
				]

				not module: select system/modules source [
					return _
					; Return none if no module of that name found
				]

				; assert/type [module [module! block!] module-hash [binary! none!]] _

				all [
					; If no further processing is needed, shortcut return

					not version
					not check
					any [
						delay
						module? :module
					]
				][
					return reduce [
						source if module? :module [
							module
						]
					]
				]
			]
		]

		binary? source [
			module-content: source
		]

		string? source [
			module-content: to binary! source
		]

		any [
			file? source
			url? source
		][
			if file? source [
				source: any [
					to-real-file source
					; if possible, make absolute source path

					source
				]
			]

			detected-type: file-type? source

			case [
				not detected-type [
					; @@what's the value in eliding errors reading source here?
					;
					unless attempt [
						module-content: read source
					][
						return _
						; Return none if read or load-extension fails
					]
				]

				detected-type = 'extension [
					; special processing for extensions
					; load-extension also fails for url!

					try/with [
						extension: load-extension source
					][
						log/error 'REBOL system/state/last-error
						return _
					]

					module-content: extension/lib-boot
					; save for checksum before it's unset

					case [
						import [
							set [module-header: module-code:] load-ext-module extension
						]

						word? set [module-header: tmp:] load-header/only/required module-content [
							cause-error 'syntax module-header source
							; word is error code
						]

						not any [
							delay
							delay: did find module-header/options 'delay
						][
							set [module-header: module-code:] load-ext-module extension
							; import now
						]
					]

					if module-header/checksum [
						module-hash: copy module-header/checksum
					]
				]

				'else [
					cause-error 'access 'no-script source
					; needs better error
				]
			]
		]

		module? source [
			module: source

			module-header: spec-of module

			; see if the same module is already in the list
			;
			if all [
				module-header/name
				module? existing: select system/modules module-header/name
			][
				if as [
					cause-error 'script 'bad-refine /as
					; already imported
				]

				; ; the original code:
				; ; https://github.com/rebol/rebol/blob/25033f897b2bd466068d7663563cd3ff64740b94/src/mezz/sys-load.r#L488-L490
				; ; system/modules was a block with [name module modsum ...]

				; ; For now I will return existing module when there was not used /version and /check
				; ; but it must be revisited and handled correctly! So far there is not good support
				; ; for modules with same name but different versions:-/

				; ; Main purpose of this code is to reuse existing module in cases like
				; ; running: `do "rebol [type: module name: n]..."` multiple times

				if all [
					not version
					not check
					equal? module existing
				][
					return reduce [
						module-header/name existing
					]
				]
			]
		]

		block? source [
			if any [
				version
				check
				as
			][
				cause-error 'script 'bad-refines _
			]

			module-content: make block! length? source

			unless parse source [
				any [
					mark:
					set name opt set-word!

					set module [
						word! | module! | file! | url! | string! | binary!
					]

					set needs opt tuple!

					set hash opt binary!
					; ambiguous

					(
						repend module-content [
							module
							needs
							hash
							if name [
								to word! name
							]
						]
					)
				]
			][
				cause-error 'script 'invalid-arg mark
			]

			; this was MAP-EACH [...] SOURCE
			;
			return map-each [module needs hash name] module-content [
				apply :load-module [
					module
					did needs
					needs
					did hash
					hash
					no-share
					no-lib
					import
					did name
					name
					delay
				]
			]
		]
	]

	case/all [
		module? module [
			; Get info from preloaded or delayed modules

			delay:
			no-share: _

			module-header: spec-of module

			assert/type [
				module-header/options [block! none!]
			]
		]

		block? module [
			; module/block module used later for override testing

			set/any [module-header: module-code:] module
		]

		url? module [
			; used by `import` for downloading extensions

			return _
		]

		; Get and process the header
		;
		not module-header [
			; Only happens for string, binary or non-extension file/url source

			set [module-header: module-code: remaining:] load-header/required module-content

			case [
				word? module-header [
					cause-error 'syntax module-header source
				]

				import _
				; /import overrides 'delay option

				not delay [
					delay: did find module-header/options 'delay
				]
			]

			unless tail? remaining [
				module-code: copy/part module-code remaining
			]

			if module-header/checksum [
				module-hash: copy module-header/checksum
			]
		]

		no-share [
			module-header/options: append any [
				module-header/options
				make block! 1
			] 'isolate
		]

		name [
			; Unify module-header/name and /as name

			module-header/name: name
			; rename /as name
		]

		not name [
			set/any 'name :module-header/name
		]

		all [
			not no-lib
			not word? :name
			; requires name for full import
		][
			no-lib: yes
			; Unnamed module can't be imported to lib, so /no-lib here
			; Still not /no-lib in IMPORT

			unless find module-header/options 'private [
				; But make it a mixin and it will be imported directly later

				module-header/options: append any [
					module-header/options make block! 1
				] 'private
			]
		]

		not tuple? set/any 'module-version :module-header/version [
			module-version: 0.0.0
			; get version
		]

		; See if it's there already, or there is something more recent
		;
		all [
			override?: not no-lib
			; set to false later if existing module is used

			existing: select system/modules name
		][
			; Get existing module's info

			case/all [
				module? :existing [
					existing-header: spec-of existing
					; final header
				]

				block? :existing [
					existing-header: first existing
					; cached preparsed header
				]

				url? :existing [
					existing-header: object [
						version: 0.0.0
						url: :existing
						checksum: _
					]
				]

				; assert/type [existing-name word! existing-header object! existing-hash [binary! none!]] _
				; not tuple? set/any 'existing-version :existing-header/version [existing-version: 0.0.0] ;@@ remove?
			]

			existing-version: any [
				existing-header/version
				0.0.0
			]

			existing-hash: existing-header/checksum

			; Compare it to the module we want to load
			;
			case [
				same? module existing [
					; here already

					override?: not any [
						delay
						module? module
					]
				]

				module? existing [
					; premade module

					; pos: _
					; just override, don't replace

					if existing-version >= module-version [
						; it's at least as new, use it instead

						module: existing
						module-header: existing-header
						module-code: _

						module-version: existing-version
						module-hash: existing-hash

						override?: no
					]
				]

				; else is delayed module

				existing-version > module-version [
					; and it's newer, use it instead

					module: _

					set [module-header: module-code:] existing

					module-version: existing-version
					module-hash: existing-hash

					extension: if object? module-code [
						; delayed extension

						module-code
					]

					override?: not delay
					; stays delayed if /delay
				]
			]
		]

		not module? module [
			module: _
			; don't need/want the block reference now
		]

		; Verify /check and /version

		all [
			check
			hash !== module-hash
		][
			cause-error 'access 'invalid-check module
		]

		all [
			version
			needs > module-version
		][
			cause-error 'syntax 'needs reduce [
				any [
					name
					'version
				]

				needs
			]
		]

		all [
			; If no further processing is needed, shortcut return

			not override?
			any [
				module
				delay
			]
		][
			return reduce [
				name module
			]
		]

		; If /delay, save the intermediate form
		;
		delay [
			module: reduce [
				module-header either object? extension [
					extension
				][
					module-code
				]
			]
		]

		; Else not /delay, make the module if needed
		;
		not module [
			; not prebuilt or delayed, make a module

			case/all [
				find module-header/options 'isolate [
					no-share: yes
					; in case of delay
				]

				object? module-code [
					; delayed extension

					set [module-header: module-code:] load-ext-module module-code

					module-header/name: name
					; in case of delayed rename

					if all [
						no-share
						not find module-header/options 'isolate
					][
						module-header/options: append any [
							module-header/options
							make block! 1
						] 'isolate
					]
				]

				binary? module-code [
					module-code: make block! module-code
				]
			]

			assert/type [
				module-header object!
				module-code block!
			]

			module: reduce [
				module-header
				module-code
				do-needs/no-user module-header
			]

			module: catch/quit [
				make module! module
			]
		]

		all [
			not no-lib
			override?
		][
			repend system/modules [
				name module
			]

			case/all [
				all [
					module? module
					not mixin? module-header
					block? select module-header 'exports
				][
					resolve/extend/only lib module module-header/exports
					; no-op if empty
				]
			]
		]
	]

	reduce [
		name

		if module? module [
			module
		]
	]
]

locate-extension: function [
	name [word!]
][
	modules: system/options/modules

	foreach test [
		[modules name %.rebx]
		[modules name #"-" system/build/arch %.rebx]

		; not sure, if keep the folowing ones too.. it simplifies CI testing
		; they should be probably removed, when all used CI tests will be modified
		;
		[modules name #"-" system/build/os #"-" system/build/arch %.rebx]
		[modules name #"-" system/build/sys #"-" system/build/arch %.rebx]
	][
		if exists? file: as file! ajoin test [
			return file
		]

		sys/log/debug 'REBOL [
			"Not found extension file:" file
		]
	]

	_
]

download-extension: function [
	"Downloads extension from a given url and stores it in the modules directory!"

	name [word!]
	source [url!]

	; ; currently the used urls are like: https://github.com/Oldes/Rebol-MiniAudio/releases/download/1.0.0/
	; ; and the file is made according Rebol version, which needs the extension
][
	options: system/options

	file: as file! ajoin either dir? source [
		source: as url! ajoin [
			source name #"-" system/platform #"-" system/build/arch %.rebx
		]

		if system/platform <> 'Windows [
			append source %.gz
		]

		; save the file into the modules directory (using just name+arch)
		[options/modules name #"-" system/build/arch %.rebx]
	][
		[options/modules lowercase second split-path source]
	]

	saved-log-settings: options/log

	try/with [
		if exists? file [
			; we don't want to overwrite any existing files!

			log/info 'REBOL [
				"File already exists:" options/ansi/reset file
			]

			return file
		]

		log/info 'REBOL [
			"Downloading:" options/ansi/reset source
		]

		options/log: #[http: 0 tls: 0]
		; temporary turn off any logs

		content: read source

		if %.gz = suffix? source [
			content: decompress content 'gzip
		]

		log/info 'REBOL [
			"Saving file:" options/ansi/reset file
		]

		write file content
	][
		error: system/state/last-error

		log/error 'REBOL [
			"Failed to download:" options/ansi/reset file ajoin [
				options/ansi/error error/type ": " error/id
			]
		]

		file: _
	]

	options/log: saved-log-settings

	file
]

import: function [
	"Imports a module; locate, load, make, and setup its bindings."

	'module [any-word! file! url! string! binary! module! block!]

	/version

	needs [tuple!]
	"Module must be this version or greater"

	/check

	hash [binary!]
	"Match checksum (must be set in header)"

	/no-share
	"Force module to use its own non-shared global namespace"

	/no-lib
	"Don't export to the runtime library (lib)"

	/no-user
	"Don't export to the user context"

	; See also: sys/make-module*, sys/load-module, sys/do-needs
][
	source: :module
	options: system/options

	if block? source [
		; If it's a needs dialect block, call DO-NEEDS/block:
		; Note: IMPORT block! returns a block of all the modules imported.

		assert [
			not version
			not check
			; these can only apply to one module
		]

		return apply :do-needs [
			source
			no-share
			no-lib
			no-user
			/block
		]
	]

	if any-word? source [
		source: to word! source
	]

	; Try to load and check the module.
	;
	set [name: module:] apply :load-module [
		source
		version
		needs
		check
		hash
		no-share
		no-lib
		/import
	]

	case [
		module _
		; success!

		word? source [
			; Module (as word!) is not loaded already, so let's try to find it.

			file: append to file! source options/default-suffix

			set [name: module:] apply :load-module [
				options/modules/:file
				version
				needs
				check
				hash
				no-share
				no-lib
				/import
				/as
				source
			]

			unless name [
				; try to locate as an extension...

				either file: any [
					locate-extension source

					all [
						url? module: select system/modules source
						download-extension source module
					]
				][
					log/info 'REBOL [
						"Importing extension:" options/ansi/reset file
					]

					set [name: module:] apply :load-module [
						file
						version
						needs
						check
						hash
						no-share
						no-lib
						/import
						/as
						source
					]
				][
					module: _
					; failed
				]
			]
		]

		any [
			file? source
			url? source
		][
			cause-error 'access 'cannot-open reduce [
				source "not found or not valid"
			]
		]
	]

	unless module [
		cause-error 'access 'cannot-open reduce [
			source "module not found"
		]
	]

	; Do any imports to the user context that are necessary.
	; The lib imports were handled earlier by LOAD-MODULE.

	case [
		; Do nothing if /no-user or no exports.
		;
		no-user _

		not block? exports: select header: spec-of module 'exports _

		empty? exports _

		; If it's a private module (mixin), we must add *all* of its exports to user.
		;
		any [
			no-lib

			find select header 'options 'private
			; /no-lib causes private
		][
			resolve/extend/only system/contexts/user module exports
		]

		; Unless /no-lib its exports are in lib already, so just import what we need.
		not no-lib [
			resolve/only system/contexts/user lib exports
		]
	]

	protect 'module/lib-base

	protect/hide 'module/lib-boot

	module
	; module! returned
]

export [
	load import
]

#test [
	test: [
		[
			write %test-emb.reb {123^/[REBOL [title: "embed"] 1 2 3]^/123^/}
			[1 2 3] = xload/header/as %test-emb.reb 'unbound
		]
	][
		; General function:

		[[1 2 3] = xload ["1" "2" "3"]]
		[[] = xload " "]
		[1 = xload "1"]
		[[1] = xload "[1]"]
		[[1 2 3] = xload "1 2 3"]
		[[1 2 3] = xload/as "1 2 3" _]
		[[1 2 3] = xload "rebol [] 1 2 3"]
		[
			d: xload/header "rebol [] 1 2 3"
			all [object? first d [1 2 3] = next d]
		]
		[[Rebol [] 1 2 3] = xload/all "rebol [] 1 2 3"]

		; File variations:
		[equal? read %./ xload %./]
		[
			write %test.txt s: "test of text"
			s = xload %test.txt
		]
		[
			write %test.html "<h1>test</h1>"
			[<h1> "test" </h1>] = xload %test.html
		]
		[
			save %test2.reb 1
			1 = xload %test1.reb
		]
		[
			save %test2.reb [1 2]
			[1 2] = xload %test2.reb
		]
		[
			save/header %test.reb [1 2 3] [title: "Test"]
			[1 2 3] = xload %test.reb
		]
		[
			save/header %test-checksum.reb [1 2 3] [checksum: true]
			; print read/string %test-checksum.reb
			[1 2 3] = xload %test-checksum.reb
		]
		[
			save/header %test-checksum.reb [1 2 3] [checksum: true compress: true]
			; print read/string %test-checksum.reb
			[1 2 3] = xload %test-checksum.reb
		]
		[
			save/header %test-checksum.reb [1 2 3] [checksum: script compress: true]
			; print read/string %test-checksum.reb
			[1 2 3] = xload %test-checksum.reb
		]
		[
			write %test-emb.reb {123^/[REBOL [title: "embed"] 1 2 3]^/123^/}
			[1 2 3] = probe xload/header %test-emb.reb
		]
	]

	foreach t test [
		print either do t [
			'ok
		][
			join "FAILED:" mold t
		]

		print ""
	]

	halt
]
