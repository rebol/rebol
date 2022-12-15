Rebol [
	title:  "SMTP protocol scheme"
	name:    smtp
	type:    module
	author:  ["Graham" "Oldes"]
	rights:  BSD
	version: 1.1.0
	date:    14-Jul-2022
	file:    %prot-smtp.reb
	notes: {
		0.0.1 original tested in 2010
		0.0.2 updated for the open source versions
		0.0.3 Changed to use a synchronous mode rather than async.  Authentication not yet supported
		0.0.4 Added LOGIN, PLAIN and CRAM-MD5 authentication.  Tested against CommunigatePro
		0.0.5 Changed to move credentials to the url or port specification
		0.0.6 Fixed some bugs in transferring email greater than the buffer size.
		1.0.0 Oldes: Updated to work with my Rebol3 fork; including TLS.
		1.0.1 Oldes: Using extenal IP in the EHLO message, when domain-name is not available
		1.1.0 Oldes: Recipient's address validation and possibility to have more than one

		Note that if your password does not work for gmail then you need to 
		generate an app password.  See https://support.google.com/accounts/answer/185833
		
		synchronous mode
		write smtp://user:password@smtp.clear.net.nz [ 
			from:
			to: 
			message: 
		]
		
		eg: write smtp://user:password@smtp.yourisp.com compose [
			from: me@somewhere.com
			to: recipient@other.com
			message: (message)
		]

		message: rejoin [ {To: } recipient@other.com {
From: } "R3 User" { <} me@somewhere.com {>
Date: Mon, 21 Jan 2013 17:45:07 +1300
Subject: testing from r3
X-REBOL: REBOL3 Alpha

where's my kibble?}]
		
		write [ 
			scheme: 'smtp 
			host: "smtp.yourisp.com"
			user: "joe"
			pass: "password"
			ehlo: "local.domain.name" ; optional, if not available, external IP will be used
		] compose [
			from: me@somewhere.com
			to:   recipient@other.com
			message: (message)
		]
		
		Where message is an email with all the appropriate headers.
		In Rebol2, this was constructed by the 'send function
		
		If you need to use smtp asynchronously, you supply your own awake handler
		
		p: open smtp://smtp.provider.com
		p/state/connection/awake: :my-async-handler
	}
]

system/options/log/smtp: 2

bufsize: 16384 ;-- use a write buffer of 16KiB (maximum TLS record size!) for sending large attachments

mail-obj: make object! [ 
	from: 
	to:
	message: none
]


throw-smtp-error: func [
	smtp-port  [port!]
	error [error! string! block!]
][
	sys/log/error 'SMTP error
	unless error? error [
		error: make error! [
			type: 'Access
			id:   'Protocol
			arg1: either block? error [ajoin error][error]
		]
	]
	either object? smtp-port/extra [
		smtp-port/extra/error: error
		smtp-port/awake make event! [type: 'error port: smtp-port]
	][  do error ]
]

; auth-methods: copy []
alpha: system/catalog/bitsets/alpha
digit: system/catalog/bitsets/numeric

net-log: func[data /C /S /E /local msg][
	msg: clear ""
	case [
		C [append msg "Client: "]
		S [append msg "Server: "]
		E [sys/log/error 'SMTP :data return :data]
	]
	append msg data
	sys/log/more 'SMTP trim/tail msg
	data
]

sync-smtp-handler: function [event][
	sys/log/debug 'SMTP ["sync-smtp-handler event:" event/type event/port/spec/ref]
	; client is the real port ie. port/state/connection
	client:    event/port
	smtp-port: client/parent
	spec:      smtp-port/spec
	state:     smtp-port/state
	smtp-ctx:  smtp-port/extra
	sys/log/debug 'SMTP ["State:" state]

	switch event/type [
		error [
			sys/log/error 'SMTP "Network error"
			close client
			return true
		]
		lookup [
			open client
			false
		]
		connect [
			smtp-port/state: 'EHLO
			either state = 'STARTTLS [
				sys/log/more 'SMTP "TLS connection established..."
				write smtp-ctx/connection to binary! net-log/C ajoin ["EHLO " spec/ehlo CRLF]
				smtp-port/state: 'AUTH
			][
				read client
			]
			false
		]

		read [
			response: to string! client/data
			clear client/data
			if empty? response [return false]
			
			code: none
			parse response [copy code: 3 digit to end (code: to integer! code)]

			if system/options/log/smtp > 1 [
				foreach line split-lines trim/tail response [
					sys/log/more 'SMTP ["Server:^[[32m" line]
				]
			]

			switch/default state [
				EHLO
				INIT [
					write client to binary! net-log/C ajoin ["EHLO " spec/ehlo CRLF]
					smtp-port/state: 'AUTH
					return false
				]
				AUTH [
					if code = 250 [
						if parse response [
							thru "STARTTLS" CRLF to end (
								;throw-smtp-error "STARTTLS not implemented!"
								smtp-port/state: 'STARTTLS
								write client to binary! net-log/C "STARTTLS^M^/"
								return false
							)
							|
							thru "AUTH" [SP | #"="] copy auth-methods: to CRLF to end (
								auth-methods: split auth-methods SP
								foreach auth auth-methods [
									try [auth: to word! auth]
									switch auth [
										CRAM-MD5 [
											smtp-port/state: 'CRAM-MD5
											write client to binary! net-log/C "AUTH CRAM-MD5^M^/"
											return false
										]
										LOGIN [
											smtp-port/state: 'LOGIN
											write client to binary! net-log/C "AUTH LOGIN^M^/"
											return false
										]
										PLAIN [
											smtp-port/state: 'PLAIN
											net-log/C "AUTH PLAIN ***"
											write client to binary! ajoin [
												"AUTH PLAIN "
												enbase/flat ajoin [spec/user #"^@" spec/user #"^@" spec/pass] 64
												CRLF
											]
											return false
										]
										'else [
											sys/log/debug 'SMTP ["Unknown authentication method:" auth]
										]
									]
								]
							)
						]
						sys/log/debug 'SMTP ["Trying to send without authentication!"]
						smtp-port/state: 'FROM
						write client to binary! net-log/C ajoin ["MAIL FROM: <" smtp-ctx/mail/from ">" CRLF]
						return false
					]
				]

				STARTTLS [
					if code = 220 [
						sys/log/more 'SMTP "Upgrading client's connection to TLS port"
						;; tls-port will be a new layer between existing smtp and client (tcp) connections
						tls-port: open [scheme: 'tls conn: client]
						tls-port/parent: smtp-port
						client/parent: tls-port
						smtp-ctx/connection: client/extra/tls-port
						return false
					]
				]

				LOGIN [
					case [
						find/part response "334 VXNlcm5hbWU6" 16 [ ;enbased "Username:"
							; username being requested
							sys/log/more 'SMTP "Client: ***user-name***"
							write client to binary! ajoin [enbase/flat spec/user 64 CRLF]
						]
						find/part response "334 UGFzc3dvcmQ6" 16 [ ;enbased "Password:"
							; pass being requested
							sys/log/more 'SMTP "Client: ***user-pass***"
							write client to binary! ajoin [enbase/flat spec/pass 64 CRLF]
							smtp-port/state: 'PASSWORD
						]
						true [
							throw-smtp-error smtp-port join "Unknown response in AUTH LOGIN " response						
						]
					]
				]
				CRAM-MD5 [
					either code = 334 [
						auth-key: skip response 4
						auth-key: debase auth-key 64
						; compute challenge response
						auth-key: checksum/with auth-key 'md5 spec/pass
						sys/log/more 'SMTP "Client: ***auth-key***"
						write client to binary! ajoin [enbase/flat ajoin [spec/user SP lowercase enbase auth-key 16] 64 CRLF]
						smtp-port/state: 'PASSWORD
						false
					][
						throw-smtp-error smtp-port join "Unknown response in AUTH CRAM-MD5 " response						
					]
				]
				PLAIN
				PASSWORD [
					either code = 235 [
						write client to binary! net-log/C ajoin ["MAIL FROM: " mold as tag! smtp-ctx/mail/from CRLF]
						smtp-port/state: 'FROM
						smtp-ctx/recipients: 0
						false
					][
						throw-smtp-error smtp-port "Failed authentication"
					]
				]
				FROM
				RCPT [
					if code <> 250 [
						either state == 'FROM [
							throw-smtp-error smtp-port "FROM address rejected by server"
							return true ; awake.. no more job to do.
						][
							sys/log/error 'SMTP ["Server rejects TO address:" as-red smtp-ctx/rcpt]
							smtp-ctx/rcpt: none
							smtp-ctx/recipients: smtp-ctx/recipients - 1
						]
					]
					either empty? smtp-ctx/mail/to [
						;; no more recipients, check if at least one was accepted...
						;sys/log/debug 'SMTP ["Number of accepted recipients:" smtp-ctx/recipients]
						if smtp-ctx/recipients == 0 [
							throw-smtp-error smtp-port "There were no accepted recipients!"
							return true
						]
						;; if so, request the DATA start...
						write client to binary! net-log/C join "DATA" CRLF
						smtp-port/state: 'DATA
					][
						;; register another recipient...
						smtp-ctx/rcpt: take smtp-ctx/mail/to
						smtp-ctx/recipients: smtp-ctx/recipients + 1
						write client to binary! net-log/C ajoin ["RCPT TO: " mold as tag! smtp-ctx/rcpt crlf]
						smtp-port/state: 'RCPT
					]
					false
				]
				DATA [
					either code = 354 [
						replace/all smtp-ctx/mail/message "^/." "^/.."
						smtp-ctx/mail/message: ptr: rejoin [ enline smtp-ctx/mail/message ]
						sys/log/more 'SMTP ["Sending"  min bufsize length? ptr "bytes of" length? ptr ]
						write client take/part ptr bufsize
						smtp-port/state: 'SENDING
						false
					] [
						throw-smtp-error smtp-port "Not allowing us to send ... quitting"
					]
				]
				END [
					either code = 250 [
						sys/log/info 'SMTP "Message successfully sent."
						smtp-port/state: 'QUIT
						write client to binary!  net-log/C join "QUIT" crlf
						true
					][
						throw-smtp-error smtp-port "Some error occurred on sending."
					]
				]
				QUIT [
					throw-smtp-error smtp-port "Should never get here"
				]
			][
				throw-smtp-error smtp-port ["Unknown state " state]
			]
		]
		wrote [
			switch/default state [
				SENDING [
					either not empty? ptr: smtp-ctx/mail/message [
						sys/log/more 'SMTP ["Sending"  min bufsize length? ptr "bytes of" length? ptr ]
						write client to binary! take/part ptr bufsize
					][
						sys/log/debug 'SMTP "Sending ends."
						write client to binary! rejoin [ crlf "." crlf ]
						smtp-port/state: 'END
					]
				]
				QUIT [
					close client
					true
				]
			][
				read client
				false
			]
		]
		close [
			net-log/E "Port closed on me"
			true
		]
	]
]
	
sync-write: func [
	port [port!]
	body [block!]
	/local ctx result rcpt error
][
	sys/log/debug 'SMTP ["sync-write state:" port/state]

	;; there may be multiple recipients...
	;; do validation before actually opening the connection.
	rcpt: select body 'to
	case/all [
		block? :rcpt [
			;; only emails are valid here, so remove everything else...
			rcpt: copy rcpt
			remove-each m rcpt [not email? m]
		]
		email? :rcpt [
			rcpt: to block! rcpt
		]
		any [not block? :rcpt empty? :rcpt] [
			throw-smtp-error port "There must be at least one recipient!"
			return true
		]
	]

	unless ctx: port/extra [
		open port
		ctx: port/extra
		port/state: 'READY
	]
	; construct the email object from the specs 
	ctx/mail: construct/with body mail-obj
	ctx/mail/to: :rcpt

	ctx/connection/awake: :sync-smtp-handler

	if port/state = 'READY [ 
		; the read gets the data from the smtp server and triggers the events
		; that follow that is handled by our state engine in the sync-smtp-handler
		read port 
	]
	until [
		result: wait [ port ctx/connection port/spec/timeout ]
		unless port? result [
			throw-smtp-error port "SMTP timeout"
		]
		any [none? result none? port/state port/state = 'CLOSE]
	]
	if port/state = 'CLOSE [
		close port
	]
	;print "sync-write DONE"
	if all [port port/extra error? port/extra/error][
		do port/extra/error
	]
	true
]
	
sys/make-scheme [
	name: 'smtp
	title: "Simple Mail Transfer Protocol"
	spec: make system/standard/port-spec-net [
		port:    25
		timeout: 60
		ehlo: 
		user:
		pass: none
	]
	actor: [
		open: func [
			port [port!]
			/local conn spec
		][
			if port/extra [return port]
			if none? port/spec/host [
				throw-smtp-error port "Missing host address when opening smtp server"
			]
			; set the port state to hold the tcp port
			port/extra: construct [
				connection:
				mail:
				error:
				rcpt:       ;= used to store the last requested RCPT address
				recipients: ;= number of accepted recipients (must be at least one to proceed data sending)
			]
			spec: port/spec
			; create the tcp port and set it to port/state/connection

			unless spec/ehlo [
				unless spec/ehlo: select system/options 'domain-name [
					try [
						;; resolve external IP and use it as domain name
						spec/ehlo: read http://ifconfig.me/ip
						;; and store it for later use
						put system/options 'domain-name :spec/ehlo
					]
				]
			]
			conn: context [
				scheme: none
				host:   spec/host
				port:   spec/port
				ref:    none
			]
			conn/scheme: either 465 = spec/port ['tls]['tcp]
			conn/ref: as url! ajoin [conn/scheme "://" spec/host #":" spec/port]

			port/state: 'INIT
			port/extra/connection: conn: make port! conn
			
			conn/parent: port
			open conn ;-- open the actual tcp port
			
			; return the newly created and open port
			port
		]
		open?: func [
			port [port!]
		][
			not none? port/state
		]

		close: func [
			port [port!]
		][
			sys/log/debug 'SMTP "Close"
			if open? port [
				close port/extra/connection
				port/extra/connection/awake: none
				port/state: none
			]
			port
		]

		read: func [
			port [port!]
		][
			sys/log/debug 'SMTP "Read"
		]

		write: func [
			port [port!]
			body [block!]
		][
			sync-write port body
		]
	]
	awake: func[event /local port type][
		port: event/port
		type: event/type
		sys/log/debug 'SMTP ["SMTP-Awake event:" type]
		switch/default type [
			error [
				port/state: 'ERROR
				try [ close port/extra/connection ]
				true
			]
			close [
				port/state: 'CLOSE
				true
			]
		][
			sync-smtp-handler :event
		]
	]
]

sys/make-scheme/with [
	name: 'smtps
	title: "Simple Mail Transfer Protocol Secure"
	spec: make spec [
		port: 465
	]
] 'smtp