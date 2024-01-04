Rebol [
	Title:  "WebDriver (chrome) scheme"
	Type:    module
	Name:    webdriver
	Date:    03-Jan-2024
	Version: 0.1.0
	Author:  @Oldes
	Home:    https://github.com/Oldes/Rebol-WebDriver
	Rights:  http://opensource.org/licenses/Apache-2.0
	Purpose: {Can be used to automate browser sessions.}
	History: [
		03-Jan-2024 "Oldes" {Initial version}
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

		Available methods are documented here: https://chromedevtools.github.io/devtools-protocol/
	}
]

system/options/log/chrome: 4

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
		wait [port time]

		process-packets port

		time: difference end now/precise
		time <= 0:0:0
	]
]

process-packets: function[
	"Process incomming webscocket packets of the webdrive scheme"
	conn [port!]
][
	port: conn/parent ;; outter webdrive scheme
	ctx:  port/extra
	foreach packet conn/data [
		try/with [
			packet: decode 'json packet
			either packet/id [
				ctx/pending: ctx/pending - 1
				append port/data packet
			][ port/actor/on-method packet ]
		] :print
	]
	clear conn/data
]

ws-decode: :codecs/ws/decode

;- The Chrome scheme ---------------------------------------------------------------
sys/make-scheme [
	name: 'chrome
	title: "Chrome WebDriver API"
	spec: object [title: scheme: ref: host: none port: 9222]

	actor: [
		open: func [port [port!] /local ctx spec host conn data port-spec][
			spec: port/spec
			spec/host: any [spec/host "localhost"]
			spec/port: any [spec/port 9222]

			port/data: copy [] ;; holds decoded websocket responses
			port/extra: ctx: context [
				host: rejoin [http:// spec/host #":" spec/port]
				version: none
				browser: none
				counter: 0
				pending: 0 ;; increments when a new method is sent, decremented when response is received
				req: #(id: 0 method: none params: #[none]) ;; used to send a command (to avoid cerating a new map)
				page-info: none ;; holds resolved info from an attached page
				page-conn: none ;; webscocket connection to an attached page
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
			port
		]
		open?: func[port /local ctx][
			all [
				ctx: port/extra
				any [ctx/browser ctx/page-conn]
			]
		]
		close: func[port /local ctx][
			ctx: port/extra
			if ctx/port-conn [
				try [close ctx/port-conn wait [ctx/page-conn 1]]
				ctx/port-conn: ctx/port-info: none

			]
			if ctx/browser [
				try [close ctx/browser wait [ctx/browser 1]]
				ctx/browser: none 
			]
			port
		]

		write: func[port data /local ctx url time method params conn][
			unless block? data [data: reduce [data]]

			sys/log/info 'CHROME ["WRITE:" as-green mold/flat data]

			clear port/data

			ctx: port/extra
			either open? ctx/browser [
				parse data [some [

					set url: url! (
						;- Open a new target (page)                                                    
						try/with [
							ctx/page-info: decode 'json write join ctx/host/json/new? url [PUT]
							;?? ctx/page-info
							append port/data ctx/page-info

							ctx/page-conn: conn: open as url! ctx/page-info/webSocketDebuggerUrl
							;conn/awake: :ws-web-awake
							conn/parent: port
							wait [conn 15]
							conn: none
						] :print
					)
					| set time: [time! | decimal! | integer!] (
						;- Wait some time while processing incomming messages                          
						time: to time! time
						sys/log/info 'CHROME ["WAIT" as-green time]
						read-and-wait any [ctx/page-conn ctx/browser] time
					)
					|
					set method: word! set params: opt [map! | block!] (
						;- Send a command with optional options                                        
						if block? params [params: make map! reduce/no-set params]
						sys/log/info 'CHROME ["Command:" as-red method as-green mold/flat/part params 100]
						;; resusing `req` value for all commands as it is just used to form a json anyway
						ctx/req/id: ctx/counter: ctx/counter + 1 ;; each command has an unique id
						ctx/req/method: method
						ctx/req/params: params
						ctx/pending: ctx/pending + 1
						write conn: any [ctx/page-conn ctx/browser] ctx/req
						;; don't wake up until received responses for all command requests
						forever [
							;@@TODO: handle the timeout to awoid infinite loop!
							wait [conn 15]              ;; wait for any events
							process-packets conn        ;; process incomming websocket messages
							if ctx/pending <= 0 [break] ;; exit the loop if there are no pending requests
							read conn                   ;; keep reading
						]
					)
				]]
				either 1 = length? port/data [first port/data][port/data]
			][ sys/log/error 'CHROME "Not open!"]  
		]

		read: func[port /local ctx conn packet][
			;; waits for any number of incomming messages
			if all [
				ctx: port/extra
				conn: any [ctx/page-conn ctx/browser] 
			][
				clear port/data
				read conn
				wait [conn 1] ;; don't wait more then 1 second if there are no incomming messages
				process-packets conn
			]
			port/data
		]

		pick: func[port value /local result][
			;; just a shortcut to get a single result direcly
			unless block? value [value: reduce [value]]
			result: write port value
			if block? result [result: last result]
			result/result
		]

		on-method: func[packet][
			;; this function is supposed to be user defined and used to process incomming messages
			;; in this case it just prints its content...
			sys/log/info 'CHROME [as-red packet/method mold packet/params]
		]
	]
]

