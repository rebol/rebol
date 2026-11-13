Rebol [
	Name:    pop3
	Type:    module
	Options: [delay]
	Version: 0.1.1
	Date:    20-Jul-2022
	File:    %prot-pop3.reb
	Title:   "POP3 protocol scheme"
	Author:  @Oldes
	Rights:  "Copyright (C) 2022 Oldes. All rights reserved."
	License: MIT
	History: [
		0.1.0 16-Jul-2022 @Oldes "Initial version"
		0.1.1 20-Jul-2022 @Oldes "Bug fixing"
	]
	Notes: {
		1. So far it is using  mix of sync and async behaviours.
           `write` action are asynchronous
           `read` action (after `write` is synchronous)
        2. The protocol so far works only in plain unsecured POP3 or,
           with POP3 submission (encryption started using STLS command).
           There is some issue when using POP3s (over TLS directly)!
	}
	Usage: [
		mbox: open user's pop3
		;; wait connection is established and user authenticated
		read mbox ;= returns _Logged in_ message on success
		;; now the port should be ready to accept commands

		;- request number of messages in the mailbox        
		;; displays the number of messages currently in the
		;; mailbox and the size in bytes
		print length? mbox

		;- request a summary of messages (sizes per message)
		print read write mbox 'LIST

		;- Request first message                            
		data: pick mbox 1

		;- Retrieving part of the message with TOP          
		data: read write mbox [TOP 1 2] ;= full header and first 2 lines

		;- Deleting a message                               
		remove/key mbox 1
		; or
		read write mbox [DELE 2]

		;- Resetting the session with RSET
		;; This will undelete all deleted messages in this session.
		read write mbox 'RSET

		;- Closing connection
		close mbox
	]
]


system/options/log/pop3: 1


net-log: func[data /C /S /E /local msg][
	msg: clear ""
	case [
		C [append msg "Client: "]
		S [append msg "Server: "]
		E [log-error 'POP3 :data return :data]
	]
	append msg data
	log-debug 'POP3 trim/tail msg
	data
]

do-cmd: func[ctx cmd /with arg /local pop3][
	ctx/command: cmd
	ctx/argument: arg
	pop3: ctx/connection/parent
	clear pop3/data
	try/with [
		write ctx/connection to binary! net-log/C ajoin either/only arg [cmd SP arg CRLF][cmd CRLF]
		if pop3/state <> 'INIT [ pop3/state: 'WRITING ]
	][
		ctx/error: system/state/last-error
	]
]
do-cmd-hide: func[ctx cmd arg msg /local pop3][
	ctx/command: cmd
	ctx/argument: arg
	pop3: ctx/connection/parent
	clear pop3/data
	try/with [
		net-log/C ajoin [cmd SP msg]
		write ctx/connection to binary! ajoin either/only arg [cmd SP arg CRLF][cmd CRLF]
		if pop3/state <> 'INIT [ pop3/state: 'WRITING ]
	][
		ctx/error: system/state/last-error
	]
]

pop3-conn-awake: function [event][
	; conn is the real port ie. port/state/connection
	conn:  event/port
	pop3:  conn/parent
	ctx:   pop3/extra
	log-trace 'POP3 ["Command:" ctx/command "state:" pop3/state "event:" event/type "ref:" event/port/spec/ref]

	info?: system/options/log/pop3 > 0

	wake?: switch event/type [
		error [
			log-error 'POP3 "Network error"
			close conn
			return true
		]
		lookup [
			open conn
			false
		]
		connect [
			either ctx/command = 'STLS [
				log-debug 'POP3 "TLS connection established..."
				do-cmd/with ctx 'AUTH 'PLAIN
				return false
			][
				log-debug 'POP3 "Reading server's invitation..."
				ctx/command: 'CONNECT
				read conn
			]
			false
		]

		read [
			response: to string! conn/data
			clear conn/data
			if empty? response [return false]

			if system/options/log/pop3 > 1 [
				foreach line split-lines response [
					log-debug 'POP3 ["Server:^[[32m" line]
				]
			]

			ok-msg: err: none

			if empty? pop3/data [
				parse response [
					#"+" opt "OK" any SP
						 opt [copy ok-msg: to CRLF CRLF]
						response: to end
					|
					"-ERR " opt [set err: to CRLF CRLF] to end
				]
			]

			if err [
				pop3/state: 'ERROR
				 ctx/error: err
				insert system/ports/system make event! [type: 'error port: pop3]
				return false
			]

			;? response

			switch/default pop3/state [
				READ
				READING [
					append pop3/data response
					
					if info? [
						switch ctx/command [
							RETR [
								if pop3/state <> 'READING [
									stat: load ok-msg
									log-info 'POP3 ["Receiving message" ctx/argument "having" stat/1 "bytes."]
								]
							]
							TOP [
								if pop3/state <> 'READING [
									log-info 'POP3 ["Receiving headers of message" first ctx/argument]
								]
							]
							STAT [
								stat: load ok-msg
								log-info 'POP3 ["Mailbox has" stat/1 "messages having" stat/2 "bytes."]
							]
							DELE [
								log-info 'POP3 ["Message" ctx/argument response]
							]
							RSET [
								log-info 'POP3 ["Session reset." response]
							]
							QUIT [
								log-info 'POP3 ["Server quits:^[[32m" response]
							]
						]
					]

					either all [
						find [RETR LIST UIDL CAPA TOP] ctx/command
						none? end: find/match skip tail pop3/data -5 "^M^/.^M^/"
					][
						pop3/state: 'READING
						log-debug 'POP3 "Data are not complete yet..."
						read conn
						false
					][
						if end [clear end]
						pop3/state: 'READY
						true
					]
				]
				INIT [
					switch/default ctx/command [
						CONNECT [
							log-info 'POP3 ["Connected to server:^[[32m" response]
							log-debug 'POP3 "Requesting server's capabilities..."
							do-cmd ctx 'CAPA
							false
						]
						CAPA [
							ctx/capabilities: split-lines copy response
							take/last ctx/capabilities ;= removes the trailing dot
							log-info 'POP3 ["Server accepts: ^[[32m" mold/flat ctx/capabilities]
							case [
								all [ctx/start-tls? find response "STLS"] [
									do-cmd ctx 'STLS
								]
								find response "PLAIN" [
									do-cmd/with ctx 'AUTH 'PLAIN
								]
								find response "USER" [
									;; not using do-cmd, because it could show credential in log
									do-cmd-hide ctx 'USER pop3/spec/user "***name***"
								]
								'else [
									ctx/error: "No known authentication method!"
								]
							]
							false
						]
						STLS [
							log-debug 'POP3 "Upgrading client's connection to TLS port"
							;; tls-port will be a new layer between existing pop3 and client (tcp) connections
							tls-port: open compose [scheme: 'tls conn: (conn)]
							tls-port/parent: pop3
							conn/parent: tls-port
							ctx/connection: conn/extra/tls-port
							false
						]
						AUTH [
							;= only with argument PLAIN so far...
							net-log/C "***credentials***"
							write ctx/connection to binary! ajoin [
								enbase ajoin ["^@" pop3/spec/user "^@" pop3/spec/pass] 64
								CRLF
							]
							pop3/state: 'WRITING ;= exits from INIT state
							false
						]
						USER [
							;; not using do-cmd, because it could show credential in log
							do-cmd-hide ctx 'PASS pop3/spec/pass "***pass***"
							pop3/state: 'WRITING ;= exits from INIT state
							false
						]
					][
						ctx/error: ajoin ["Unknown init command: " ctx/command]
					]
				]
			][
				ctx/error: ajoin ["Unknown state: " pop3/state]
			]
		]
		wrote [
			if pop3/state == 'WRITING [pop3/state: 'READ]
			read conn
			false
		]
		close [
			ctx/error: "Port closed on me"
		]
	]
	if ctx/error [
		;; force wake-up to report error in all cases.
		wake?: true
		pop3/state: 'ERROR
	]
	if wake? [
		insert system/ports/system make event! [type: pop3/state port: pop3]
	]
	wake?
]

assert-ready: func[port][
	unless open? port       [ cause-error 'Access 'not-open  port/spec/ref ]
	if port/state <> 'READY [ cause-error 'Access 'not-ready port/spec/ref ]
	true
]

sys/make-scheme [
	name: 'pop3
	title: "Post Office Protocol (v3)"
	spec: make system/standard/port-spec-net [
		port:    110
		timeout: 10
	]
	awake: func[event /local port type][
		port: event/port
		type: event/type
		log-trace 'POP3 ["POP3-Awake event:" type]
		switch/default type [
			error [
				port/state: 'ERROR
				;@@ could be reported into upper level, if possible
				;@@ instead of just failing!
				all [
					port/extra
					port/extra/error
					do make error! port/extra/error
				]
				true
			]
			close [
				port/extra:
				port/state: none
				true
			]
			ready [
				true
			]
		][
			pop3-conn-awake :event
		]
	]
	actor: [
		open: func [
			port [port!]
			/local spec conn
		][
			if port/extra [return port]

			;@@ TODO: spec validation!

			port/extra: construct [
				connection:
				commands:
				command:
				argument:
				error:
				capabilities: none
				start-tls?: false ;= no TLS upgrade
			]
			port/data: copy ""

			spec: port/spec

			conn: context [
				scheme: none
				host:   spec/host
				port:   spec/port
				ref:    none
			]
			conn/scheme: either 995 == spec/port ['tls]['tcp]
			;; `ref` is used in logging and errors
            conn/ref: as url! ajoin [conn/scheme "://" spec/host #":" spec/port]
            spec/ref: as url! ajoin ["pop3://" spec/user #"@" spec/host #":" spec/port]

            port/state: 'INIT
            port/extra/connection: conn: make port! conn

            conn/parent: port
            conn/awake: :pop3-conn-awake

            open conn ;-- open the actual tcp (or tls) port
			
			; return the newly created port
			port
		]
		open?: func [
			port [port!] /local conn
		][
			to logic! all [
				port/state
				port/extra
				port? conn: port/extra/connection
				open? conn
			]
		]
		close: func [
			port [port!]
		][
			if open? port [
				;@@ what if we are reading or writing!
				do-cmd port/extra 'QUIT
				wait [port port/spec/timeout]
				try [close port/extra/connection]
				port/extra: none
				port/state: none
			]
			port
		]
		read: func [
			port [port!]
		][
			if all [
				;; allow `read` when we already initialized connection...
				port/state <> 'INIT
				not open? port
			][ cause-error 'Access 'not-open port/spec/ref ]

			either all [
				find [WRITING INIT] port/state
				port? wait [port port/spec/timeout]
				port/data
			][
				copy port/data
			][	none ]
		]
		write: func [port [port!] data /local cmd arg][
			assert-ready port
			case [
				word? :data [ do-cmd port/extra data ]
				all [
					block? :data
					;; only one command with args
					parse data [
						;; using also some command aliases
						  ['GET | 'RETR | 'RETRIEVE] set arg integer! (cmd: 'RETR)
						| ['DEL | 'DELE | 'DELETE  ] set arg integer! (cmd: 'DELE)
						|  'TOP copy arg 1 2 integer! (
							cmd: 'TOP
							;; add optional zero length, if not set
							all [1 = length? arg  append arg 0]
						)
						;; and aliases for commands without arguments...
						| 'RESET (cmd: 'RST)
					]
				][
					do-cmd/with port/extra :cmd :arg
				]
				'else [
					 cause-error 'Access 'invalid-port-arg :data
				]
			]
			port
		]
		pick: func[port [port!] index [integer!]][
			assert-ready port
			do-cmd/with port/extra 'RETR index
			;; keep waiting if we are still reading and timeout happens
			while [integer? wait [port port/spec/timeout]][
				if port/state <> 'READING [break]
			]
			copy port/data
		]
		remove: func[port [port!] /key index [integer!]][
			unless index [
				while [not integer? try [index: to integer! ask "Enter mail index: "]][
					log-error 'POP3 "Index must be an integer!"
				]
			]
			assert-ready port
			do-cmd/with port/extra 'DELE index
			all [
				wait [port port/spec/timeout]
				port/data
				copy port/data
			]
		]
		length?: func[port [port!]][
			assert-ready port
			do-cmd port/extra 'STAT
			all [
				wait [port port/spec/timeout]
				port/data
				load port/data
			]
		]
	]
]

sys/make-scheme/with [
	name: 'pop3s
	title: "Post Office Protocol (v3) Secure"
	spec: make spec [
		port: 995
	]
] 'pop3