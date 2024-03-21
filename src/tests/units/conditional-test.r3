Rebol [
	Title:   "Rebol conditional test script"
	Author:  "Oldes"
	File: 	 %conditional-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Conditional"

===start-group=== "Dealing with unset! value in conditions"
	--test-- "any"
		--assert 2 = any [() 2]
		--assert true?  any [() 2]
	--test-- "all"
		--assert true? all []
		--assert true? all [()]
		--assert 3 = all [() 1 2 3]
		--assert 1 = all [1 ()]
		--assert 1 = if all [1 ()][1]
		--assert 1 = either all [()][1][2]
	--test-- "any and all"
		if any [
			all [false x: 1 ()]
			all [true  x: 2 ()]
		][ x: 2 * x]
		--assert x = 4
	--test-- "unset value transparency in any and all"
		;@@ https://github.com/Oldes/Rebol-issues/issues/850
		;@@ https://github.com/Oldes/Rebol3/discussions/88
		--assert  none? any []
		--assert  none? any [()]
		--assert    1 = any [() 1]
		--assert    1 = all [1 ()]
		--assert unset? all []
		--assert unset? all [()]
===end-group===

===start-group=== "SWITCH"
	--test-- "switch ref!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2466
		--assert 1 = switch @a [@a [1] @b [2]]
		--assert 2 = switch @b [@a [1] @b [2]]
===end-group===

;- using tests from Red language...
===start-group=== "switch basics"

	--test-- "switch-basic-1"
		sb1-i: 1
		sb1-j: -1
		switch sb1-i [
			0	[sb1-j: 0]
			1	[sb1-j: 1]
			2	[sb1-j: 2]
		]
        --assert sb1-j = 1
	
	--test-- "switch-basic-2"
		sb2-i: "Nenad"
		sb2-j: "REBOL"
		switch sb2-i [
			"Andreas" 	[sb2-j: "Earl"]
			"Nenad"		[sb2-j: "Red"]
			"Peter"		[sb2-j: "Peter"]
		]
	    --assert sb2-j = "Red"
	
	--test-- "switch-basic-3"
		sb3-i: "Χαῖρε, κόσμε"
		sb3-j: "REBOL"
		switch sb3-i [
			"Andreas" 			[sb3-j: "Earl"]
			"Nenad"				[sb3-j: "Red"]
			"Peter"				[sb3-j: "Peter"]
			"Χαῖρε, κόσμε"		[sb3-j: "Zorba"]
		]
	    --assert sb3-j = "Zorba"
	
	--test-- "switch-basic-4"
		sb4-i: "Zorba"
		sb4-j: "REBOL"
		switch sb4-i [
			"Andreas" 			[sb4-j: "Earl"]
			"Nenad"				[sb4-j: "Red"]
			"Peter"				[sb4-j: "Peter"]
			"Zorba"				[sb4-j: "Χαῖρε, κόσμε"]
		]
	    --assert sb4-j = "Χαῖρε, κόσμε"
	
	--test-- "switch-basic-5"
	;@@ R3 does not support UCS4 chars yet!
;		sb5-i: "abcde^(010000)"
;		sb5-j: "REBOL"
;		switch sb5-i [
;			"Andreas" 			[sb5-j: "Earl"]
;			"Nenad"				[sb5-j: "Red"]
;			"Peter"				[sb5-j: "Peter"]
;			"Zorba"				[sb5-j: "Χαῖρε, κόσμε"]
;			"abcde^(010000)"	[sb5-j: "boron"]
;		]
;	    --assert sb5-j = "boron"
	
	--test-- "switch-basic-6"
	;@@ R3 does not support UCS4 chars yet!
;		sb6-i: #"^(010000)"
;		sb6-j: "REBOL"
;		switch sb6-i [
;			#"a" 				[sb6-j: "Earl"]
;			#"b"				[sb6-j: "Red"]
;			#"c"				[sb6-j: "Peter"]
;			#"d"				[sb6-j: "Χαῖρε, κόσμε"]
;			#"^(010000)"		[sb6-j: "boron"]
;		]
;    	--assert sb6-j = "boron"
	
	--test-- "switch-basic-8"
		sb8-i: %Nenad
		sb8-j: "REBOL"
		switch sb8-i [
			%Andreas 	[sb8-j: "Earl"]
			%Nenad		[sb8-j: "Red"]
			%Peter		[sb8-j: "Peter"]
		]
		--assert sb8-j = "Red"
	
	--test-- "switch-basic-9"
		sb9-i: #Nenad
		sb9-j: "REBOL"
		switch sb9-i [
			#Andreas 	[sb9-j: "Earl"]
			#Nenad		[sb9-j: "Red"]
			#Peter		[sb9-j: "Peter"]
		]
        --assert sb9-j = "Red"
	
	--test-- "switch-basic-10"
		sb10-i: true
		sb10-j: "REBOL"
		switch sb10-i [
			#(false)	[sb10-j: "Earl"]
			#(true)		[sb10-j: "Red"]
			#(true)		[sb10-j: "Peter"]
		]
        --assert sb10-j = "Red"
	
	--test-- "switch-basic-11"
		sb11-i: first [(1 2 3)]
		sb11-j: "REBOL"
		switch sb11-i [
			(3 2 1)	 	[sb11-j: "Earl"]
			(1 2 3)		[sb11-j: "Red"]
			(2 3 1)		[sb11-j: "Peter"]
		]
        --assert sb11-j = "Red"
	
	--test-- "switch-basic-13"
		sb13-i: first [(2)]
		sb13-j: "REBOL"
		switch sb13-i [
			(1)	 	[sb13-j: "Earl"]
			(2)		[sb13-j: "Red"]
			(3)		[sb13-j: "Peter"]
		]
        --assert sb13-j = "Red"
	
	--test-- "switch-basic-14"
		sb14-i: [2]
		sb14-j: "REBOL"
		switch first sb14-i [
			1 		[sb14-j: "Earl"]
			2		[sb14-j: "Red"]
			3		[sb14-j: "Peter"]
		]
        --assert sb14-j = "Red"
	
	--test-- "switch-basic-15"
		sb15-i: integer!
		sb15-j: "REBOL"
		switch sb15-i [
			1 			[sb15-j: "Earl"]
			integer!	[sb15-j: "Peter"]
			#(integer!)	[sb15-j: "Red"]
			#(char!)	[sb15-j: "Blue"]
		]
        --assert sb15-j = "Red"

	--test-- "switch-basic-16"
		sb16-i: number!
		sb16-j: "REBOL"
		do [switch sb16-i reduce [
			1 			[sb16-j: "Earl"]
			'number!	[sb16-j: "Peter"]
			number!		[sb16-j: "Red"]					;@@ no idea how to encode a typeset literally, have to evaluate
			char!		[sb16-j: "Blue"]
		]]
        --assert sb16-j = "Red"

;	--test-- "switch-basic-17"
	;@@ having unset! as a switch value is not allowed in R3.. should it be?
;		sb17-j: "REBOL"
;		do [switch () reduce [
;			unset! 		[sb17-j: "Earl"]
;			100			[sb17-j: "Peter"]
;			()			[sb17-j: "Red"]					;@@ no idea how to encode `unset` literally, have to evaluate
;			char!		[sb17-j: "Blue"]
;		]]
;        --assert sb17-j = "Red"


===end-group===

===start-group=== "switch basics local"    ;; one "direct" type & one "indirect"
										   ;;  should be sufficient

	sb-i: "Hello"
	sb-j: "World"
	switch-fun: func [/local sb-i sb-j][
		--test-- "switch-basic-local-1"
			sb-i: 1
			sb-j: -1
			switch sb-i [
				0	[sb-j: 0]
				1	[sb-j: 1]
				2	[sb-j: 2]
			]
		    --assert sb-j = 1
		--test-- "switch-basic-local-2"
			sb-i: "Nenad"
			sb-j: "REBOL"
			switch sb-i [
				"Andreas" 	[sb-j: "Earl"]
				"Nenad"		[sb-j: "Red"]
				"Peter"		[sb-j: "Peter"]
			]
		    --assert sb-j = "Red"
	]
	switch-fun
	--test-- "switch-basic-local-global-1"
	    --assert sb-i = "Hello"
	    --assert sb-j = "World"
	
===end-group===

===start-group=== "switch-default"
	
	--test-- "switch-default-1"
		sd1-i: 2
		sd1-j: -1
		switch/default sd1-i [
			1	[sd1-j: 1]
			2	[sd1-j: 2]
		][
			sd1-j: 0
		]
	    --assert sd1-j = 2
	
	--test-- "switch-default-2"
		sd2-i: 999
		sd2-j: -1
		switch/default sd2-i [
			1	[sd2-j: 1]
			2	[sd2-j: 2]
		][
			sd2-j: 0
		]
	    --assert sd2-j = 0
	
	--test-- "switch-default-3"
		sd3-i: "hello"
		sd3-j: -1
		switch/default sd3-i [
			1	[sd3-j: 1]
			2	[sd3-j: 2]
		][
			sd3-j: 0
		]
	    --assert sd3-j = 0

===end-group===

===start-group=== "switch-all"			;; not sure if it will be implemented.
	comment {
	--test-- "switch-all-1" 
		sa1-i: 1
		sa1-j: 0
		switch/all sa1-i [
			0	[sa1-j: sa1-j + 1]
			1	[sa1-j: sa1-j + 2]
			2	[sa1-j: sa1-j + 4]
		]
	    --assert sa1-j = 6 
	}
	
===end-group===

===start-group=== "switch-multi"		;; not sure if it will be implemented. 

    --test-- "switch-multi-1"
        sm1-i: 2
        sm1-j: 0
        switch sm1-i [
            1 2	[sm1-j: 1]
        ]
        --assert sm1-j = 1
        
===end-group===

~~~end-file~~~