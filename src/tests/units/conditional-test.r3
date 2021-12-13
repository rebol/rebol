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
		--assert unset? any [() 2]
		--assert true?  any [() 2]
	--test-- "all"
		--assert true? all []
		--assert true? all [()]
		--assert 3 = all [() 1 2 3]
		--assert unset? all [1 ()]
		--assert 1 = if all [1 ()][1]
		--assert 1 = either all [()][1][2]
	--test-- "any and all"
		if any [
			all [false x: 1 ()]
			all [true  x: 2 ()]
		][ x: 2 * x]
		--assert x = 4
===end-group===

===start-group=== "SWITCH"
	--test-- "switch ref!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2466
		--assert 1 = switch @a [@a [1] @b [2]]
		--assert 2 = switch @b [@a [1] @b [2]]
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