Rebol [
	Title:   "TCP Port scanner"
	File:    %port-scanner.reb
	Type:    module
	Name:    port-scanner
	Version: 1.0.0
	Date:    16-Feb-2021
	Exports: [scan-ports]
	Note: {
		If you use anything other than `localhost` you had better have permission
		for the host name you do use, or you will suddenly be an *internet bad guy*.
		Don't.}
]

; result:
found: copy []

; options:
batch-size: 8000 ; higher batch size means higher CPU use!
timeout: 0:0:0.5 ; setting timeout too low may result in not resolved opened ports!

; actor:
on-awake: func [event /local port id] [
	port: event/port
	;print [as-green "==TCP-event:" as-red event/type "port:" as-red port/spec/port-id]
	switch/default event/type [
		lookup  [open port]
		connect [
			id: port/spec/port-id
			print ["Open port found:" as-red id]
			append found id
			false
		]
	][true]
]

scan-ports: function [
	from [integer!] "lowest port numer"
	to   [integer!] "max port number"
	/host name [tuple! string!]
][
	clear found
	checking: make block! batch-size
	started: now/time/precise

	if to < from [id: to to: from from: id]

	id: low: 
	from: max 1 from
	to:   max 1 min to 65535

	total: 1 + to - from

	try/except [
		ip: either tuple? name [ name ][
			read join dns:// any [name 'localhost]
		]
	][
		sys/log/error 'REBOL system/state/last-error
		return none
	]

	print [
		as-green "Using IP:"   as-yellow ip
		as-green "timeout:"    as-yellow timeout
		as-green "batch size:" as-yellow batch-size
		newline
	]

	while [id <= to][
		port: make port! compose [
			scheme:  'tcp
			host:    (ip)
			port-id: (id)
			awake:   :on-awake
		]
		open port
		append checking port
		if any [
			id = to
			batch-size = length? checking
		][
			print [as-green "Checking range:" as-yellow low "-" as-yellow id]
			; wait some time...
			wait timeout
			; and check what was opened...
			foreach port checking [ close port ]
			clear checking
			if id < to [ low: low + batch-size ]
		]
		++ id
	]
	checking: none
	print ajoin [
		as-green "^/Scan of " as-yellow total
		as-green " ports finished in time: " as-yellow now/time/precise - started
		as-green ".^/Found " as-red length? found as-green " listening TCP ports!^/"
	]
	found
]
