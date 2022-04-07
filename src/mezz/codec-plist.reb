REBOL [
	name:    plist
	type:    module
	options: [delay]
	version: 0.0.1
	title:   "REBOL 3 codec for PLIST files"
	file:    https://raw.githubusercontent.com/Oldes/Rebol3/master/src/mezz/codec-plist.reb
	author:  "Oldes"
	history: [
		07-Apr-2022 "Oldes" {Initial version of the PLIST decoder}
	]
	References: [
		https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/PropertyLists/Introduction/Introduction.html
		https://medium.com/@karaiskc/understanding-apples-binary-property-list-format-281e6da00dbd
	]
	todo: {
		* Support binary PLIST version
		* PLIST encoder
	}
]

system/options/log/plist: 1

stack: copy []
key: value: none

~spnl: system/catalog/bitsets/whitespace

~dict: [
	any ~spnl
	<dict>
	(	
		append append stack :key make map! 8
	)
	any [
		~key
		~value
		(
			put last stack :key :value
		)
		| any ~spnl
	]
	any ~comment
	</dict>
	(
		value: take/last stack
		key:   take/last stack
	)
]
~key:     [<key>     any ~spnl copy key:   to </key>     thru #">" (try [key: to word! key])]
~string:  [<string>  any ~spnl copy value: to </string>  thru #">" ]
~data:    [<data>    any ~spnl copy value: to </data>    thru #">" (value: debase value 64)]
~date:    [<date>    any ~spnl copy value: to </date>    thru #">" (value: to-date value)]
~integer: [<integer> any ~spnl copy value: to </integer> thru #">" (value: to integer! value)]
~real:    [<real>    any ~spnl copy value: to </real>    thru #">" (value: to decimal! value)]
~true:    [<true/>  (value: true )]
~false:   [<false/> (value: false)]
~array:   [
	<array>
	(append/only stack copy [])
	any [~value (append/only last stack :value) any ~spnl]
	</array>
	(value: take/last stack)
]
~comment: [any ~spnl opt ["<!--" thru "-->"]]

~value: [
	any ~comment [
		  ~string
		| ~true
		| ~false
		| ~array
		| ~dict
		| ~date
		| ~data
		| ~integer
		| ~real
	]
]

register-codec [
	name:  'plist
	type:  'text
	title: "Property List File Format"
	suffixes: [%.plist]

	decode: function [
		{Extract content of the PLIST file}
		data  [binary! file! url!]
		;return: [map!]
	] [
		verbose: system/options/log/plist
		unless binary? data [ data: read data ]
		if verbose > 0 [
			sys/log/info 'PLIST ["^[[1;32mDecode PLIST data^[[m (^[[1m" length? data "^[[mbytes )"]
		]
		unless parse to string! data [
			thru "<plist " thru #">"
			~dict
			any ~comment
			</plist>
			to end
		][	return none ]

		if verbose: system/options/log/plist > 0 [
			foreach [k v] value [
				switch to word! k [
					DeveloperCertificates [
						v: copy v
						forall v [
							try [
								crt: codecs/crt/decode v/1
								change/only v compose [
									commonName:  (crt/subject/commonName)
									valid-to:    (crt/valid-to)
									fingerprint: (crt/fingerprint)
								]
							]
						]
					]
					DER-Encoded-Profile [
						sys/log/more 'PLIST ajoin [as-green k ": " mold v]
						continue
					]
				]
				sys/log 'PLIST ajoin [as-green k ": " mold v]
			]		
		]

		value
	]

	;encode: function [data [binary!]][	]

	identify: function [data [binary!]][
		; just a simple test if there are key parts...
		parse data [
			thru "<!DOCTYPE plist"
			thru "<plist " to end
		]
	]
]
