REBOL [
	Version:   0.9.1
	Title:     "Scheduler"
	Purpose:   "Task scheduling library with dialect"
	Name:      scheduler
	Type:      module
	Date:      29/04/2024
	Home:      https://github.com/Oldes/Rebol-Scheduler
	Author:    ["Nenad Rakocevic (SOFTINNOV)" @Oldes]
	Needs:     3.12.0
	License:   BSD
	History: [
		26/08/2009 0.9.0 {Original Softinnov's Rebol2 version}
		29/04/2024 0.9.1 {Oldes: ported to Rebol3}
	]
	Copyright: "2009 SOFTINNOV"
	Comments: {

		Scheduler DSL quickstart
		------------------------
		
		Legend:
		- <value> means that the value is optional
		- CAPITALIZED words design dialect's keywords

		o Event with a precise point in time :
			<name:> AT time! DO action

		o Event with a delay :
			<name:> IN n <unit> DO action

		o Recurring event :
			<name:> EVERY 
				<n> <unit>      ; recurring unit
				<allowed>       ; specific point(s) in time or duration(s) allowed
				<NOT forbidden> ; specific point(s) in time or duration(s) forbidden
				<FROM moment>   ; starting point
				<AT moment>     ; fix time for each event (only date changes)
				<t TIMES>       ; limit the number of event occurences
				DO action       ; job to execute
		  with
			<name:>: set-word! value for naming a task (for future access using the API).
			<n>: integer! value for unit multiplying.
			<unit>: any of 
				s|sec|second|seconds
				mn|minute|minutes
				h|hour|hours
				d|day|days
				w|week|weeks
				m|month|months
			<allowed>: any time (00:00:00), calendar day (#dd), weekday (mon|monday), 
					   month (jan|january), range of time|calendar-days or block of any
					   of theses options.
			<forbidden>: same options as <allowed>.
			<moment>: date! or time! value.
			<t>: integer! value.
			action: file!|url!|block!|function!|word! value to be evaluated when
			        event is fired.
			
		Syntactic sugar
		---------------
		Default dialect is parsed in BLOCK! mode. That means that only REBOL values
		are accepted, but some may want to write calendar dates like: 1st, 2nd,...
		instead or #1, #2,...
		
		So, a preprocessor is included allowing tasks to be passed as string! values
		extending the accepted syntax for the following cases :
				1st, 2nd, 3rd,..nth 	: accepted
				12s, 12mn, 12h, 12d,...	: accepted

		Scheduler API
		-------------
		scheduler/plan [spec]      ; add one or more tasks to the scheduler
		scheduler/plan/new [spec]  ; same as above but removes all previous tasks
		scheduler/delete 'name     ; removes the named task from the scheduler
		scheduler/wait             ; provides an adequate global event loop

		Examples
		--------
		scheduler/plan [
			at 18:30 do http://domain.com/update.r3
			every 3 days not [#2 - #9 #12 sat sun] at 00:30 do %batch.r3
			smk: every friday at 13:00 do %test.r3
			cc: every 12 hours do %backup.r3
			every [sat sun] at 12:00 do %beep.r3
			every month on #01 do %check.r3
		]
		scheduler/wait
		
		(See %test-scheduler.r3 for more examples)
	}

]

name: mult: unit: n: allow: forbid: on-day: ts: from: times: job: err:
type: _s: _e: s: e: value: none
	
jobs:  make block! 8
queue: make block! 1
wait-list:  system/state/wait-list

get-now: has [n][n: now/precise n/zone: 0:0 n]

exec: func [spec /local action res][	; TBD: catch errors + log in file
	action: select spec 'action
	switch type?/word action [
		url!      [read action]
		block!    [do action]
		file!     [do action]
		function! [do :action]
		word!     [do get :action]
	]
]

wait: does [
	while [none? lib/wait wait-list][
		on-timer
		if empty? jobs [exit]
	]
]

on-timer: has [task job][
	task: take queue
	job: back find/only jobs task
	task/last: get-now
	exec task
	if any [
		task/at
		all [task/repeat zero? task/repeat: task/repeat - 1]
		none? job/1: next-event? task
	][
		remove/part job 2
	]
	update-sys-timer
]

update-sys-timer: does [
	sort/skip jobs 2
	remove find wait-list time!
	if not empty? jobs [
		append/only queue jobs/2
		;; avoid negatime timeout! https://github.com/Oldes/Rebol-issues/issues/2597
		append wait-list max 0:0 difference jobs/1 get-now
	]
]

reset-series: func [s [series!] len [integer!]][head clear skip s len]

reset-locals: has [list][
	name: mult: unit: n: allow: forbid: on-day: ts: from: times: job: err:
	type: _s: _e: s: e: value: none
]

allowed?: func [spec [block!] time [time!]][
	foreach v spec [
		if any [
			all [block? v v/1 <= time time <= v/2]
			v = time
		][return yes]
	]
	no
]

search-event: func [spec new /short /local tests offset next-new sa sf u list][
	tests: clear []
	sa: spec/allow
	sf: spec/forbid
	u:  spec/unit

	;-- constraints compilation --
	foreach [cond test][
		[sa select sa 'cal-days]   [find sa/cal-days  new/day]
		[sa select sa 'week-days]  [find sa/week-days new/weekday]
		[sa select sa 'months]     [find sa/months    new/month]
		[sa select sa 'time]       [allowed? sa/time  new/time]
		[sf select sf 'cal-days]   [not find sf/cal-days  new/day]
		[sf select sf 'week-days]  [not find sf/week-days new/weekday]
		[sf select sf 'months]     [not find sf/months    new/month]
		[sf select sf 'time]       [not allowed? sf/time  new/time]
	][
		if all cond [append tests test]
	]
	offset: any [spec/multiple 1]
	next-new: either find [day month] u [
		[new/:u: new/:u + offset]
	][
		offset: offset * select [hour 1:0 minute 0:1 second 0:0:1] u
		[new/time: new/time + offset]
	]
	if short [do next-new return new]
	
	;-- evaluation --
	loop select [
		second  60
		minute  60
		hour    24
		day     366		; account for leap years
		month   12
	] u [	
		do next-new				
		if all tests [return new]			
	]
	do make error! rejoin ["can't find next event for rule " mold spec/source]
]

set-datetime: func [src [date! time!] dst [date!]][
	if date? src [
		dst/date: src/date
		if src/time [dst/time: src/time]
		dst/zone: 0:0
	]
	if time? src [dst/time: src]
	dst
]

next-event?: func [spec [block!] /local new][
	if spec/repeat = 0 [return none]
	if spec/at = 'in [
		spec/at: search-event/short spec get-now
	]
	either any [date? spec/at none? spec/unit][
		;-- AT --
		new: get-now
		new: set-datetime spec/at new
	][
		;-- EVERY --
		new: any [spec/last get-now]
		if all [not spec/last spec/from][new: set-datetime spec/from new]
		if spec/at [new/time: spec/at]
		if spec/unit = 'month [
			new/day: any [
				spec/on
				all [date? spec/from spec/from/day]
				1
			]
		]
		new: search-event spec new
	]		
	new
]

store-job: has [record al src][
	src: copy/part _s _e
	if all [
		block? allow
		block? forbid
		not empty? intersect allow forbid
	][
		do make error! rejoin ["bad or sub-optimal specifications for" mold src]
	]
	record: reduce [
		'name       all [name to word! name]
		'multiple   mult
		'unit       unit
		'allow      allow
		'forbid     forbid
		'on         on-day
		'at         ts
		'from       from
		'repeat     times
		'action     job
		'last       none
		'log?       yes
		'debug?     no
		'source     src
	]
	;probe new-line/all copy/part record 20 off
	repend jobs [next-event? record record]
]

blockify: func [name type /local blk][
	unless block? get name [set name make block! 1]
	name: get name
	unless blk: select name type [
		repend name [type blk: make block! 1]
	]
	blk
]

expand: func [name type s e /local list][
	list: blockify name type
	s: -1 + to integer! form s
	e:  1 + to integer! form e
	repeat c min e - s 60 [insert list e - c]
]

store: func [name type value /only /local list][
	list: blockify name type
	if issue? value [value: to integer! form value]
	either only [append/only list value][append list value]
]

cal-days: [set n integer!]

week-days: [
	  ['Monday     | 'Mon]	(n: 1)
	| ['Tuesday    | 'Tue]	(n: 2)
	| ['Wednesday  | 'Wed]	(n: 3)
	| ['Thursday   | 'Thu]	(n: 4)
	| ['Friday     | 'Fri]	(n: 5)
	| ['Saturday   | 'Sat]	(n: 6)
	| ['Sunday     | 'Sun]	(n: 7)
]
months: [
	  ['January    | 'Jan]	(n: 1)
	| ['February   | 'Feb]	(n: 2)
	| ['March      | 'Mar]	(n: 3)
	| ['April      | 'Apr]	(n: 4)
	| ['May        | 'May]	(n: 5)
	| ['June       | 'Jun]	(n: 6)
	| ['July       | 'Jul]	(n: 7)
	| ['August     | 'Aug]	(n: 8)
	| ['September  | 'Sep]	(n: 9)
	| ['October    | 'Oct]	(n: 10)
	| ['November   | 'Nov]	(n: 11)
	| ['December   | 'Dec]	(n: 12)
]

delays: [
	  ['seconds | 'second  | 'sec | 's] (unit: 'second)
	| ['minutes | 'minute  | 'mn]       (unit: 'minute)
	| ['hours   | 'hour    | 'h]        (unit: 'hour)
	| ['days    | 'day     | 'd]        (unit: 'day)
	| ['weeks   | 'week    | 'w]        (unit: 'day mult: 7 * any [mult 1])
	| ['months  | 'month   | 'm]        (unit: 'month) opt rule-on-day
   ;| 'last-day-of-month                (unit: 'ldom)	; unsupported	use every -1, -2...??
   ;| 'day-of-year                      (unit: 'doy)	; unsupported
]

rule-on-day: ['on set value issue! (unit: 'day store 'allow 'cal-days value)]

week-months: [
	week-days  (unit: 'day store type 'week-days n)
	| months   (unit: any [unit 'month] store type 'months n)
	  opt rule-on-day
]

restriction: [
	  set s issue! '- set e issue! (expand type 'cal-days s e)
	| set s time!  '- set e time!  (store/only type 'time reduce [s e])
	| set value time!              (store type 'time value) 
	| set value issue!             (unit: 'day store type 'cal-days value) 
	| week-months
]

restrictions: [restriction | into [some restriction]]

times-rule: [set times integer! ['times | 'time]]

every-rule: [
	opt [set mult integer!]
	[
		(type: 'allow) restrictions opt rule-on-day
		| [delays | (type: 'allow) week-months] opt [(type: 'allow) restrictions]
	]
	1 4 [
		opt ['not (type: 'forbid) restrictions]
		opt ['from set from [date! | time!]]
		opt ['at set ts [date! | time!]]
		opt times-rule
	]
]

dialect: [
	any [
		(reset-locals) err: _s:
		opt [set name set-word!]
		[
			'at [set ts [date! | time!]]
			| 'in set mult integer! delays (ts: 'in)
			| 'every every-rule
		]
		'do set job [file! | url! | block! | word! | function!] _e: (store-job)
	]
]

digits: system/catalog/bitsets/numeric

pre-process: func [src [string!] /local s e v fix][
	fix: [e: (s: change/part s v e) :s]
	parse src [
		any [
			s: "1st"   (v: "#1")  fix
			|  "2nd"   (v: "#2")  fix
			|  "3rd"   (v: "#3")  fix
			| copy v 1 3 digits [
				  "th" (v: join #"#" v)  fix
				| "s"  (v: join v " s")  fix
				| "mn" (v: join v " mn") fix
				| "h"  (v: join v " h")  fix
				| "w"  (v: join v " w")  fix
				| "m"  (v: join v " m")  fix
			]
			| skip
		]
	]		
	try/with [transcode src][
		do make error! join "Scheduler input syntax error in: " src
	]	
]

plan: func [spec [block! string!] /new][
	if new [reset]
	if string? spec [spec: pre-process spec]
	
	if not parse copy/deep spec dialect [
		print ["Error parsing at rule:" mold copy/part err 10]
	]	
	update-sys-timer
]

reset: does [
	clear jobs
	clear queue
	remove find wait-list time!
]

delete: func [name [word!] /local job][
	job: jobs
	forskip job 2 [
		if job/2/name = name [
			remove/part job 2
			return true
		]
	]
	do make error! reform ["job" mold name "not found!"]
]

