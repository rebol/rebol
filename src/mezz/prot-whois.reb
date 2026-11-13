Rebol [
	title: "WHOIS Protocol"
	author: "Oldes"
	version: 0.0.1
	file: %prot-whois.r
	date: 30-Apr-2019
	purpose: {
		Low level WHOIS protocol implementation, which may be used to write
		more advanced WHOIS command}
	links: [
		https://www.iana.org/domains/root/db
		https://github.com/rfc1036/whois
		https://simonecarletti.com/blog/2012/03/whois-protocol/
	]
	usage: [
		;- version 1:
		read whois://seznam.cz ;<- will do recursive lookup
		;- version 2:
		write whois://whois.nic.cz "seznam.cz"
	]
]

append system/options/log [whois: 1]
sys/make-scheme [
	name: 'whois
	title: "WHOIS Protocol"
	spec: make system/standard/port-spec-net [port: 43 timeout: 15 ]
	awake: func [event /local port parent] [
		log-trace 'WHOIS ["Awake:^[[22m" event/type]
		port: event/port
		parent: port/extra
		switch event/type [
			lookup  [ open port ]
			connect [
				parent/state/state: 'ready
				either parent/data [
					write port parent/data
				][	return true ]
			]
			read  [ read port ]
			wrote [ parent/state/state: 'reading read port ]
			close [
				parent/state/state: none
				parent/data: none
				close port
				return true
			]
			error [
				parent/state/state: make error! [
					type: 'Access
					id: 'Protocol
					arg1: to integer! event/offset/x ;@@ needs revire once EVENT will be fixed
				]
			]
		]
		false
	]
	actor: [
		open: func [
			port [port!]
			/local conn spec
		] [
			if all [
				port/state
				conn: port/state/connection
			][
				case [
					none? port/state/state [ open conn ]
					all [port/state/state = 'ready port/data][
						 write conn port/data
					]
				]
				return port 
			]
			spec: port/spec
			if none? spec/host [ spec/host: "whois.iana.org" ]

			port/state: object [
				state: 'inited
				connection:
				awake: :port/awake
				close?: false
			]
			port/state/connection: conn: make port! [
				scheme: 'tcp
				host: spec/host
				port: spec/port
				ref: rejoin [tcp:// host #":" port]
			]
			conn/extra: port
			conn/awake: :awake
			open conn
			port
		]

		open?: func [port [port!]][
			not none? all [
				port/state
				open? port/state/connection
			]
		]

		close: func [ port [port!]] [
			if open? port [
				close port/state/connection
				port/state: none
			]
		]

		write: func [
			port   [port!]
			target [string! tuple!]
			/local conn	
		][
			port/spec/path: target
			if tuple? target [
				target: join 
					any [
						select [
							"whois.ripe.net" "-V Md5.2 "
							"whois.iana.org" ""
						] port/spec/host
						"n + "
					]
					target
			]
			log-info 'WHOIS ["Query:^[[22m" target "^[[1mfrom:^[[22m" port/spec/host]
			port/data: join target CRLF
			open port
			conn: port/state/connection
			either port? wait [conn port/spec/timeout][
				port/actor/on-result port
			][
				do make error! [type: 'Access id: 'Timeout arg1: port/spec/ref]
			]
		]
		
		read: func [
			port [port!]
			/local target 
		][
			target: port/spec/host
			try [target: to tuple! target]
			port/spec/host: "whois.iana.org"
			port/actor/write port target
		]

		on-result: func[
			port [port!]
			/local result refer
		][
			result: port/state/connection/data
			if result [
				try [ result: to string! result ]
				log-info 'WHOIS ajoin ["Result:^/^[[22m" result]
				if all [
					parse result [
						[
							thru "^/whois: " |
							thru "^/   Registrar WHOIS Server: "
						] copy refer to LF to end
					]
					any [
						tuple? port/spec/path    ;- follow if request is IP...
						find port/spec/path #"." ;- ... or TLD
					]
					port/spec/host <> refer: trim/all refer
				][
					close port/state/connection
					port/state: none
					port/spec/host: refer
					return port/actor/write port port/spec/path
				]
			]
			result
		]
	]
]
