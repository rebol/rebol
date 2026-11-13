Rebol [
	Name:    daytime
	Type:    module
	Options: [delay]
	Version: 1.0.0
	Date:    20-Jul-2022
	File:    %prot-daytime.reb
	Title:   "Daytime protocol scheme"
	Author:  @Oldes
	Rights:  "Copyright (C) 2022 Oldes. All rights reserved."
	License: MIT
	History: [
		1.0.0 20-Jul-2022 @Oldes "Initial version"
	]
	Notes: {
		https://www.nist.gov/pml/time-and-frequency-division/time-distribution/internet-time-service-its
	}
	Usage: [
		;; for default time.nist.gov
		read daytime://
		;; or custom: 
		read daytime://utcnist.colorado.edu
	]
]

system/options/log/daytime: 1
daytime-sync-awake: func[event /local port type][
	port: event/port
	type: event/type
	log-trace 'DAYTIME ["Sync-Awake event:" type]
	switch type [
		lookup  [ open port return false]
		connect [ read port return false]
	]
	true
]
parse-NIST: func[
	data [string!]
	/local JJJJJ YR MO DA HH MM SS TT L H msADV numeric
][
	numeric: :system/catalog/bitsets/numeric
	;JJJJJ YR-MO-DA HH:MM:SS TT L H msADV UTC(NIST) OTM
	all [
		parse data [
			any whitespace
			copy JJJJJ: some numeric SP
			;; JJJJJ is the Modified Julian Date (MJD). 
			;; The MJD has a starting point of midnight on November 17, 1858. You can obtain the MJD 
			;; by subtracting exactly 2 400 000.5 days from the Julian Date, which is an integer day 
			;; number obtained by counting days from the starting point of noon on 1 January 4713 B.C.
			;; (Julian Day zero).
			copy YR: 2 numeric #"-"
			copy MO: 2 numeric #"-"
			copy DA: 2 numeric SP
			;; YR-MO-DA is the date.
			;; It shows the last two digits of the year, the month, and the current day of month.
			copy HH: 2 numeric #":"
			copy MM: 2 numeric #":"
			copy SS: 2 numeric SP
			;; HH:MM:SS is the time in hours, minutes, and seconds. 
			;; The time is always sent as Coordinated Universal Time (UTC).
			copy TT: 2 numeric SP
			;; TT is a two digit code (00 to 99) that indicates whether the United States is on 
			;; Standard Time (ST) or Daylight Saving Time (DST). It also indicates when ST or 
			;; DST is approaching. This code is set to 00 when ST is in effect, or to 50 when DST 
			;; is in effect. During the month in which the time change actually occurs, this number 
			;; will decrement every day until the change occurs. For example, during the month of November, 
			;; the U.S. changes from DST to ST. On November 1, the number will change from 50 to the actual 
			;; number of days until the time change. It will decrement by 1 every day until the change 
			;; occurs at 2 a.m. local time when the value is 1. Likewise, the spring change is at 2 a.m. 
			;; local time when the value reaches 51.
			copy  L: 1 numeric SP
			;; L is a one-digit code that indicates whether a leap second will be added or subtracted 
			;; at midnight on the last day of the current month. If the code is 0, no leap second will 
			;; occur this month. If the code is 1, a positive leap second will be added at the end of 
			;; the month. This means that the last minute of the month will contain 61 seconds 
			;; instead of 60. If the code is 2, a second will be deleted on the last day of the month. 
			;; Leap seconds occur at a rate of about one per year. They are used to correct for irregularity 
			;; in the earth's rotation. The correction is made just before midnight UTC (not local time).
			copy  H: 1 numeric SP
			;; H is a health digit that indicates the health of the server. If H = 0, the server is healthy. 
			;; If H = 1, then the server is operating properly but its time may be in error by up to 5 seconds. 
			;; This state should change to fully healthy within 10 minutes. If H = 2, then the server is 
			;; operating properly but its time is known to be wrong by more than 5 seconds. If H = 3, then 
			;; a hardware or software failure has occurred and the amount of the time error is unknown. 
			;; If H = 4 the system is operating in a special maintenance mode and both its accuracy and 
			;; its response time may be degraded. This value is not used for production servers except in 
			;; special circumstances. The transmitted time will still be correct to within ±1 second in this mode.
			copy msADV: [some numeric #"." some numeric] SP
			;; msADV displays the number of milliseconds that NIST advances the time code to partially 
			;; compensate for network delays. The advance is currently set to 50.0 milliseconds.
			"UTC(NIST) *" to end
			;; The label UTC(NIST) is contained in every time code. It indicates that you are receiving 
			;; Coordinated Universal Time (UTC) from the National Institute of Standards and Technology (NIST).
			;; OTM (on-time marker) is an asterisk (*). The time values sent by the time code refer to 
			;; the arrival time of the OTM. In other words, if the time code says it is 12:45:45, 
			;; this means it is 12:45:45 when the OTM arrives.
		]
		reduce [
			to integer! JJJJJ
			to date! ajoin [DA #"-" MO #"-" YR #"/" HH #":" MM #":" SS]
			to integer! TT
			to integer! L
			to integer! H
			to decimal! msADV
		]
	]
]

sys/make-scheme [
	name: 'daytime
	title: "Daytime Protocol (RFC 867)"
	spec: make system/standard/port-spec-net [
		port:    13
		timeout: 15
	]
;	awake: func[event][
;		log-info 'DAYTIME ["Awake event:" type]
;		true
;	]
	actor: [
		open: func [
			port [port!]
			/local spec conn
		][
			if open? port [return port]
			port/extra: open as url! ajoin [
				tcp://
				any [port/spec/host "time.nist.gov"]
				#":" port/spec/port
			]
			port/extra/awake: :daytime-sync-awake
			port/extra
		]
		open?: func [port][
			all [
				port? port/extra
				open? port/extra
			]
		]
		read: func[port][
			open port
			wait [port/extra port/spec/timeout]
			if binary? port/extra/data [
				port/data: trim/tail/head to string! port/extra/data
			]
			log-info 'DAYTIME ["Server:" port/data]
			close port
			all [
				port/data
				parse-NIST port/data
			]
		]
		close: func [port][
			if open? port [return port]
			close port/extra
			port/extra: none
			port
		]
	]
]
