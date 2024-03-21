Rebol [
	Title:   "Rebol3 Poly1305 test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %poly1305-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Poly1305 mmessage authentication"
if native? :poly1305 [
;- this tests are only for the now deprecated `poly1305` native function!
===start-group=== "Poly1305 test vectors"
foreach [test-id key msg mac] [
	1
	#{eea6a7251c1e72916d11c2cb214d3c252539121d8e234e652d651fa4c8cff880}
	#{8E993B9F48681273C29650BA32FC76CE48332EA7164D96A4476FB8C531A1186AC0DFC17C98DCE87B4DA7F011EC48C97271D2C20F9B928FE2270D6FB863D51738B48EEEE314A7CC8AB932164548E526AE90224368517ACFEABD6BB3732BC0E9DA99832B61CA01B6DE56244A9E88D5F9B37973F622A43D14A6599B1F654CB45A74E355A5}
	#{f3ffc7703f9400e52a7dfb4b3d3305d9}

	2 ;- generates a final value of (2^130 - 2) == 3
	#{0200000000000000000000000000000000000000000000000000000000000000}
	#{ffffffffffffffffffffffffffffffff}
	#{03000000000000000000000000000000}
][
	--test-- join "Poly1305 one pass test " test-id
	--assert poly1305/update/verify key msg mac
	; or using finish:
	--assert mac = poly1305/update/finish key msg

	--test-- join "Poly1305 divided test " test-id
	;@@ NOTE: if key is pecified using handle like in this test,
	;@@ only one call to /verify or /finish will succeed as it internaly clears handle's state
	--assert handle? h: poly1305 key
	--assert handle? poly1305/update h take/part msg 32
	--assert handle? poly1305/update h take/part msg 32
	--assert handle? poly1305/update h take/part msg 3
	--assert handle? poly1305/update h msg ; rest of data
	--assert poly1305/verify h mac
]

--test-- "Poly1305 with failed verification"
	key: #{eea6a7251c1e72916d11c2cb214d3c252539121d8e234e652d651fa4c8cff880}
	msg: #{8E993B9F48681273C29650BA32FC76CE48332EA7164D96A4476FB8C531A1186A}
	--assert not poly1305/update/verify key msg #{0102} ; mac too short
	--assert not poly1305/update/verify key msg #{01020304050607082a7dfb4b3d3305d9} ; mac not correct

===end-group===
]

~~~end-file~~~