Rebol [
	name:    wait-for-key
	title:   "Simple Rebol `wait-for-key` function"
	purpose: "Demonstrate how to get simple user keyboard input from console without need to press ENTER"
	author:  "Oldes"
	needs:   3.12.0  ; https://github.com/Oldes/Rebol3
]

wait-for-key: func[
	"Wait for single key press and return char as a result"
	/only chars [bitset! string!] "Limit input to specified chars"
	/local port old-awake
][
	; using existing input port
	port: system/ports/input
	; store awake actor and enter turn off read-line mode
	old-awake: :port/awake
	modify port 'line false
	; store optional chars limit in port's extra
	port/extra: chars
	; clear old data (in case user cancel's waiting)
	port/data: none
	; define new awake, which checks single key
	port/awake: func[event][
		all[
			'key = event/type
			any [
				none? port/extra
				find port/extra event/key
			]
			event/port/data: event/key 
			true
		]
	]
	; wait for user input
	wait/only port
	; put back original awake actor and read-line mode
	port/awake: :old-awake
	modify port 'line true
	; return result and clear port's data
	also port/data port/data: none
]

;-- Usage example ------------------------------------------------------
print "Press any key!"
char: wait-for-key
print ["You pressed:" mold char]

print "Press ENTER!"
wait-for-key/only CRLF

print "Press any number!"
char: wait-for-key/only charset [#"0" - #"9"]
print ["You pressed:" char]
print "Good choice!^/"
print "That's all... good bye!"
wait-for-key

