Rebol [
	Title:  "WebDriver (chrome) scheme"
	Type:    module
	Name:    webdriver
	Date:    26-Dec-2024
	Version: 0.2.1
	Author:  @Oldes
	Home:    https://github.com/Oldes/Rebol-WebDriver
	Rights:  http://opensource.org/licenses/Apache-2.0
	Purpose: {Can be used to automate browser sessions.}
	History: [
		03-Jan-2024 "Oldes" {Initial version}
		25-Dec-2024 "Oldes" {Improvements... WIP}
	]
	Needs: [
		3.11.0 ;; Minimal Rebol version required by WebScocket module
		websocket
		json
	]
	Notes: {
		Currently only `chrome` scheme is implemented which is supposed to be working
		with Chromium, Chrome and other Blink-based browsers.

		The browser must be started with `remote-debugging` enabled.

		For example on macOS using a Brave browser:
		```terminal
		/Applications/Brave\ Browser.app/Contents/MacOS/Brave\ Browser --remote-debugging-port=9222
		```

		It is also possible to run the browser in a Docker:
		```terminal
		docker container run -d -p 9222:9222 zenika/alpine-chrome --no-sandbox --remote-debugging-address=0.0.0.0 --remote-debugging-port=9222
		```

		Available methods are documented here: https://chromedevtools.github.io/devtools-protocol/
		Or once running browser with the debugging enabled: http://localhost:9222/json/protocol
	}
]

system/options/log/chrome: 1

;; internal functions....
read-and-wait: function[
	"Wait specified time while processing read events"
	port [port!] "Internal websocket port of the webdrive scheme"
	time [time!]
][
	start: now/precise
	end: start + time
	until [
		read port
		if port? wait [port time][
			process-packets port
		]
		time: difference end now/precise
		any [time <= 0 not open? port]
	]
]

process-packets: function[
	"Process incomming webscocket packets of the webdrive scheme"
	conn [port!]
][
	;print "--- process-packets"
	port: conn/parent ;; outter webdrive scheme
	ctx:  port/extra
	foreach packet conn/data [
		;? packet
		try/with [
			packet: decode 'json packet
			either all [packet/id any [packet/result packet/error]] [
				;- command response
				;? packet
				sys/log/info 'CHROME ["Received response:" as-yellow packet/id]
				if packet/error [
					sys/log/error 'CHROME [as-red packet/error/code as-purple packet/error/message]
				]
				;sys/log/info 'CHROME [as-red packet/method mold packet/params]
				if ctx/wait-for == packet/id [
					ctx/wait-for: none
				]
				;; Keep all responses in the context (for possible future use)
				repend ctx/responses [
					ctx/req/method
					packet
				]
				append port/data packet
			][
				;- event notification
				repend ctx/events [
					packet/method
					packet/params
				]
				port/actor/on-method packet
				if ctx/wait-for == packet/method [
					;print "SHOULD AWAKE!"
					ctx/wait-for: none
				]
				if packet/method == "Inspector.detached" [
					sys/log/info 'CHROME ["Closing connection:" as-red packet/params/reason]
					close ctx/page-conn
					ctx/page-conn: none
					clear ctx/command-que
					break
				]
			]
		] :print
	]
	clear conn/data
]

page-awake: func[event /local port ctx][
	;print ["--------------------- page-awake" event/type]
	port: event/port
	ctx: port/extra
	switch event/type [
		wrote [
			read port
			return false
		]
		read [
			process-packets ctx/page-conn
			if ctx/wait-for [
				read port
				return false
			] 
		]
	]
	true
]

ws-decode: :codecs/ws/decode

parse-commands: function[ctx data][
	que: ctx/command-que
	parse data [some [
		set url: url! (
			append/only que compose/deep [Page.navigate [url: (url)]]
		)
		| opt 'wait set time: [time! | decimal! | integer!] (
			;- Wait some time while processing incomming messages                          
			append/only que reduce ['wait to time! time]
		)
		| 'wait set event: [word! | lit-word!] (
			append/only que reduce ['wait to string! event]
		)
		|
		set method: word! set params: opt [map! | block!] (
			repend/only que [method params]
		)
	]]
	que
]
do-next-command: function[port][
	;print "---do-next-command"
	ctx: port/extra
	cmd: take ctx/command-que
	unless cmd [return none]
	set [method: params:] cmd
	either method == 'wait [
		sys/log/info 'CHROME ["WAIT" as-green params]
		if time? params [
			read-and-wait any [ctx/page-conn ctx/browser] params
			exit
		]
		ctx/wait-for: params
		read conn: ctx/page-conn
	][
		;- Send a command with optional options                                        
		if block? params [params: make map! reduce/no-set params]
		sys/log/info 'CHROME ["Command:" as-red method as-green mold/flat params]
		;; resusing `req` value for all commands as it is just used to form a json anyway
		ctx/req/id: ctx/counter: ctx/counter + 1 ;; each command has an unique id
		ctx/req/method: method
		ctx/req/params: params
		write conn: any [ctx/page-conn ctx/browser] ctx/req

		ctx/wait-for: any [
			select [
				Page.navigate "Page.frameStoppedLoading"
				Page.close    "Inspector.detached"
			] method
			ctx/req/id
		]
	]
	;; don't wake up until received responses for all command requests
	forever [
		if any [
			none? wait [conn 15] ;; timeout
			none? ctx/wait-for   ;; not waiting for any specific response
		][	break]
		read conn ;; keep reading
	]
]

init-session: function[port][
	ctx: port/extra
	clear ctx/command-que
	clear ctx/responses
	clear ctx/events
	;; Open a blank page in the browser.
	sys/log/info 'CHROME "Opening a new blank tab."
	ctx/page-info: decode 'json write ctx/host/json/new [PUT]
	ctx/page-conn: conn: open as url! ctx/page-info/webSocketDebuggerUrl
	conn/parent: port
	unless wait [conn 15] [
		do make error! "Failed to open webSocketDebuggerUrl websocket connection!"
	]
	sys/log/info 'CHROME ["Session initialized:" ctx/page-info/id]
	port/awake: :page-awake
	write port 'Page.enable
	port
]

;- The Chrome scheme ---------------------------------------------------------------
sys/make-scheme [
	name: 'chrome
	title: "Chrome WebDriver API"
	spec: object [title: scheme: ref: host: none port: 9222]

	actor: [
		open: func [port [port!] /local ctx spec conn data][
			spec: port/spec
			spec/host: any [spec/host "localhost"]
			spec/port: any [spec/port 9222]

			port/data: copy [] ;; holds decoded websocket responses
			port/extra: ctx: context [
				host: rejoin [http:// spec/host #":" spec/port]
				version: none
				browser: none
				counter: 0
				req: make map! [id: 0 method: none params: none] ;; used to send a command (to avoid cerating a new map)
				page-info: none ;; holds resolved info from an attached page
				page-conn: none ;; webscocket connection to an attached page
				wait-for: none
				command-que: copy []
				responses: copy []
				events: copy []
			]

			ctx/version: data: try/with [
				decode 'json read ctx/host/json/version
			][
				sys/log/error 'CHROME "Failed to get browser info!"
				sys/log/error 'CHROME system/state/last-error
				return none
			]

			ctx/browser: conn: open as url! data/webSocketDebuggerUrl
			conn/parent: port
			wait [conn 15]
			sys/log/more 'CHROME "Browser connection opened."

			init-session port
		]
		open?: func[port /local ctx][
			all [
				ctx: port/extra
				any [ctx/browser ctx/page-conn]
				true
			]
		]
		close: func[port /local ctx][
			ctx: port/extra
			clear ctx/command-que
			if ctx/page-conn [
				write port 'Page.close
				ctx/page-info: none
			]
			if ctx/browser [
				close ctx/browser
				ctx/browser: none 
			]
			port
		]

		write: func[port data /local ctx url time method params conn pos p][
			unless block? data [data: reduce [data]]

			sys/log/debug 'CHROME ["WRITE:" as-green mold/flat data]

			ctx: port/extra

			clear port/data

			either open? ctx/browser [
				unless ctx/page-conn [
					;print "---------------------------"
					init-session port
				]
				parse-commands ctx data
				while [not empty? ctx/command-que][
					do-next-command port
				]
				either 1 = length? port/data [first port/data][port/data]
			][ sys/log/error 'CHROME "Not open!"]  
		]

		read: func[port /local ctx conn packet][
			;; waits for any number of incomming messagesto
			if all [
				ctx: port/extra
				conn: any [ctx/page-conn ctx/browser] 
			][
				read conn
				;wait [conn 1] ;; don't wait more then 1 second if there are no incomming messages
				;process-packets conn
			]
			port/data
		]

		pick: func[port value /local result][
			;; just a shortcut to get a single result direcly
			all [
				object? port/extra
				select/last port/extra/responses :value
			]
		]

		on-method: func[packet /local verbose][
			;; this function is supposed to be user defined and used to process incomming messages
			;; in this case it just prints its content...
			verbose: system/options/log/chrome
			case [
				verbose > 1 [
					sys/log/debug 'CHROME ["Event:" as-yellow packet/method mold packet/params]
				]
				verbose > 0 [
					sys/log/info 'CHROME ["Event:" as-yellow packet/method]
				]
			]
		]
	]
]

