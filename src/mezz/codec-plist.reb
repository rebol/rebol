REBOL [
	title:   "Codec: PLIST"
	name:    plist
	type:    module
	options: [delay]
	version: 1.0.0
	file:    https://raw.githubusercontent.com/Oldes/Rebol3/master/src/mezz/codec-plist.reb
	author:  "Oldes"
	history: [
		07-Apr-2022 "Oldes" {Initial version of the PLIST and Provisioning Profile decoder}
	]
	references: [
		https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/PropertyLists/Introduction/Introduction.html
		https://medium.com/@karaiskc/understanding-apples-binary-property-list-format-281e6da00dbd
	]
	todo: {
		* Support binary PLIST version
		* PLIST encoder
		* Provision profile data validation?
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
			log-info 'PLIST ["^[[1;32mDecode PLIST data^[[m (^[[1m" length? data "^[[mbytes )"]
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
									fingerprint: (select crt 'fingerprint)
								]
							]
						]
					]
					DER-Encoded-Profile [
						log-debug 'PLIST ajoin [as-green k ": " mold v]
						continue
					]
				]
				log-warn 'PLIST ajoin [as-green k ": " mold v]
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

register-codec [
	name:  'provision
	type:  'cryptography
	title: "Apple's Provisioning Profile File Format"
	suffixes: [%.provisionprofile %.mobileprovision]

	decode: function [
		{Extract PLIST data from a provision profile}
		data  [binary! file! url!]
		;return: [map!]
	] [
		unless binary? data [ data: read data ]

		der: codecs/der/decode data
		parse der [
			'SEQUENCE into [
				'OBJECT_IDENTIFIER #{2A864886F70D010702} 'CS0 into [
					'SEQUENCE into [
						'INTEGER set version: binary!
						'SET into [
							'SEQUENCE into [
								'OBJECT_IDENTIFIER set oid: binary! (
									hash-alg: codecs/der/decode-oid oid
								)
								to end
							]
						]
						'SEQUENCE into [
							'OBJECT_IDENTIFIER #{2A864886F70D010701} 'CS0 into [
								'OCTET_STRING set plist: binary!
							]
						]
						; follows certificates used to sign the data..
						; validation is not implemented!
						to end
					]
					to end
				]
				to end
			]
		]
		either binary? plist [
			codecs/plist/decode plist
		][	none ]
	]
]
