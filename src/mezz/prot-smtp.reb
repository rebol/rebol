Rebol [
	title:  "Rebol3 SMTP protocol scheme"
	name:    smtp
	type:    module
	author:  ["Graham" "Oldes"]
	rights:  BSD
	version: 0.1.0
	date:    5-Feb-2022
	file:    %prot-smtp.reb
	notes: {
		0.0.1 original tested in 2010
		0.0.2 updated for the open source versions
		0.0.3 Changed to use a synchronous mode rather than async.  Authentication not yet supported
		0.0.4 Added LOGIN, PLAIN and CRAM-MD5 authentication.  Tested against CommunigatePro
		0.0.5 Changed to move credentials to the url or port specification
		0.0.6 Fixed some bugs in transferring email greater than the buffer size.
        0.1.0 Oldes: Updated to work with my Rebol3 fork; including TLS.

        Note that if your password does not work for gmail then you need to 
        generate an app password.  See https://support.google.com/accounts/answer/185833
		
		synchronous mode
		write smtp://user:password@smtp.clear.net.nz [ 
			from:
			name:
			to: 
			subject:
			message: 
		]

		name, and subject are not currently used and may be removed
		
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
            ehlo: "FQDN" ; if you don't have one, then substitute your IP address
		] compose [
			from: me@somewhere.com
			to: recipient@other.com
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

bufsize: 32000 ;-- use a write buffer of 32k for sending large attachments

mail-obj: make object! [ 
	from: 
	to:
	name: 
	subject:
	message: none
]

make-smtp-error: func [
	message
][
	do make error! [
		type: 'Access
		id: 'Protocol
		arg1: message
	]
]

; auth-methods: copy []
alpha: system/catalog/bitsets/alpha
digit: system/catalog/bitsets/numeric

net-log: func[data /C /S /local msg][
	msg: clear ""
	case [
		C [append msg "Client: "]
		S [append msg "Server: "]
	]
	append msg data
	sys/log/more 'SMTP trim/tail msg
	data
]

sync-smtp-handler: function [event][
	; client is the real port ie. port/state/connection
	client: event/port
	state: client/spec/state
	sys/log/debug 'SMTP [state " event " event/type]

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
			client/spec/state: 'EHLO
			;write client to binary! net-log/C ajoin ["EHLO " client/spec/ehlo CRLF]
			;client/spec/state: 'AUTH
			read client
			false
		]

		read [
			response: to string! client/data
			clear client/data
			if empty? response [return false]
			
			code: none
			parse response [copy code: 3 digit to end (code: to integer! code)]

			if system/options/log/smtp > 1 [
				foreach line split trim/tail response CRLF [
					sys/log/more 'SMTP ["Server:^[[32m" line]
				]
			]

			switch/default state [
				EHLO
				INIT [
					write client to binary! net-log/C ajoin ["EHLO " client/spec/ehlo CRLF]
					client/spec/state: 'AUTH
					return false
				]
				AUTH [
					if code = 250 [
						if parse response [
							thru "STARTTLS" CRLF to end (
								make-smtp-error "STARTTLS not implemented!"
							)
							|
							thru "AUTH" [#" " | #"="] copy auth-methods: to CRLF to end (
								auth-methods: split auth-methods #" "
								foreach auth auth-methods [
									try [auth: to word! auth]
									switch auth [
										CRAM-MD5 [
											client/spec/state: 'CRAM-MD5
											write client to binary! net-log/C "AUTH CRAM-MD5^M^/"
											return false
										]
										LOGIN [
											client/spec/state: 'LOGIN
											write client to binary! net-log/C "AUTH LOGIN^M^/"
											return false
										]
										PLAIN [
											client/spec/state: 'PLAIN
											write client to binary! ajoin [
												"AUTH PLAIN "
												enbase ajoin [client/spec/user #"^@" client/spec/user #"^@" client/spec/pass] 64
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
						client/spec/state: 'FROM
						write client to binary! net-log/C ajoin ["MAIL FROM: <" client/spec/email/from ">" CRLF]
						return false
					]
				]

				LOGIN [
					case [
						find/part response "334 VXNlcm5hbWU6" 16 [ ;enbased "Username:"
							; username being requested
							sys/log/more 'SMTP "Client: ***user-name***"
							write client to binary! ajoin [enbase client/spec/user 64 CRLF]
						]
						find/part response "334 UGFzc3dvcmQ6" 16 [ ;enbased "Password:"
                            ; pass being requested
                            sys/log/more 'SMTP "Client: ***user-pass***"
							write client to binary! ajoin [enbase client/spec/pass 64 CRLF]
							client/spec/state: 'PASSWORD
						]
						true [
							make-smtp-error join "Unknown response in AUTH LOGIN " response						
						]
					]
				]
				
				CRAM-MD5 [
					either code = 334 [
						auth-key: skip response 4
						auth-key: debase auth-key 64
						; compute challenge response
						auth-key: checksum/with auth-key 'md5 client/spec/pass
						sys/log/more 'SMTP "Client: ***auth-key***"
						write client to binary! ajoin [enbase ajoin [client/spec/user #" " lowercase enbase auth-key 16] 64 CRLF]
						client/spec/state: 'PASSWORD
					][
						make-smtp-error join "Unknown response in AUTH CRAM-MD5 " response						
					]
				]
				
				PASSWORD [
					either code = 235 [
						client/spec/state: 'FROM
						write client to binary! net-log/C ajoin ["MAIL FROM: <" client/spec/email/from ">" CRLF	]
					][
						make-smtp-error "Failed authentication"
					]
				]

				FROM [
					either code = 250 [
						write client to binary! net-log/C ajoin ["RCPT TO: <" client/spec/email/to ">" crlf]
						client/spec/state: 'TO
					] [
						net-log "Rejected by server"
						return true
					]
				]
				TO [
					either code = 250 [
						client/spec/state: 'DATA
						write client to binary! net-log/C join "DATA" CRLF
					] [
						net-log "server rejects TO address"
						return true
					]
				]
				DATA [
					either code = 354 [
						replace/all client/spec/email/message "^/." "^/.."
						client/spec/email/message: ptr: rejoin [ enline client/spec/email/message ]
						sys/log/more 'SMTP ["Sending"  min bufsize length? ptr "bytes of" length? ptr ]
						write client take/part ptr bufsize
						client/spec/state: 'SENDING
					] [
						net-log "Not allowing us to send ... quitting"
					]
				]
				
				END [
					either code = 250 [
						net-log "Message successfully sent."
						client/spec/state: 'QUIT
						write client to binary!  net-log/C join "QUIT" crlf
						return true
					][
						net-log "Some error occurred on sending."
						return true
					]
				]
				QUIT [
					net-log "Should never get here"
				]
			][
				net-log join "Unknown state " state
			]
		]
		wrote [
			either state = 'SENDING [
				either not empty? ptr: client/spec/email/message [
					sys/log/debug 'SMTP ["Sending "  min bufsize length? ptr " bytes of " length? ptr ]
					write client to binary! take/part ptr bufsize
				][
					sys/log/debug 'SMTP "Sending ends."
					write client to binary! rejoin [ crlf "." crlf ]
					client/spec/state: 'END
				]
			][
				read client
			]
		]
		close [net-log "Port closed on me"]
	]
	false
]
	
sync-write: func [ port [port!] body
	/local state result
][
	unless port/state [ open port port/state/close?: yes ]
	state: port/state
	; construct the email from the specs 
	port/state/connection/spec/email: construct/with body mail-obj
	
	port/state/connection/awake: :sync-smtp-handler
	if state/state = 'ready [ 
		; the read gets the data from the smtp server and triggers the events that follow that is handled by our state engine in the sync-smtp-handler
		read port 
	]
	unless port? wait [ state/connection port/spec/timeout ] [ make error! "SMTP timeout" ]
	if state/close? [ close port ]
	true
]
	
sys/make-scheme [
	name: 'smtp
	title: "Simple Mail Transfer Protocol"
	spec: make system/standard/port-spec-net [
		port: 25
		timeout: 60
		email: ;-- object constructed from argument
		ehlo: 
		user:
		pass: none
	]
	actor: [
		open: func [
			port [port!]
			/local conn spec
		] [
			if port/state [return port]
			if none? port/spec/host [
				make-smtp-error "Missing host address when opening smtp server"
			]
			; set the port state to hold the tcp port
			port/state: construct [
				state:
				connection:
				error:
				awake: none  ;-- so port/state/awake will hold the awake handler :port/awake
				close?: no   ;-- flag for us to decide whether to close the port eg in syn mode
			]
			spec: port/spec
            ; create the tcp port and set it to port/state/connection
           ; unless system/user/identity/fqdn [make-smtp-error "Need to provide a value for the system/user/identity/fqdn"]
            either 465 = spec/port [
                port/state/connection: conn: make port! [
                    scheme: 'tls
                    host:   spec/host
                    port:   spec/port
                    state:  'INIT
                    ref:    rejoin [tls:// host ":" port]
                    email:  spec/email
                    user:   spec/user
                    pass:   spec/pass
                    ehlo:   spec/ehlo ; "system/user/identity/fqdn]
                ]
            ][
                port/state/connection: conn: make port! [
                    scheme: 'tcp
                    host:   spec/host
                    port:   spec/port
                    state:  'INIT
                    ref:    rejoin [tcp:// host ":" port]
                    email:  spec/email
                    user:   spec/user
                    pass:   spec/pass
                    ehlo:   spec/ehlo ;any [port/spec/ehlo system/user/identity/fqdn]
                ]
            ]
            conn/extra: port
			open conn ;-- open the actual tcp port
			
			; return the newly created and open port
			port
		]
		open?: func [
			port [port!]
		] [
			all [port/state]
		]

		close: func [
			port [port!]
		] [
			if open? port [
				close port/state/connection
				port/state/connection/awake: none
				port/state: none
			]
			port
		]

		read: func [
			port [port!]
		] [
			either any-function? :port/awake [
				either not open? port [
					print "opening & waiting on port"
					unless port? wait [open port/state/connection port/spec/timeout] [make-smtp-error "Timeout"]
					; wait open port/state/connection
				] [
					print "waiting on port"
					unless port? wait [ port/state/connection port/spec/timeout] [make-smtp-error "Timeout"]
				]
				port
			] [
				print "No handler for the port exists yet"
				; should this be used at all for smtp?
			]
		]

		write: func [
			port [port!] body [block!]
			/local conn email
		][
			sync-write port body
		]
	]
]

sys/make-scheme/with [
	name: 'smtps
	title: "Secure Simple Mail Transfer Protocol"
	spec: make spec [
		port: 465
	]
] 'smtp