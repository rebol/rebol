REBOL []

;catch any error:
if error? set/any 'err try [

TOOLS-Win32: ""
TOOLS-Win64: ""

attempt [do %make-settings.r3]

cmd-menu: {^/^[[1;33;49mPlease choose version to build:^[[0m^/}

cmd-actions: [
	"q" [quit]
	""  [val: none]
]

settings: []

rebol-tool: join "prebuild\r3-make" switch/default system/version/4 [
	1  [%-amiga  ]
	2  [
		append settings [
			"OSX - PPC"                  [0.2.04 %..\build\osx-ppc\        ""]
			"OSX - Intel"                [0.2.05 %..\build\osx-intel\      ""]
			"OSX - Intel x64"            [0.2.40 %..\build\osx-intel-x64\  ""]
		]
		%-osx    ]
	3  [
		append settings [
			"Win32 console"              [0.3.01 %..\build\win-x86\ TOOLS-Win32]
			"Win64 console"              [0.3.40 %..\build\win-x64\ TOOLS-Win64]
		]
		%-win.exe]
	4  [
		append settings [
			"Linux libc 2.3"             [0.4.02 %../build/linux/   ""]
			"Linux libc 2.5"             [0.4.03 %../build/linux/   ""]
			"Linux libc 2.11"            [0.4.04 %../build/linux/   ""]
			"Linux PPC"                  [0.4.10 %../build/linux/   ""]
			"Linux ARM"                  [0.4.20 %../build/linux/   ""]
			"Linux ARM bionic (Android)" [0.4.21 %../build/linux/   ""]
			"Linux MIPS"                 [0.4.30 %../build/linux/   ""]
			"Linux x64"                  [0.4.40 %../build/linux/   ""]
		]
		%-linux  ]
	5  [%-haiku  ]
	7  [%-freebsd]
	9  [%-openbsd]
	13 [%-android]
][	%"" ]

i: 1
foreach [name data] settings [
	set [OS_ID BUILD_DIR TOOLS] data
	append cmd-menu rejoin ["^-^[[1;32;49m" i "^[[0m:  " name " (" OS_ID ")^/"]
	append cmd-actions compose/deep [
		(form i) [OS_ID: (OS_ID) BUILD_DIR: (BUILD_DIR) TOOLS: (TOOLS)]
	]
	i: i + 1
]
append cmd-menu {^-^[[1;32;49mq^[[0m:  Quit^/}

eval-cmd: func[cmd [string! block!] /local err][
	out: copy ""
	if block? cmd [cmd: rejoin cmd]
	prin "^[[1;33;49m"
	print [cmd "^[[0m^[[36m"]
	call/wait/shell cmd out
	print "^[[0m"
	;print out
]

menu?: true
forever [
	if menu? [print cmd-menu]
	error? try [val: trim/head/tail ask "^[[1;32;49mBuild version: ^[[0m"]
	switch/default val cmd-actions [
		print "What?"
		val: none
	]

	if val [
		eval-cmd [rebol-tool " -qs ../src/tools/make-make.r " OS_ID]
		eval-cmd [{make TOOLS=} TOOLS { all}]

		make-dir/deep BUILD_DIR
		eval-cmd [{move /Y r3.exe    } to-local-file BUILD_DIR]
		eval-cmd [{move /Y host.exe  } to-local-file BUILD_DIR]
		eval-cmd [{move /Y libr3.dll } to-local-file BUILD_DIR]
		eval-cmd [{dir }               to-local-file BUILD_DIR]
		menu?: true
	]
]

][ ; there was some error!
	print "ERROR!"
	print err
]