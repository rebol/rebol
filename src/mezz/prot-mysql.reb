REBOL [
	Title: "MySQL Protocol"
	Purpose: "MySQL Driver for REBOL"
	Name: 'mysql
	Type: 'module
	Exports: [connect-sql send-sql sql-escape to-sql-binary to-sql mysql-map-rebol-values]
	Authors: [
		"Nenad Rakocevic / SOFTINNOV <mysql@softinnov.com>"
		"Shixin Zeng <szeng@atronixengineering.com>"
		"Oldes"
	]
	Web: http://softinnov.org/rebol/mysql.shtml
	Date: 27-May-2020
	File: %prot-mysql.r
	Version: 2.1.0
	History: [
		1.3.1 "Nenad Rakocevic" "Rebol2 original version"
		2.0.0 "Shixin Zeng"     "Rebol3 async version"
		2.1.0 "Oldes"           "Including it in Rebol3 source code as a module"
	]
	References: [
		http://softinnov.org/rebol/mysql.shtml
		https://github.com/zsx/mysql-r3
	]
	Notes: {
		Old "native_password" authentication is required so far (with MySQL 8.0).
		For details how to enable it see: https://stackoverflow.com/a/50131831/494472
	}
]

append system/options/log [mysql: 1]

mysql-driver: make object! [
	sql-buffer: make string! 1024
	not-squote: complement charset "'"
	not-dquote: complement charset {"}

	copy*:		get in system/contexts/lib 'copy
	insert*:	get in system/contexts/lib 'insert
	close*:		get in system/contexts/lib 'close

	std-header-length: 4
	std-comp-header-length:	3
	end-marker: 254

	change-type-handler: none
;------ Internals --------

	defs: compose/deep [
		cmd [
			sleep			0
			quit			1
			init-db			2
			query			3
			field-list		4
			create-db		5
			drop-db			6
			reload			7
			shutdown		8
			statistics		9
			;process-info	10
			;connect		11
			process-kill	12
			debug			13
			ping			14
			;time			15
			;delayed-insert	16
			change-user		17
		]
		refresh [
			grant		1	; Refresh grant tables
			log			2	; Start on new log file
			tables		4	; Close all tables 
			hosts		8	; Flush host cache
			status		16	; Flush status variables
			threads		32	; Flush status variables
			slave		64	; Reset master info and restart slave thread
			master		128 ; Remove all bin logs in the index
		]					; and truncate the index
		types [
			0		decimal
			1		tiny
			2		short
			3		long
			4		float
			5		double
			6		null
			7		timestamp
			8		longlong
			9		int24
			10		date
			11		time
			12		datetime
			13		year
			14		newdate
			15		var-char
			16		bit
			246		new-decimal
			247		enum
			248		set
			249		tiny-blob
			250		medium-blob
			251		long-blob
			252		blob
			253		var-string
			254		string
			255		geometry
		]
		flag [
			not-null		1		; field can't be NULL
			primary-key		2		; field is part of a primary key
			unique-key 		4		; field is part of a unique key
			multiple-key	8		; field is part of a key
			blob			16
			unsigned		32
			zero-fill		64
			binary			128
			enum			256		; field is an enum
			auto-increment	512		; field is a autoincrement field
			timestamp		1024	; field is a timestamp
			set				2048	; field is a set
			num				32768	; field is num (for tcp-ports)
		]
		client [
			long-password		1		; new more secure passwords
			found-rows			2		; Found instead of affected rows
			long-flag			4		; Get all column flags
			connect-with-db		8		; One can specify db on connect
			no-schema			16		; Don't allow db.table.column
			compress			32		; Can use compression protcol
			odbc				64		; Odbc tcp-port
			local-files			128		; Can use LOAD DATA LOCAL
			ignore-space		256		; Ignore spaces before '('
			protocol-41			512		; Use new protocol (was "Support the mysql_change_user()")
			interactive			1024	; This is an interactive tcp-port
			ssl					2048	; Switch to SSL after handshake
			ignore-sigpipe		4096	; IGNORE sigpipes
			transactions		8196	; Client knows about transactions
			reserved			16384	; protocol 4.1 (old flag)
			secure-connection	32768	; use new hashing algorithm
			multi-queries		65536	; enable/disable multiple queries support
			multi-results		131072	; enable/disable multiple result sets
			ps-multi-results	(shift 1 18)	; multiple result sets in PS-protocol
			plugin-auth			(shift 1 19) ; Client supports plugin authentication
			ssl-verify-server-cert	(shift 1 30)
			remember-options		(shift 1 31)
		]
	]

	locals-class: make object! [
	;--- Internals (do not touch!)---
		seq-num: 0
		last-status:
		stream-end?:		;Should an immediate READ be following this packet?
		more-results?: false
		expecting: none
		packet-len: 0
		last-activity: now/precise
		next-packet-length: 0
		current-cmd:
		state:
		packet-state: none
		o-buf:
		buf: make binary! 4 ; buffer for big packets
		data-in-buf?:	; when this is set, the packet data is in `buf`, instead of tcp-port/data
		current-result: none ;current result set
		results: copy [] ;all result sets in current send-sql, there could be multiple results if more queries were sent in one call
		result-options: none
		current-cmd-data: none
		query-start-time: none
	;-------
		auto-commit: on		; not used, just reserved for /Command compatibility.
		delimiter: #";"
		newlines?: value? 'new-line
		last-insert-id: make block! 1
		affected-rows: 0
		init:
		protocol:
		version:
		thread-id:
		crypt-seed:
		capabilities:
		error-code:
		error-msg:
		conv-list: 
		character-set:
		server-status:
		seed-length:
		auth-v11: none
	]

	result-class: make object! [
		query-start-time: 0
		query-finish-time: 0
		n-columns:
		affected-rows:
		warnings: 0
		server-status: 0
		last-insert-id: 0
		rows: make block! 1
		columns: make block! 1
	]

	result-option-class: make object! [
		named?: off
		auto-conv?: on
		flat?: off
		newlines?: off
		verbose?: off
	]

	column-class: make object! [
		table: name: length: type: flags: decimals: none
		catalog: db: org_table: org_name: charsetnr: length: default: none
	]
	
	month: [ "Jan" "Feb" "Mar" "Apr" "May" "Jun" "Jul"
		 "Aug" "Sep" "Oct" "Nov" "Dec" ]
	my-to-date: func [v	/local mm dd yy][
		any [
			attempt [
				yy: copy*/part v 4
				mm: copy*/part skip v 5 2
				dd: copy*/part skip v 8 2
				either (to-integer mm) <> 0 [
					to date! rejoin [dd "-" month/(to-integer mm) "-" yy]
				][
					to date! rejoin ["01-Jan-"yy]
				]
			] 
			1-jan-0000
		]
	]
	my-to-datetime: func [v /local mm dd yy h m][
		any [
			attempt [
				yy: copy*/part v 4
				mm: copy*/part skip v 5 2
				dd: copy*/part skip v 8 2
				
				h: copy*/part skip v 11 2
				m: copy*/part skip v 14 2
				either (to-integer mm) <> 0 [
					to date! rejoin [dd "-" month/(to-integer mm) "-" yy "/" h ":" m]
				][
					to date! rejoin ["01-Jan-" yy "/" h ":" m]
				] 
			]
			1-jan-0000/00:00
		]
	]
	
	
	conv-model: [
		decimal			[to decimal! to string!]
		tiny			[to integer! to string!]
		short			[to integer! to string!]
		long			[to integer! to string!]
		float			[to decimal! to string!]
		double			[to decimal! to string!]
		null			[to string!]
		timestamp		[to string!]
		longlong		[to integer! to string!]
		int24			[to integer! to string!]
		date			[my-to-date to string!]
		time			[to time! to string!]
		datetime		[my-to-datetime to string!]
		year			[to integer! to string!]
		newdate			[to string!]
		var-char		[to string!]
		bit				none
		new-decimal		[to decimal! to string!]
		enum			[to string!]
		set				[to string!]
		tiny-blob		none
		medium-blob		none
		long-blob		none
		blob			none
		tiny-text		[to string!]
		medium-text		[to string!]
		long-text		[to string!]
		text			[to string!]
		var-string		[to string!]
		string			[to string!]
		geometry		[to string!]
	]

	set 'change-type-handler func [p [port!] type [word!] blk [block!]][
		head change/only next find p/locals/conv-list type blk
	]
	
	convert-types: func [
		p [port!]
		rows [block!] 
		/local row i convert-body action cols col conv-func tmp
	][
		;;sys/log/more 'MySQL["converting types"]
		cols: p/locals/current-result/columns
		convert-body: make block! 1
		action: [if tmp: pick row (i)]
		foreach col cols [
			i: index? find cols col
			if 'none <> conv-func: select p/locals/conv-list col/type [
				;;sys/log/more 'MySQL["conv-func:" mold conv-func "for" col/type]
				append convert-body append/only compose action head
					insert* at compose [change at row (i) :tmp] 5 conv-func
			]
		]
		;;sys/log/more 'MySQL["convert-body:" mold convert-body]
		if not empty? convert-body [
			either p/locals/result-options/flat? [
				while [not tail? rows][
					row: rows
					do convert-body
					rows: skip rows length? cols
				]
			][
				foreach row rows :convert-body
			]
		]
	]
	
	decode: func [int [integer!] /features /flags /type /local list name value][
		either type [
			any [select defs/types int 'unknown]
		][
			list: make block! 10
			foreach [name value] either flags [defs/flag][defs/client][
				if value = (int and value) [append list :name]	
			]
			list
		]
	]
	
	encode-refresh: func [blk [block!] /local total name value][
		total: 0
		foreach name blk [
			either value: select defs/refresh :name [
				total: total + value
			][
				net-error reform ["Unknown argument:" :name]
			]
		]
		total
	]

	sql-chars: charset sql-want: {^(00)^/^-^M^(08)'"\}
	sql-no-chars: complement sql-chars
	escaped: make map! [
		#"^(00)"	"\0"
		#"^/" 		"\n"
		#"^-" 		"\t"
		#"^M" 		"\r"
		#"^(08)" 	"\b"
		#"'" 		"\'"
		#"^""		{\"}
		#"\" 		"\\"
	]

	set 'sql-escape func [value [string!] /local c][
		parse/all value [
			any [
				c: sql-chars (c: change/part c select escaped c/1 1) :c 
				| sql-no-chars
			]
		]
		value
	]

	set 'to-sql-binary func [value [binary!] /local i][
		m: make string! 10 + (2 * length? value) ;preallocate space for better performance
		append m "_binary 0x"
		forall value [
			i: to integer! first value
			append m pick "0123456789ABCDEF" (to integer! i / 16) + 1
			append m pick "0123456789ABCDEF" (i // 16) + 1
		]
		m
	]

	set 'to-sql func [value /local res][
		switch/default type?/word value [
			none!	["NULL"]
			date!	[
				rejoin ["'" value/year "-" value/month "-" value/day
					either value: value/time [
						rejoin [" " value/hour	":" value/minute ":" value/second]
					][""] "'"
				]
			]
			time!	[join "'" [value/hour ":" value/minute ":" value/second "'"]]
			money!	[head remove find mold value "$"]
			string!	[join "'" [sql-escape copy* value "'"]]
			binary!	[to-sql-binary value]
			block!	[
				if empty? value: reduce value [return "()"]
				res: append make string! 100 #"("
				forall value [repend res [to-sql value/1 #","]]
				head change back tail res #")"
			]
		][
			either any-string? value [to-sql form value][form value]
		]
	]
	
	set 'mysql-map-rebol-values func [data [block!] /local args sql mark][
		args: reduce next data
		sql: copy* pick data 1
		mark: sql
		while [mark: find mark #"?"][
			mark: insert* remove mark either tail? args ["NULL"][to-sql args/1]
			if not tail? args [args: next args]
		]
		;sys/log/more 'MySQL sql
		sql
	]
	
	show-server: func [obj [object!]][
		sys/log/more 'MySQL[														newline
			"----- Server ------" 									newline
			"Version:"					obj/version					newline
			"Protocol version:"			obj/protocol 				newline
			"Thread ID:" 				obj/thread-id 				newline
			"Crypt Seed:"				obj/crypt-seed				newline
			"Capabilities:"				mold decode/features 		obj/capabilities 		newline
			"Seed length:"				obj/seed-length 			newline
			"-------------------"
		]	
	]

;------ Encryption support functions ------

	scrambler: make object! [
		to-pair: func [value [integer!]][system/words/to-pair reduce [value 1]]
		xor-pair: func [p1 p2][to-pair p1/x xor p2/x]
		or-pair: func [p1 p2][to-pair p1/x or p2/x]
		and-pair: func [p1 p2][to-pair p1/x and p2/x]
		remainder-pair: func [val1 val2 /local new][
			val1: either negative? val1/x [abs val1/x + 2147483647.0][val1/x]
			val2: either negative? val2/x [abs val2/x + 2147483647.0][val2/x]
			to-pair to-integer val1 // val2
		]
		floor: func [value][
			value: to-integer either negative? value [value - .999999999999999][value]
			either negative? value [complement value][value]
		]

		hash-v9: func [data [string!] /local nr nr2 byte][
			nr: 1345345333x1
			nr2: 7x1
			foreach byte data [
				if all [byte <> #" " byte <> #"^-"][
					byte: to-pair to-integer byte
					nr: xor-pair nr (((and-pair 63x1 nr) + nr2) * byte) + (nr * 256x1)
					nr2: nr2 + byte
				]
			]
			nr
		]

		hash-v10: func [data [string!] /local nr nr2 adding byte][
			nr: 1345345333x1
			adding: 7x1
			nr2: to-pair to-integer #12345671
			foreach byte data [
				if all [byte <> #" " byte <> #"^-"][
					byte: to-pair to-integer byte
					nr: xor-pair nr (((and-pair 63x1 nr) + adding) * byte) + (nr * 256x1)
					nr2: nr2 + xor-pair nr (nr2 * 256x1)
					adding: adding + byte
				]
			]
			nr: and-pair nr to-pair to-integer #7FFFFFFF
			nr2: and-pair nr2 to-pair to-integer #7FFFFFFF
			reduce [nr nr2]
		]

		crypt-v9: func [data [string!] seed [string!] /local
			new max-value clip-max hp hm nr seed1 seed2 d b i
		][
			new: make string! length? seed
			max-value: to-pair to-integer #01FFFFFF
			clip-max: func [value][remainder-pair value max-value]
			hp: hash-v9 seed
			hm: hash-v9 data	
			nr: clip-max xor-pair hp hm
			seed1: nr
			seed2: nr / 2x1

			foreach i seed [
				seed1: clip-max ((seed1 * 3x1) + seed2)
				seed2: clip-max (seed1 + seed2 + 33x1)
				d: seed1/x / to-decimal max-value/x
				append new to-char floor (d * 31) + 64
			]
			new
		]

		crypt-v10: func [data [string!] seed [string!] /local
			new max-value clip-max pw msg seed1 seed2 d b i
		][
			new: make string! length? seed
			max-value: to-pair to-integer #3FFFFFFF
			clip-max: func [value][remainder-pair value max-value]
			pw: hash-v10 seed
			msg: hash-v10 data	

			seed1: clip-max xor-pair pw/1 msg/1
			seed2: clip-max xor-pair pw/2 msg/2

			foreach i seed [
				seed1: clip-max ((seed1 * 3x1) + seed2)
				seed2: clip-max (seed1 + seed2 + 33x1)
				d: seed1/x / to-decimal max-value/x
				append new to-char floor (d * 31) + 64
			]		
			seed1: clip-max (seed1 * 3x1) + seed2
			seed2: clip-max seed1 + seed2 + 33x0
			d: seed1/x / to-decimal max-value/x
			b: to-char floor (d * 31)

			forall new [new/1: new/1 xor b]
			head new
		]
		
		;--- New 4.1.0+ authentication scheme ---
		crypt-v11: func [data [binary!] seed [binary!] /local key1 key2][
			key1: checksum data 'sha1
			key2: checksum key1 'sha1
			key1 xor checksum rejoin [(to-binary seed) key2] 'sha1
		]
		
		scramble: func [data [string! none!] port [port!] /v10 /local seed][
			if any [none? data empty? data][return make binary! 0]
			seed: port/locals/crypt-seed
			if v10 [return crypt-v10 data copy*/part seed 8]
			either port/locals/protocol > 9 [
				either port/locals/auth-v11 [
					crypt-v11 to-binary data seed
				][
					crypt-v10 data seed
				]
			][
				crypt-v9 data seed
			]
		]
	]

	scramble: get in scrambler 'scramble
	
;------ Data reading ------

	b0: b1: b2: b3: int: int24: long: string: field: len: byte: s: none
	byte-char: complement charset []
	null: to-char 0
	ws: charset " ^-^M^/"

	read-bin-string: [[copy string to null null] | [copy string to end]] ;null-terminated string
	read-string: [read-bin-string (string: to string! string)] ;null-terminated string
	read-byte: [copy byte byte-char (byte: to integer! :byte)]

	;mysql uses little endian for all integers
	read-int: [
		read-byte (b0: byte)
		read-byte (b1: byte	int: b0 + (256 * b1))
	]
	read-int24: [
		read-byte (b0: byte)
		read-byte (b1: byte)
		read-byte (b2: byte	int24: b0 + (256 * b1) + (65536 * b2))
	]
	read-long: [
		read-byte (b0: byte)
		read-byte (b1: byte)
		read-byte (b2: byte)
		read-byte (
			b3: byte
			long: to-integer b0 or (shift b1 8) or (shift b2 16) or (shift b3 24) ;use or instead of arithmetic operations since rebol doesn't handle unsigned integers and the number could be larger than (2^31 - 1)
		)
	]
	read-long64: [
		read-long (low: long)
		read-long (long64: (shift long 32) or low)
	]
	read-length: [; length coded binary
		#"^(FC)" read-int (len: int)
		| #"^(FD)" read-int24 (len: int24)
		| #"^(FE)" read-long64 (len: long64)
		| read-byte (len: byte)
	]
	read-field: [ ;length coded string
		"^(FB)" (field: none)
		| read-length copy field [len byte-char]
	]
	
	read-cmd: func [port [port!] cmd [integer!] /local res][
		either cmd = defs/cmd/statistics [
			to-string read-packet port
		][
			res: read-packet port
			either all [cmd = defs/cmd/ping zero? port/locals/last-status][true][none]
		]
	]
	
;------ Data sending ------

	write-byte: func [value [integer!] /local b][
		b: skip to binary! value 7
	]
	
	write-int: func [value [integer!]][
		join write-byte value // 256 write-byte value / 256
	]

	write-int24: func [value [integer!]][
		join write-byte value // 256 [
			write-byte (to integer! value / 256) and 255
			write-byte (to integer! value / 65536) and 255
		]
	]

	write-long: func [value [integer!]][
		join write-byte value // 256 [
			write-byte (to integer! value / 256) and 255
			write-byte (to integer! value / 65536) and 255
			write-byte (to integer! value / 16777216) and 255
		]
	]

	write-string: func [value [string! none! binary!] /local t][
		if none? value [return make binary! 0]
		;sys/log/more 'MySQL["writing a string:" mold value]
		to-binary join value to char! 0
	]
	
	pack-packet: func [port [port!] data /local header][
		while [16777215 <= length? data] [; size < 2**24 - 1
			header: join
				#{FFFFFF}
				write-byte port/locals/seq-num: port/locals/seq-num + 1

			insert data header
			data: skip data 16777215 + length? header
		]
		;;sys/log/more 'MySQL["write function port state " open? tcp-port]
		;if not open? tcp-port [open tcp-port]
		;sys/log/more 'MySQL["state:" tcp-port/locals/state]
		header: join
			write-int24 length? data
			write-byte port/locals/seq-num: port/locals/seq-num + 1

		insert data header
		head data
	]

	send-packet: func [port [port!] data [binary!] /local tcp-port header][
		write port pack-packet port data
	]

	send-cmd: func [port [port!] cmd [integer!] cmd-data] compose/deep [
		port/locals/seq-num: -1
		port/locals/current-cmd: cmd
		;sys/log/more 'MySQL["sending cmd:" cmd]
		send-packet port rejoin [
			write-byte cmd
			port/locals/current-cmd-data: 
			switch/default cmd [
				(defs/cmd/quit)			[""]
				(defs/cmd/shutdown)		[""]
				(defs/cmd/statistics)	[""]
				(defs/cmd/debug)		[""]
				(defs/cmd/ping)			[""]
				(defs/cmd/field-list)	[write-string pick cmd-data 1]
				(defs/cmd/reload)		[write-byte encode-refresh cmd-data]
				(defs/cmd/process-kill)	[write-long pick cmd-data 1]
				(defs/cmd/change-user)	[
					rejoin [
						write-string pick cmd-data 1
						write-string scramble pick cmd-data 2 port
						write-string pick cmd-data 3
					]
				]
			][either string? cmd-data [cmd-data][pick cmd-data 1]]
		]
		;sys/log/more 'MySQL["sent a command"]
		port/locals/state: 'sending-cmd
	]
	
	insert-query: func [port [port!] data [string! block!]][
		sys/log/more 'MySQL["insert-query:" data]
		send-cmd port defs/cmd/query data
		none
	]
	
	insert-all-queries: func [port [port!] data [string!] /local s e res d][
		d: port/locals/delimiter
		parse/all s: data [
			any [
				#"#" thru newline
				| #"'"  any ["\\" | "\'" | "''" | not-squote] #"'"
				| #"^"" any [{\"} | {""} | not-dquote] #"^""
				| #"`" thru #"`"
				| "begin" thru "end"
				| e: d (
					clear sql-buffer
					insert*/part sql-buffer s e
					res: insert-query port sql-buffer
				  ) any [ws] s:
				| skip
			]
		]
		if not tail? s [res: insert-query port s]
		res
	]

	insert-cmd: func [port [port!] data [block!] /local type][
		;sys/log/more 'MySQL["inserting cmd:" mold data]
		type: select defs/cmd data/1
		either type [
			send-cmd port type next data
		][
			cause-error 'user 'message reform ["Unknown command" data/1]
		]
	]

	on-error-packet: func[
		port [port!]
		/local pl
	][
		pl: port/locals
		parse next port/data case [
			pl/capabilities and defs/client/protocol-41 [
				[
					read-int 	(pl/error-code: int)
					6 skip
					read-string (pl/error-msg: string)
				]
			]
			any [none? pl/protocol pl/protocol > 9][
				[
					read-int 	(pl/error-code: int)
					read-string (pl/error-msg: string)
				]
			]
			true [
				pl/error-code: 0
				[read-string (pl/error-msg: string)]
			]
		]
		cause-error 'mysql 'message reduce [pl/error-code pl/error-msg]
	]

	parse-a-packet: func [
		port [port!]
		/local pl status rules
	][
		pl: port/locals

		sys/log/debug 'MySQL["parsing a packet:" mold port/data]
		pl/last-status: status: to integer! port/data/1
		pl/error-code: pl/error-msg: none

		switch status [
			255 [
				pl/state: 'idle
				on-error-packet port
				return 'ERR
			]
			254 [
				if pl/packet-len < 9 [
					if pl/packet-len = 5 [
						parse/all/case next port/data [
							read-int	(pl/current-result/warnings: int)
							read-int	(pl/more-results?: not zero? int and 8)
						]
					]
					return 'EOF
				]
			]
			0 [
				if none? pl/expecting [
					rules: copy [
						read-length	(pl/current-result/affected-rows: len)
						read-length (pl/current-result/last-insert-id: len)
						read-int	(pl/more-results?: not zero? int and 8)
						read-int	(pl/current-result/server-status: int)
					]
					if pl/capabilities and defs/client/protocol-41 [
						append rules [
							read-int (pl/current-result/warnings: int)
						]
					]
					; ignore session track info
					parse/all/case next port/data rules
				]
				return 'OK
			]
			251 [; #fb
				return 'FB
			]
		]
		return 'OTHER
	]

	start-next-cmd: func [
		port [port!]
		/local pl qry
	][
		pl: port/locals
		either empty? pl/o-buf [
			pl/state: 'idle
		][
			qry: take pl/o-buf
			do-tcp-insert port qry/1 qry/2
		]
	]

	emit-event: func [
		port
		evt-type
		/local mysql-port pl
	][
		pl: port/locals
		mysql-port: pl/mysql-port
		sys/log/more 'MySQL ["Event:^[[22m" evt-type]
		case compose [
			(evt-type = 'read) [
				sys/log/more 'MySQL["emitting result (" length? pl/results ")"]
				mysql-port/data: convert-results port pl/results pl/result-options
				append system/ports/system make event! [type: evt-type port: mysql-port]
			]
			(any [
					evt-type = 'wrote
					evt-type = 'connect
					evt-type = 'close
				])[
				append system/ports/system make event! [type: evt-type port: mysql-port]
			]
		][
			cause-error 'user 'message rejoin ["Unsupported event: " type]
		]
	]

	process-a-packet: func [
		port [port!]
		buf [binary!] "the packet buffer"
		/local
		pl
		col
		row
		mysql-port
		pkt-type
		blob-to-text
		text-type
		infile-data
	][
		pl: port/locals
		mysql-port: pl/mysql-port
		sys/log/more 'MySQL["processing a packet in state:" pl/state]
		sys/log/debug 'MySQL["buf:" mold buf]
		switch pl/state [
			reading-greeting [
				process-greeting-packet port buf
				send-packet port pack-auth-packet port
				pl/state: 'sending-auth-pack
				sys/log/more 'MySQL["state changed to sending-auth-pack"]
				pl/stream-end?: true ;wait for a WROTE event
			]

			reading-auth-resp [
				either all [1 = length? buf buf/1 = #"^(FE)"][
					;switch to old password mode
					send-packet port write-string scramble/v10 port/pass port
					pl/state: 'sending-old-auth-pack
					sys/log/more 'MySQL["state changed to sending-old-auth-pack"]
				][
					if buf/1 = 255 [;error packet
						pl/state: 'init
						on-error-packet port
					]
					sys/log/more 'MySQL "handshaked"
					;OK?
					emit-event port 'connect
					;sys/log/more 'MySQL["o-buf after auth resp:" mold port/locals/o-buf]
					start-next-cmd port
				]
				pl/stream-end?: true ;done or wait for a WROTE event
			]

			reading-old-auth-resp [
				if buf/1 = #"^(00)"[
					emit-event port 'connect

					;sys/log/more 'MySQL["o-buf after old auth resp:" mold port/locals/o-buf]
					start-next-cmd port
				]
				pl/stream-end?: true ;done or wait for a WROTE event
			]

			reading-cmd-resp [;reading a response
				;check if we've got enough data
				sys/log/more 'MySQL["read a cmd response for" pl/current-cmd]
				switch/default parse-a-packet port [
					OTHER [
						case [
							any [pl/current-cmd = defs/cmd/query
								pl/current-cmd = defs/cmd/field-list][
								parse/all/case buf [
									read-length (if zero? pl/current-result/n-columns: len [
											pl/stream-end?: true 
											sys/log/more 'MySQL["stream ended because of 0 columns"]
										]
										sys/log/more 'MySQL["Read number of columns:" len]
									)
								]
								pl/state: 'reading-fields
							]
							'else [
								cause-error 'user 'message reduce ["Unexpected response" pl]
							]
						]
					]
					OK [;other cmd response
						pl/stream-end?: not pl/more-results?
						pl/current-result/query-finish-time: now/precise
						append pl/results pl/current-result
						either pl/stream-end? [
							emit-event port 'read
							sys/log/more 'MySQL["Stream ended by an OK packet"]
							start-next-cmd port
							exit
						][
							pl/current-result: make result-class [
								query-start-time: pl/current-result/query-start-time
							]
						]
					]
					FB [;LOCAL INFILE request
						unless parse next buf [
							read-string (file-name: string)
						][
							cause-error 'user 'message reduce ["unrecognized LOCAL INFILE request:" buf]
						]

						if error? err: try [
							infile-data: read to file! file-name
						][
							pl/stream-end?: true
							do err
						]

						write port join
							pack-packet port write-string infile-data
							pack-packet port #{} ;last empty packet to end it
						pl/stream-end?: true
						pl/state: 'sending-infile-data
					]
				][
					cause-error 'user 'message reduce ["Unexpected number of fields" pl]
				]
				sys/log/more 'MySQL["stream-end? after reading-cmd-resp:" pl/stream-end?]
			]

			reading-fields [
				pkt-type: 'OTHER
				if 0 != to integer! first buf [; string with a length of 0, will be confused as an OK packet
					pkt-type: parse-a-packet port
				]
				switch/default pkt-type [
					OTHER [
						col: make column-class []
						either pl/capabilities and defs/client/protocol-41 [
							parse/all/case buf [
								read-field 	(col/catalog: to string! field)
								read-field 	(col/db: to string! field)
								read-field	(col/table: to string!	field)
								read-field	(col/org_table: to string!	field)
								read-field	(col/name: to string! field)
								read-field	(col/org_name: to string! field)
								read-byte	;filler
								read-int	(col/charsetnr: int)
								read-long	(col/length: long)
								read-byte	(col/type: decode/type byte)
								read-int	(col/flags: decode/flags int)
								read-byte	(col/decimals: byte)
								read-int	;filler, always 0
								read-length	(col/default: len)
							]
						][
							parse/all/case buf [
								read-field	(col/table:	to string! field)
								read-field	(col/name: 	to string! field)
								read-length	(col/length: len)
								read-length	(col/type: decode/type len)
								read-length	(col/flags: decode/flags len)
								read-byte	(col/decimals: byte)
							]
						]
						blob-to-text: [blob text tinyblob tinytext mediumblob mediumtext longblob longtext]
						unless none? text-type: select blob-to-text col/type [
							unless find col/flags 'binary [
								col/type: text-type
							]
						]
						if none? pl/current-result/columns [
							pl/current-result/columns: make block! pl/current-result/n-columns
						]
						;sys/log/more 'MySQL["read a column: " col/name]
						append pl/current-result/columns :col
						pl/state: 'reading-fields
					]
					EOF [
						case compose [
							(pl/current-cmd = defs/cmd/query) [
								pl/state: 'reading-rows
							]
							(pl/current-cmd = defs/cmd/field-list) [
								sys/log/more 'MySQL["result ended for field-list"]
								pl/current-result/query-finish-time: now/precise
								append pl/results pl/current-result
								pl/stream-end?: not pl/more-results?
								either pl/stream-end? [
									emit-event port 'read
									start-next-cmd port
									exit
								][
									;prepare for next result set
									pl/current-result: make result-class [
										query-start-time: pl/current-result/query-start-time
									]
								]
							]
							'else [
								cause-error 'user 'message reduce ["unexpected EOF" pl]
							]
						]
					]
				][
					cause-error 'user 'message reduce ["Unexpected fields" pl]
				]
			]

			reading-rows [
				pkt-type: 'OTHER
				if 0 != to integer! first buf [; string with a length of 0, will be confused as an OK packet
					pkt-type: parse-a-packet port
				]
				switch/default pkt-type [
					OTHER FB[
						row: make block! pl/current-result/n-columns
						;sys/log/more 'MySQL["row buf:" copy/part buf pl/next-packet-length]
						parse/all/case buf [pl/current-result/n-columns [read-field (append row field)]]
						;sys/log/more 'MySQL["row:" mold row]
						if none? pl/current-result/rows [
							pl/current-result/rows: make block! 10
						]
						either pl/result-options/flat? [
							insert* tail pl/current-result/rows row
						][
							insert*/only tail pl/current-result/rows row
						]
						pl/state: 'reading-rows
					]
					; TODO: an OK packet can be sent here if CLIENT_DEPRECATE_EOF was set for MySQL > 5.7
					EOF [
						if pl/result-options/auto-conv? [convert-types port pl/current-result/rows]
						if pl/result-options/newlines? [
							either pl/result-options/flat? [
								new-line/skip pl/current-result/rows true pl/current-result/n-columns
							][
								new-line/all pl/current-result/rows true
							]
						]
						pl/current-result/query-finish-time: now/precise
						sys/log/more 'MySQL["Length of rows in current result:" length? pl/current-result/rows]
						append pl/results pl/current-result
						;sys/log/more 'MySQL["results length: " length? pl/results]
						either pl/more-results? [
							pl/stream-end?: false
							pl/state: 'reading-cmd-resp
							pl/current-result: make result-class [query-start-time: pl/query-start-time] ;get ready for next result set
						][
							pl/stream-end?: true
							sys/log/more 'MySQL["Emitting read event because of no more results"]
							emit-event port 'read
							;sys/log/more 'MySQL["o-buf after reading query resp:" mold port/locals/o-buf]
							start-next-cmd port
							exit
						]
					]
				][
					;sys/log/more 'MySQL["unexpected row" mold pl]
					cause-error 'user 'message reduce ["Unexpected row" pl]
				]
				;sys/log/more 'MySQL["stream-end? after reading-rows:" pl/stream-end?]
			]

			idle [
				sys/log/more 'MySQL["unprocessed message from server" tcp-port/data]
				break
			]
		][
			cause-error 'user 'message rejoin [rejoin ["never be here in read" pl/state]]
		]
	]

	tcp-port-param: none
	process-greeting-packet: func [
		port [port!]
		data [binary!]
		/local pl tcp-port feature-supported?
	][
		sys/log/more 'MySQL["processing a greeting packet"]
		tcp-port: port
		pl: port/locals
		if data/1 = 255 [;error packet
			parse/all skip data 1 [
				read-int 	(pl/error-code: int)
				read-string (pl/error-msg: string)
			]
			cause-error 'Access 'message reduce [pl/error-code pl/error-msg]
		]
		parse/all data [
			read-byte 	(pl/protocol: byte)
			read-string (pl/version: string)
			read-long 	(pl/thread-id: long)
			read-bin-string	(pl/crypt-seed: string)
			read-int	(pl/capabilities: int)
			read-byte	(pl/character-set: byte)
			read-int	(pl/server-status: int) 
			read-int	(pl/capabilities: (shift int 16) or pl/capabilities)
			read-byte	(pl/seed-length: byte)
			10 skip		; reserved for future use
			read-bin-string	(
				if string [
					pl/crypt-seed: join copy* pl/crypt-seed string
					pl/auth-v11: yes
				]
			)
			to end		; skipping data for pre4.1.x protocols
		]

		if pl/protocol = -1 [
			close* tcp-port
			cause-error 'user 'message ["Server configuration denies access to locals source^/Port closed!"]
		]

		;show-server pl

		feature-supported?: func ['feature] [
			(select defs/client feature) and pl/capabilities
		]

		tcp-port-param: defs/client/found-rows or defs/client/connect-with-db
		tcp-port-param: either pl/capabilities and defs/client/protocol-41 [
			tcp-port-param 
			or defs/client/long-password 
			or defs/client/transactions 
			or defs/client/protocol-41
			or defs/client/secure-connection
			or defs/client/multi-queries
			or defs/client/multi-results
			or defs/client/local-files
		][
			tcp-port-param and complement defs/client/long-password
		]
	]

	pack-auth-packet: func [
		port [port!]
		/local pl auth-pack path key
	][
		pl: port/locals
		path: to binary! skip port/spec/path 1
		auth-pack: either pl/capabilities and defs/client/protocol-41 [
			rejoin [
				write-long tcp-port-param
				;write-long (length? port/user) + (length? port/pass)
				;	+ 7 + std-header-length
				write-long to integer! #1000000 ;max packet length, the value 16M is from mysql.exe
				write-byte pl/character-set
				{^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@^@} ;23 0's
				write-string any [port/spec/user ""]
				write-byte length? key: scramble port/spec/pass port
				key
				write-string any [path "^@"]
			]
		][
			rejoin [
				write-int tcp-port-param
				write-int24 (length? port/spec/user) + (length? port/spec/pass)
					+ 7 + std-header-length
				write-string any [port/spec/user ""]
				write-string key: scramble port/pass port
				write-string any [path ""]
			]
		]

		;sys/log/more 'MySQL["auth-pack:" mold auth-pack]
		auth-pack
	]
	
;------ Public interface ------
	clear-data: func [ port ][
		clear port/data
		;port/locals/tcp-port/spec/response: make binary! 0
	]

	tcp-awake: func [event
		/local 
		tcp-port
		mysql-port
		pl
		packet-len
	][
		tcp-port: event/port
		mysql-port: tcp-port/locals/mysql-port
		pl: tcp-port/locals
		pl/last-activity: now/precise

		sys/log/more 'MySQL["tcp event:" event/type]
		;sys/log/more 'MySQL["o-buf:" mold tcp-port/locals/o-buf]
		;;sys/log/more 'MySQL["locals:" mold tcp-port/locals]
		;pl/exit-wait?: false
		switch event/type [
			error [
				cause-error 'Access 'read-error reduce [event/port "unknown" event]
				return true
			]
			lookup [
				;;sys/log/more 'MySQL"loop up"
				open tcp-port
			]
			connect [
				;;sys/log/more 'MySQL"connect"
				read tcp-port ;greeting from server
				pl/packet-state: 'reading-packet-head
				pl/next-packet-length: std-header-length
				pl/state: 'reading-greeting
				pl/stream-end?: false
			]
			read [
				;;sys/log/more 'MySQL"read event"
				sys/log/debug 'MySQL["buffer:" mold tcp-port/data]
				sys/log/more 'MySQL["current buffer length:" length? tcp-port/data]
				;sys/log/more 'MySQL["buffer with data:" mold tcp-port/data]
				while [true] [
					sys/log/more 'MySQL["next-len:" pl/next-packet-length ", buf: " length? tcp-port/data]
					either pl/next-packet-length > length? tcp-port/data [; not enough data
						read tcp-port
						;sys/log/more 'MySQL["keep reading"]
						break
					][
						;sys/log/more 'MySQL["current state:" pl/state]
						switch/default pl/packet-state [
							reading-packet-head [
								;sys/log/more 'MySQL["read a packet head" mold copy/part tcp-port/data std-header-length]
								parse/all tcp-port/data [
									read-int24  (pl/packet-len: int24)
									read-byte	(pl/seq-num: byte)
								]
								pl/packet-state: 'reading-packet
								pl/next-packet-length: pl/packet-len
								;sys/log/more 'MySQL["expected length of next packet:" pl/next-packet-length]
								remove/part tcp-port/data std-header-length
							]
							reading-packet [
								;sys/log/more 'MySQL["read a packet"]
								either pl/packet-len < 16777215 [; a complete packet is read
									sys/log/more 'MySQL["a COMPLETE packet is received"]
									either pl/data-in-buf? [
										append/part pl/buf tcp-port/data pl/packet-len
										process-a-packet tcp-port pl/buf
										clear pl/buf
										pl/data-in-buf?: false
									][
										process-a-packet tcp-port tcp-port/data ;adjust `state' for next step
									]
								][  ; part of a big packet
									sys/log/more 'MySQL["a CHUNK of a packet is received"]
									pl/data-in-buf?: true
									append/part pl/buf tcp-port/data pl/packet-len
								]
								pl/packet-state: 'reading-packet-head
								pl/next-packet-length: std-header-length
								remove/part tcp-port/data pl/packet-len
								if pl/stream-end? [
									sys/log/more 'MySQL["stream ended, exiting"]
									break
								]
							]
						][
							cause-error 'user 'message reduce [rejoin ["should never be here: read " pl/state]]
						]
					]
				]
			]
			wrote[
				switch/default pl/state [
					sending-cmd [
						pl/state: 'reading-cmd-resp
						emit-event tcp-port 'wrote
						;get ready for result
						pl/current-result: make result-class [query-start-time: pl/query-start-time]
						pl/results: make block! 1
						pl/more-results?: false
						pl/stream-end?: false
						unless all [
							empty? pl/results
							empty? pl/current-result/rows
						][
							cause-error 'user 'message ["rows is not properly initialized"]
						]
						;sys/log/more 'MySQL["result is properly initialized"]
					]
					sending-auth-pack [
						pl/state: 'reading-auth-resp
					]
					sending-old-auth-pack [
						pl/state: 'reading-old-auth-resp
					]
					sending-infile-data [
						pl/state: 'reading-cmd-resp ;an OK packet is expected
						pl/stream-end?: false
					]
				][
					cause-error 'user 'message reduce [rejoin ["never be here in wrote " pl/state]]
				]

				read tcp-port
				pl/packet-state: 'reading-packet-head
				pl/next-packet-length: std-header-length
				;;sys/log/more 'MySQL"write event"
				;send more request
				;return true
			]
			close [
				;close mysql-port
				sys/log/more 'MySQL["TCP port closed"]
				close tcp-port
				emit-event tcp-port 'close
				return true
			]
		]
		false
	];-----------end awake --------------

	tcp-insert: func [
		"write/cache the data"
		port [port!]
		data [string! block!]
		options [object!]
	][
		;;sys/log/more 'MySQL["inserting to " mold port]
		;sys/log/more 'MySQL["state:" port/locals/state]
		either 'idle = port/locals/state [
			do-tcp-insert port data options
		][
			append/only port/locals/o-buf reduce [data options]
		]
		;sys/log/more 'MySQL[port/spec/scheme "o-buf:" mold port/locals/o-buf]
	]

	do-tcp-insert: func [
		"actually write the data to the tcp port"
		port [port!]
		data [string! block!]
		options [object!]
		/local pl res
	][
		pl: port/locals
		;sys/log/more 'MySQL["do-tcp-insert" mold data]
		
		pl/result-options: options
		pl/query-start-time: now/precise
		if all [string? data data/1 = #"["][data: load data]
		res: either block? data [
			if empty? data [cause-error 'user 'message ["No data!"]]
			either string? data/1 [
				insert-query port mysql-map-rebol-values data
			][
				insert-cmd port data
			]
		][
			either port/locals/capabilities and defs/client/protocol-41[
				insert-query port data
			][
				insert-all-queries port data ;FIXME: not tested
			]
		]
		res
	]; end insert function


	open-tcp-port: func [
		port [port!] "mysql port"
		/local conn
	][
		conn: make port![
			scheme: 'tcp
			host: port/spec/host
			port-id: port/spec/port-id
			ref: rejoin [tcp:// host ":" port-id port/spec/path]
			user: port/spec/user
			pass: port/spec/pass
			path: port/spec/path
			timeout: port/spec/timeout
		]

		conn/locals: make locals-class [
			state: 'init
			mysql-port: port
			o-buf: make block! 10
			;o-buf: copy [none]
			conv-list: copy conv-model
			current-result: make result-class []
			result-options: make result-option-class []
		]
		conn/awake: :tcp-awake
		open conn
		conn
	]

	convert-results: func [
		"convert the results to old format for compatibility"
		port [port!] "tcp port"
		results [block!]
		opts [object!]
		/local ret tmp name-fields r
	][
		;sys/log/more 'MySQL["converting results:" mold results]
		either any [
			port/locals/current-cmd != defs/cmd/query
		][;results from multiple queries
			return results
		][
			r: make block! length? results
			foreach ret results [
				unless opts/verbose? [
					either opts/named? [
						name-fields: func [
							rows [block!]
							columns [block!]
							/local tmp n
						][
							tmp: make block! 2 * length? rows
							repeat n length? columns [
								append tmp columns/:n/name
								append tmp rows/:n
							]
							tmp
						]
						either opts/flat? [
							if ret/n-columns < length? ret/rows [
								cause-error 'user 'message ["/flat and /name-fields can't be used for this case, because of multiple rows"]
							]
							ret: name-fields ret/rows ret/columns
						][
							rows: ret/rows
							forall rows [
								change/only rows name-fields first rows ret/columns
							]
							ret: rows
						]
					][
						ret: ret/rows
					]
				]
				append/only r ret
			]
			return either 1 = length? r [r/1][r]
		]
	]

	
	extend system/catalog/errors 'MySQL make object! [
		code: 1000
		type: "MySQL-errors"
		message: ["[" :arg1 "]" :arg2] ;arg1: [error code] ;arg2: error message
	]
]

sys/make-scheme [
	name: 'mysql
	title: "MySQL Driver"
	
	spec: make system/standard/port-spec-net [
		path: %""
		port-id: 3306
		timeout: 120
		user:
		pass: none
	]
	
	info: make system/standard/file-info [
	]
		
	
	awake: func [
		event [event!]
		/local pl cb mode spec
	][
		sys/log/more 'MySQL["mysql port event:" event/type]
		pl: event/port/locals
		pl/last-activity: now/precise
		switch/default event/type [
			connect [
				; remove stored password and rebuild the reference
				; so password is not visible in logs
				spec: event/port/spec
				spec/pass: none
				spec/ref: rejoin [
					mysql:// spec/user #"@" spec/host #":" spec/port-id spec/path
				]
				sys/log/info 'MySQL ["Connected:^[[22m" spec/ref]
				pl/handshaked?: true
				if pl/exit-wait-after-handshaked? [return true]
			]
			read [
				sys/log/more 'MySQL["pending requests:" mold pl/pending-requests "block size:" pl/pending-block-size]
				mode: first pl/pending-requests
				switch/default mode [
					async [
						cb: second pl/pending-requests
						case [
							function? :cb [
								;sys/log/more 'MySQL["a function callback:" mold :cb]
								cb event/port/data
							]
							any [word? cb path? cb][
								sys/log/more 'MySQL["a word/path callback:" mold cb]
								set cb event/port/data
							]
							block? cb [
								;sys/log/more 'MySQL["a block callback:" mold cb]
								do cb
							]
							none? cb [
								sys/log/more 'MySQL["a none callback, ignored"]
								;ignored
							]
							'else [
								cause-error 'user 'message reduce [rejoin ["unsupported callback:" mold cb]]
							]
						]
						remove/part pl/pending-requests pl/pending-block-size
					]
					sync [
						sys/log/more 'MySQL["got response (" length? event/port/data ")"]
						;sys/log/more 'MySQL["sync mode should exit"]
						remove/part pl/pending-requests pl/pending-block-size
						return true
					]
				][
					cause-error 'user 'message reduce [rejoin ["unsupported query mode: " mold mode]]
				]
			]
			wrote [
				;sys/log/more 'MySQL["mysql port query sent"]
			]
			close [
				sys/log/more 'MySQL["port closed"]
				cause-error 'Access 'not-connected reduce [event/port none none]
			]
		][
			cause-error 'user 'message reduce [rejoin ["unsupported event type on mysql port:" event/type]]
		]
		false
	]

	actor: [
		; ------------- new open ---------
		open: func [
			port[port! url!]
		][	
			;;sys/log/more 'MySQL" new open function "
			if none? port/spec/host [http-error "Missing host address"]
			port/locals: make object! [
				handshaked?: false
				exit-wait-after-handshaked?: false
				pending-requests: copy []
				pending-block-size: 2
				last-activity: now/precise
				async?: false
				tcp-port: mysql-driver/open-tcp-port port
			]
			
			port/awake: :awake
			return port
		]
		
		open?: func [port [port!]][
			all [open? port/locals/tcp-port port/locals/handshaked?]
		]

		close: func [
			port [port!]
			/local tcp-port
		][
			tcp-port: port/locals/tcp-port
			tcp-port/spec/timeout: 4
			if open? tcp-port [
				try [;allow this to fail, so the port will always be closed
					mysql-driver/send-cmd tcp-port mysql-driver/defs/cmd/quit []
				]
			]
			close tcp-port
			sys/log/info 'MySQL ["Closed:^[[22m" port/spec/ref]
			tcp-port/awake: none
			port/state: none
			port
		]

		insert: func [
			port [port!]
			data [block!] "hackish: if the first element in the block is an object, then it's an option object"
			/local tcp-port options pl query
		][
			pl: port/locals
			tcp-port: pl/tcp-port
			options: data/1
			either object? options [
				either all [logic? :options/async? not :options/async?][
					append pl/pending-requests reduce ['sync none]
				][
					if options/named? [
						cause-error 'user 'message ["/named can't be used with /async"]
					]
					append pl/pending-requests reduce ['async :options/async?]
				]
				query: data/2
			][
				append pl/pending-requests reduce ['sync none]
				query: data
			]
			sys/log/debug 'MySQL["inserting a query:" mold data mold pl/pending-requests]
			mysql-driver/tcp-insert tcp-port query options
			;sys/log/debug 'MySQL["tcp-port locals after insert" mold tcp-port/locals]
		]
		
		copy: func [
			port[port!]
			/part data [integer!]
		][
			either part [
				copy/part port/data data
			][
				copy port/data
			]
		]
	]; end actor
		
]; end sys/make-scheme

send-sql: func [

	port [port!]
	data [string! block!]
	/flat "return a flatten block"
	/raw "do not do type conversion"
	/named
	/async cb [word! path! function! block! none!] "call send-sql asynchronously: set result to word, call function with the result or evaluate the block"
	/verbose "return detailed info"
	/local result pl old-handshaked? ret-from-wait
][
	pl: port/locals

	unless any [async open? port] [
		cause-error 'Access 'not-connected reduce [port none none]
	]

	data: reduce [
		make mysql-driver/result-option-class [
			flat?:      flat
			auto-conv?: not raw
			named?:     named
			verbose?:   verbose
			async?: either async [:cb][off]
		]
		data
	]

	insert port data
	;sys/log/more 'MySQL["send-sql: " mold data]
	;sys/log/more 'MySQL["in send-sql, current pending requests:" mold pl/pending-requests]
	unless async [
		;sys/log/more 'MySQL["handshaked?:" pl/handshaked?]
		old-handshaked?: pl/handshaked?
		while [pl/last-activity + port/spec/timeout >= now/precise][
			;will not return unless: 1) handshaked, 2) sync request processed, or 3) error
			ret-from-wait: wait/only [port port/locals/tcp-port port/spec/timeout]
			either port = ret-from-wait [
				;assert [empty? pl/pending-requests]
				;sys/log/more 'MySQL["port/data:" mold port/data]
				return port/data
			][
				if port? ret-from-wait [
					assert [ret-from-wait = port/locals/tcp-port]
					print ["******* Unexpected wakeup from tcp-port *********"]
				]
				;sys/log/more 'MySQL"wait returned none"
				throw-timeout port
			]
			;sys/log/more 'MySQL["trying again..."]
		]
		throw-timeout port
	]
]

throw-timeout: func[port [port!]] [
	cause-error 'Access 'timeout to url! rejoin [port/spec/scheme "://" port/spec/host #":" port/spec/port-id]
]

connect-sql: func [
	"Opens connection to MySQL port (waits for a handshake)"
	port [port! url!] "MySQL port or url"
	/local p
][
	if any [url? port not open? port][ port: open port ]
	port/locals/exit-wait-after-handshaked?: true
	p: wait/only [port port/locals/tcp-port port/spec/timeout]
	if port? p [return port]
	throw-timeout port
]

