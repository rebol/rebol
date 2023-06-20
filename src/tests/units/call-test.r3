Rebol [
	Title:   "Rebol3 CALL test script"
	Author:  "Oldes"
	File: 	 %call-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "CALL"

out-buffer: copy ""
err-buffer: copy ""

rebol-cmd: func[cmd][
	clear out-buffer
	clear err-buffer
	insert cmd join to-local-file system/options/boot #" "
	call/shell/wait/output/error cmd out-buffer err-buffer
] 

===start-group=== "Command-Line Interface"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2228
	--test-- "--do"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2467
		--assert 0 = rebol-cmd {--do "print 1 + 2"}
		--assert out-buffer = "3^/"
	--test-- "script args 1"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1890
		--assert 0 = rebol-cmd {units/files/print-args.r3 a}
		--assert out-buffer = {["a"]^/["a"]^/}
		--assert 0 = rebol-cmd {units/files/print-args.r3 a b}
		--assert out-buffer = {["a" "b"]^/["a" "b"]^/}
		--assert 0 = rebol-cmd {units/files/print-args.r3 a "b c"}
		--assert out-buffer = {["a" "b c"]^/["a" "b c"]^/}
		either find [Macintosh Linux] system/platform [
			; single quotes are not used in Windows' command line
			--assert 0 = rebol-cmd {units/files/print-args.r3 a 'b " c'}
			--assert out-buffer = {["a" {b " c}]^/["a" {b " c}]^/}
		][
			--assert 0 = rebol-cmd {units/files/print-args.r3 a "b \" c"}
			--assert out-buffer = {["a" {b " c}]^/["a" {b " c}]^/}
		]
	--test-- "script args 2"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2227
		--assert 0 = rebol-cmd {-v}
		--assert not none? find/match out-buffer {Rebol/}
		--assert 0 = rebol-cmd {units/files/print-args.r3 -v}
		--assert out-buffer = {["-v"]^/["-v"]^/}
		--assert 0 = rebol-cmd {units/files/print-args.r3 -x}
		--assert out-buffer = {["-x"]^/["-x"]^/}
		--assert 0 = rebol-cmd {units/files/print-args.r3 -- -x}
		--assert out-buffer = {["--" "-x"]^/["--" "-x"]^/}
		--assert 0 = rebol-cmd {units/files/print-args.r3 -v -- -x}
		--assert out-buffer = {["-v" "--" "-x"]^/["-v" "--" "-x"]^/}
		--assert 0 = rebol-cmd {--args "a b" units/files/print-args.r3 -v}
		--assert out-buffer = {["a b" "-v"]^/["a b"]^/}
		; providing script using --script option
		;@@ https://github.com/Oldes/Rebol-issues/issues/2469
		--assert 0 = rebol-cmd {--script units/files/print-args.r3 --args "a b" -- -v}
		--assert out-buffer = {["a b" "-v"]^/["a b"]^/}
		--assert 0 = rebol-cmd {--script units/files/print-args.r3 1 2}
		--assert out-buffer = {["1" "2"]^/["1" "2"]^/}
		--assert 0 = rebol-cmd {--args 1 --script units/files/print-args.r3 2}
		--assert out-buffer = {["1" "2"]^/["1"]^/}
	--test-- "script args 3"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2140
		cmd: "units/files/print-args.r3"
		repeat i 1000 [append append cmd #" " i]
		--assert 0 = rebol-cmd cmd
		--assert #{F41512F56B85D5805C62E6587C26A969DC3D41AE} = checksum out-buffer 'sha1
		if find [Macintosh Linux] system/platform [
			--assert 0 = rebol-cmd {--do "print system/options/args quit" `seq 1 1000`}
			--assert #{17900469C3C78A614B60FEE4E3851EF6BAF9D876} =  checksum out-buffer 'sha1
		]
===end-group===


===start-group=== "Error pipe"
	--test-- "User controlled error output"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2468
		--assert 0 = rebol-cmd {--do "prin 1 modify system/ports/output 'error on prin 2 modify system/ports/output 'error off prin 3"}
		--assert "13" = out-buffer
		--assert "2"  = err-buffer

	--test-- "Error printed to stderr"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1862
		--assert 1 = rebol-cmd {--do "prin 2 1 / 0"}
		--assert "2" = out-buffer
		--assert not none? find err-buffer "Math error"


===end-group===


===start-group=== "LAUNCH"
	--test-- "launch"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1403
		--assert 0 < launch %units/files/launched.r3 ;; returns a process id if not used /wait 
	--test-- "do launch"
		;@@ https://github.com/Oldes/Rebol-issues/issues/914
		--assert 0:0:1 > delta-time [do %units/files/launch.r3] ;; should not wait
	--test-- "do launch/wait"
		--assert 0:0:2 < delta-time [do %units/files/launch-wait.r3] ;; should wait
		--assert 6 = try [length? read/lines %units/files/launched.txt] ;; 6 because 3x launched!

	try [delete %units/files/launched.txt]
===end-group===

~~~end-file~~~