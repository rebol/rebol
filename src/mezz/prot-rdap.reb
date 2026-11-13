Rebol [
	title: "RDAP Scheme"
	SPDX-License-Identifier: MIT
	name:  rdap
	type:  module	
	purpose: {Registration Data Access Protocol}
	author: "Oldes"
	version: 0.0.2
	file: %rdap.reb
	date: 25-Mar-2025
	home: https://github.com/Oldes/Rebol-RDAP
	usage: [
		;- version 1:
		result: read rdap:google.com
		;- version 2:
		result: write rdap:// 109.81.82.250
	]
	note: "Rebol versions lower than 3.18.5 have a TLS bug that may cause requests to fail!"
]


system/options/log/rdap: 1

result:     make map! []
rdap-cache: make map! [] ;; used to store already downloaded results
rdap-que:   make block! 4
rdap-content-type: "application/rdap+json"

read-rdap: function/with [url [url!] /quiet ][
	all [
		any [
			;; use data from the cache...
			data: select rdap-cache url
			;; or read it...
			all [
				res: read/all url
				res/1 == 200
				parse res/2/content-type [rdap-content-type opt #";" to end]
				data: decode 'json res/3
				rdap-cache/:url: data
			]
		]
		put result url data
	]
	unless map? data [return none]
	collect-links data/links
	if block? data/entities [
		foreach ent data/entities [
			collect-links ent/links
		]
	]
	if all [
		not quiet
		system/options/log/rdap > 0
	][
		print form-rdap data
	]
	data
][
	collect-links: function[links][
		unless block? links [exit]
		foreach link links [
			if all [
				link/type == rdap-content-type
				link/type != "self"
				none? find result link: to url! link/href
				none? find rdap-que link
			][
				append rdap-que link
			]
		]
	]
]

form-rdap: function/with [data [map!]][
	clear out
	clear entity-handles
	print-hline
	foreach [key name] [
		rdapConformance "Conformance: "
		name            "Name       : "
		type            "Type       : "
		ldhName         "ldhName    : "
		handle          "Handle     : "
		status          "Status     : "		
		country         "Country    : "
	][
		if value: data/:key [
			if block? :value [value: ajoin/with value ", "]
			emit [name as-yellow value LF]
		]
	]
	if data/vcardArray [
		if data/objectClassName == "entity" [emit "Entity:^/"]
		emit-vcards data/vcardArray
	]
	if block? blk: data/nameservers [
		emit "^/Nameservers:^/"
		foreach ns blk [
			emit [" * " ns/ldhName SP]
			if ips: ns/ipAddresses [
				if ips/v4 [emit [ips/v4 SP]]
				if ips/v6 [emit [ips/v6 SP]]
			]
			emit LF
		]
	]
	if block? blk: data/events [
		emit "^/Events:^/"
		foreach ev blk [
			emit [indent " * " pad ev/eventAction 12 ": " ev/eventDate LF]
		]
	]
	if block? blk: data/entities [
		emit "^/Contacts:^/"
		foreach en blk [emit-entity en]
	]
	out
][
	out:    copy ""
	indent: copy ""
	entity-handles: copy []
	++indent: does [append indent "   "]
	--indent: does [head clear skip tail indent -3]
	emit: func[data][
		if block? data [data: ajoin data]
		append out data
	]
	emit-entity: function [en][
		if find entity-handles handle: en/handle [exit]
		if en/roles  [emit [indent " * Role        : " en/roles LF]]
		if handle    [emit [indent "   Handle      : "   handle LF] append entity-handles handle]
		if block? en/links [
			emit [indent "   Links       : " en/links/1/value LF ]
		]
		emit-vcards en/vcardArray
		if block? en/remarks [
			foreach re en/remarks [
				emit [indent "   Remark      : " re/title LF]
				if re/description [
					foreach line to block! re/description [
						emit [indent "               : " trim/head/tail line LF]
					]
				]
			]
		]
		if block? en/entities [
			emit LF
			foreach en en/entities [ emit-entity en ]
		]
		emit LF
	]
	emit-vcards: func[vcardArray /local card][
		if block? vcardArray [
			parse vcardArray [any ["vcard" set card: block! (emit emit-vcard card)]]
		]
	]
	emit-vcard: func[card /local value][
		++indent
		foreach field card [
			if field/1 == "version" [continue]
			value: ajoin [
				form-vcard-value field/2/label
				form-vcard-value field/4
			]
			if empty? value [continue] 
			emit [indent pad copy field/1 12 ": " value LF]
		]
		--indent
	]
	form-vcard-value: func[value][
		case [
			string? value [
				replace/all deline value LF "; "
			]
			block? value [
				value: copy value
				while [not tail? value][
					either all [series? value/1 empty? value/1][
						remove value
					][	value: next value ]
				]
				value: deline ajoin/with head value "; " 
			]
		]
		value
	]
]

sys/make-scheme [
	name: 'rdap
	title: "RDAP Protocol"
	actor: [
		;@@ Only sync operations implemented
		open:  func [port [port!]][ port ]
		open?: func [port [port!]][ true ]
		close: func [port [port!]][ port ]

		;-- Write handler performs RDAP query based on target domain or IP
		write: function [
			port   [port!] "RDAP port"
			target [any-string! tuple!] "Domain or IP"
		][
			;; Turn off HTTP traces...
			http-verbosity: system/options/log/http
			system/options/log/http: 0

			clear rdap-que
			clear result ;; Function returns map with JSON result of all related urls

			try [target: to tuple! target: as string! target]

			sys/log/info 'RDAP ["Query:^[[22m" target]

			either any-string? target [
				;- Step 1: Extract TLD from domain and query IANA RDAP server       
				try/with [
					all [
						tld: find/last/tail target #"."
						url: join https://rdap.iana.org/domain/ tld
						result/:url: res: read-rdap/quiet url
					]
				][
					sys/log/error 'RDAP system/state/last-error
					sys/log/error 'RDAP ["Failed to resolve TLD Registrar's info:" as-red tld]
					return none
				]
				;- Step 2: Update collected links with a domain query               
				foreach url rdap-que [
					if slash != last url [append url slash]
					append url ajoin [%domain/ target]
				]
				;- Step 3: Query detailed domain info from all related RDAP links   
				forall rdap-que [
					sys/log/info 'RDAP ["Resolving info from:" as-green rdap-que/1]
					try/with [
						res: read-rdap rdap-que/1
					][
						sys/log/error 'RDAP system/state/last-error
					]
				]
			][
				try/with [
					result/:target: res: read-rdap join https://rdap.arin.net/registry/ip/ target
				][
					sys/log/error 'RDAP system/state/last-error
				]
			]
			;; Restore original http verbosity level
			system/options/log/http: http-verbosity
			body-of result ;; return just a block instead of the map
		]
		
		;-- Read handler delegates to write handler using host from port spec
		read: function [
			port [port!]
		][
			port/actor/write port any [
				select port/spec 'host    ;; rdap://google.com
				select port/spec 'target  ;; rdap:google.com
			]
		]
	]
]
