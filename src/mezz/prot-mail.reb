Rebol [
	Name:    mail
	Type:    module
	Options: [delay]
	Version: 0.1.0
	Date:    14-Jul-2022
	File:    %prot-mail.reb
	Title:   "Rebol MAIL related functions"
	Purpose: {
		This `mail` scheme is supposed to be a higher level scheme
		for sending (using SMTP) and receiving emails (using POP3 or IMAP),

		The goal is to have it work asynchronously (not blocking) with possibility
		to reuse already opened connections. But that is not yet implemented.
		Also so far only sending mails is available (missing POP3/IMAP schemes).
	}
	Author:  @Oldes
	Rights:  "Copyright (C) 2022 Oldes. All rights reserved."
	License: MIT
	History: [
		0.1.0 14-Jul-2022 "Oldes" "Initial version (sending only)"
	]
	Needs:   [mime-field mime-types]
	Exports: [send]
]

form-address: func[
	address [email! tag! block! none!]
	/local out name mail
][
	unless address [return none]
	out: clear ""

	either block? address [
		parse address [some [
			end
			|
			(append out ", ")
			set mail: email! (
				append append out #" " mold as tag! mail
			)
			|
			copy name: to email! set mail: email! (
				append out encode 'mime-field form name
				append append out #" " mold as tag! mail
			)
		]]
		out: skip out 2
	][
		append out mold as tag! address
	]

	if 76 < length? out [replace/all out #"," ",^M^/   "]
	; no copy, so be careful!
	either empty? out [none][out]
]

form-text: func[
	text [any-string! none!]
][
	either any [
		none? text
		parse text [some atext-sp end]
	][	text][ encode 'mime-field text ]
]

form-message: func[
	mail
	/local
	 out from rcpt date value msg Message Message-ID attach boundary file filename file-id
][
	out: make string! 100
	append out "MIME-Version: 1.0^M^/"

	case [
		email? from: select mail 'From []
		tag?   from [ from: as email! :from ]
		block? from [ parse from [to email! set from email!] ]
	]
	unless :from [ 
		put mail 'From from: any [
			user's full-email
			user's email
			load ask "Mail from: "
		]
		;; There is difference between `From` in the message body,
		;; where it can contain also user's name, and in
		;; the `form` used in the SMTP scheme (can be just the email!)
		if block? from [ parse from [to email! set from email!] ]
	]
	unless email? from [
		sys/log/error 'MAIL ["Invalid FROM value: " as-red mold :from]
		return none
	]

	case [
		email? rcpt: select mail 'To []
		tag?   rcpt [ rcpt: as email! :rcpt ]
		block? rcpt [ ]
	]
	unless :rcpt [
		put mail 'To rcpt: load ask "Mail rcpt: "
	]
	
	unless date? date: select mail 'Date [date: now]
	append out ajoin ["Date: " to-idate :date CRLF] 

	foreach field [From To CC Return-Path][ ; do not include BCC here!
		if value: form-address select mail field [
			append out ajoin [field-names/:field :value CRLF] 
		]
	]
	foreach field [Subject Comment Organization][
		if value: form-text select mail field [
			append out ajoin [field-names/:field :value CRLF] 
		]
	]
	Message-ID: any [
		select mail 'Message-ID
		ajoin [get-id 36 #"." ++ counter "@rebol.mail"]
	]
	append out ajoin ["Message-ID: " as tag! :Message-Id CRLF] 
	append out ajoin ["X-REBOL: Rebol/" system/version " (" system/platform "; " system/build/arch ")^M^/"]
	
	Message: any [select mail 'Message ""]
	msg: make string! 1000

	either all [
		block? Message
		parse Message [2 string! to end]
	][
		boundary: ajoin ["^M^/^M^/--ALTER-" get-id 16]
		append msg ajoin [
			{Content-Type: multipart/alternative; boundary="} skip :boundary 6 #"^""
			:boundary CRLF
			{Content-Type: text/plain; charset="UTF-8"}	CRLF
			{Content-Transfer-Encoding: quoted-printable} CRLF
			CRLF encode 'quoted-printable Message/1
			:boundary CRLF
			{Content-Type: text/html; charset="UTF-8"} CRLF
			{Content-Transfer-Encoding: quoted-printable} CRLF
			CRLF encode 'quoted-printable Message/2
			:boundary "--"
		]
	][
		append msg ajoin [
			{Content-Type: text/plain; charset="UTF-8"}	CRLF
			{Content-Transfer-Encoding: quoted-printable} CRLF
			CRLF encode 'quoted-printable Message
		]
	]

	attach: select mail 'attach
	if file? :attach [attach: to block! attach]
	either all [
		block? :attach
		parse attach [to file! to end] ; if there is at least one file!
	][
		boundary: ajoin ["^M^/^M^/--MIXED-" get-id 16]
		append out ajoin [
			{Content-Type: multipart/mixed; boundary="} skip :boundary 6 #"^""
			:boundary CRLF
			:msg
		]
		msg: none
		file-id: 0
		parse attach [
			some [
				to file! set file file! (
					++ file-id
					filename: ajoin ["=?UTF-8?Q?" encode 'quoted-printable as string! second split-path file "?="]
					append out ajoin [
						:boundary CRLF
						{Content-Type: } mime-type? file {; name="} filename #"^"" CRLF
						{Content-Disposition: attachment; filename="} filename #"^"" CRLF
						{Content-Transfer-Encoding: base64} CRLF
						{Content-ID: <f_} file-id  {>} CRLF
						{X-Attachment-Id: f_} file-id  CRLF
						CRLF
						enbase read/binary :file 64 
					]
				)
				| skip ;= silently ignored?
			]
		]
		append append out :boundary "--"
	][
		append out :msg
	]

	;print out
	compose/only [
		from:    (from)
		to:      (rcpt)
		message: (out)
	]
]

get-id: func[base [integer!] /func n t][
	n: 1000000 * to integer! now
	; using just date is not precise enough, so add
	; miliseconds from stats/timer
	t: pick stats/timer 'second
	n: n + to integer! (t - to integer! t) * 1000000
	enbase to binary! n base
]

counter: 0 ;- used when generating message ids

;; To make sure, that the latter case is correct (although it does not have to be)!
field-names: #[
	From:         "From: "
	To:           "To:   "
	Cc:           "Cc:   "
	Return-Path:  "Return-Path: "
	Subject:      "Subject: "
	Comment:      "Comment: "
	Organization: "Organization: "
	;In-Reply-To:  "In-Reply-To: "
	;References:   "References: "
]

atext:    #(bitset! #{000000005F35FFC57FFFFFE3FFFFFFFE}) ;= charset [#"a"-#"z" #"A"-#"Z" #"0"-#"9" "!#$%&'*+-/=?^^_`{}|~"]
atext-sp: #(bitset! #{00400000DF35FFC57FFFFFE3FFFFFFFE}) ;= including sp and tab 

;------------------------------------------------------------------
;; This is for now just a placeholder, because only `write` action
;; is actually used. The idea is, that this scheme could work also
;; in async mode and that it could cache opened inner connections.
;;
;; Once there will be available POP3 and IMAP schemes, these should
;; be also accessed using this higher level scheme, so basically
;; `write` would use SMTP, while `read` would use POP3 or IMAP.

sys/make-scheme [
	name: 'mail
	actor: [
		open: func [port [port!]][
			either all [
				port? system/ports/mail
				'mail = system/ports/mail/scheme/name
			][
				system/ports/mail
			][
				port/state: object [
					state: 'READY
					; more to be added here...
				]
				system/ports/mail: port
			]
		]
		open?: func[port [port!]][
			object? port/state
		]
		write: func[
			port [port!]
			mail [block! map! object!]
			/local
			 smtp message
		][
			unless open? port [port: open port]
			if all [
				smtp:    user's smtp
				message: form-message mail
			][
				write :smtp :message
			]
		]
		close: func[port [port!]][
			if same? port system/ports/mail [
				system/ports/mail: none
			]
			port/state: none
			port
		]
	]
]

;-- And finally a simple shortcut function...

send: func[
	"Send a message to an address (or block of addresses)"
	address [email! block!] "An address or block of addresses"
	message [string! file! block! object! map!] "Simple text message, file or mail data"
][
	case [
		string? message [
			message: compose/only [
				to:      (address)
				subject: (copy/part message any [find message newline 50])
				message: (message)
			]
		]
		file? message [
			message: compose/only [
				to:      (address)
				subject: (join "File: " mold message)
				attach:  (message)
			]
		]
		'else [
			;; input validation...
			unless block? message [message: spec-of :message]
			parse message [
				some file! (
					; multiple attachemens...
					message: compose/only [
						subject: "Multiple files"
						attach:  (message)
					]
					break
				)
				|
				2 string! (
					; alternative plain/html message
					message: compose/only [
						subject: (copy/part message/1 any [find message/1 newline 50])
						message: (message)
					]
					break
				)
				|
				some [[word! | set-word!] [string! | file! | block!]] end
				|
				(cause-error 'Script 'invalid-arg reduce [message])
			]
			unless any [
				select message 'message
				select message 'subject
				select message 'attach
			][
				;; there must be at least some content to send
				cause-error 'Script 'invalid-arg reduce [message]
			]
			put message 'to address
		]
	]
	write mail:// message
]