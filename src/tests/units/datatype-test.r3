Rebol [
	Title:   "Rebol3 datatype! test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %datatype-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "datatype"

===start-group=== "datatype!"
	--test-- "system/catalog/datatypes"
		--assert block? system/catalog/datatypes
		--assert datatype? first system/catalog/datatypes
		--assert #[end!] = first system/catalog/datatypes

	--test-- "reflect datatype!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/1534
		--assert object? sp: reflect integer! 'spec
		--assert  sp/title = reflect integer! 'title
		--assert  sp/type  = reflect integer! 'type
		--assert  sp/type = 'scalar

	--test-- "to word! datatype!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/38
		--assert 'logic!   = to word! logic!
		--assert 'percent! = to word! percent!
		--assert 'money!   = to word! money!

	--test-- "find datatype!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/256
		--assert none? find reduce [string! binary!] binary!
		--assert none? find [#[string!] #[binary!]] #[binary!]
		--assert none? find [#[string!] #[binary!]] binary!	

		--assert not none? find/only reduce [string! binary!] binary!
		--assert not none? find/only [#[string!] #[binary!]] #[binary!]
		--assert not none? find/only [#[string!] #[binary!]] binary!

		--assert not none? find ["test"] string!
		--assert not none? find ["test"] series!
		--assert  none? find reduce [integer! binary!] series!
		--assert ["aha"] = find reduce [integer! "aha"] series!
		--assert not none? find any-string! ref!

		--assert 1 = index? find [#{00}  #[string!] #[binary!]] #[binary!]
		--assert 3 = index? find/only [#{00}  #[string!] #[binary!]] #[binary!]

		--assert 2 = index? find/only reduce ["" string! any-string! binary!] string!
		--assert 3 = index? find/only reduce ["" string! any-string! binary!] any-string!
		--assert 1 = index? find reduce ["" string! any-string! binary!] any-string!
		--assert 1 = index? find reduce [%a string! any-string! binary!] any-string!
		--assert      none? find reduce [%a string! any-string! binary!] string!

		;; using old construction syntax
		--assert none? find [#[datatype! string!] #[datatype! binary!]] #[datatype! binary!]
		--assert none? find [#[datatype! string!] #[datatype! binary!]] binary!
		--assert not none? find/only [#[datatype! string!] #[datatype! binary!]] #[datatype! binary!]
		--assert not none? find/only [#[datatype! string!] #[datatype! binary!]] binary!

===end-group===

~~~end-file~~~