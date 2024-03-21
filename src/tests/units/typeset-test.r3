Rebol [
	Title:   "Rebol3 typeset! test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %typeset-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "typeset"

===start-group=== "typeset!"
	--test-- "make typeset!"
		--assert  typeset? types: make typeset! [block! map! object!]
	--test-- "find in typeset"
		--assert  find types block!
		--assert  find types map!
		--assert  find types object!
		--assert  not find types string!

	--test-- "union of typesets"
		types2: union types make typeset! [string!]
		--assert  typeset? types2
		--assert  find types2 string!

	--test-- "complement of typesets"
		not-types: complement types
		--assert typeset? not-types
		--assert find not-types integer!

	--test-- "difference of typesets"
		types3: difference types make typeset! [object!]
		--assert typeset? types3
		--assert not find types3 object!

	--test-- "empty typeset test"
		;@@ requested: https://github.com/red/REP/issues/55
		--assert  empty? make typeset! []
		--assert  not empty? types

	--test-- "bitset! is not a series!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/782
		--assert not find series! bitset!

	--test-- "unique"
		;@@ https://github.com/Oldes/Rebol-issues/issues/819
		--assert typeset? try [make typeset! [number! integer!]]

	--test-- "scalar!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2428
		--assert empty? exclude reduce [integer! decimal! percent! money! char! pair! tuple! time! date!] to-block scalar!

	--test-- "immediate!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/1195
		--assert empty? exclude reduce [
			none! logic! integer! decimal! percent! money! char! pair! tuple! time! date! datatype!
			typeset! word! set-word! get-word! lit-word! refinement! issue! event!] to-block immediate!

	--test-- "issue-456"
		;@@ https://github.com/Oldes/Rebol-issues/issues/456
		--assert "make typeset! [none! logic!]" = mold third types-of :insert

	--test-- "issue-985"
		;@@ https://github.com/Oldes/Rebol-issues/issues/985
		--assert [#(end!)] = difference system/catalog/datatypes to-block any-type!
		--assert (last to-block any-type!) == (last system/catalog/datatypes)
		--assert #(typeset! [end!]) = difference make typeset! system/catalog/datatypes any-type!

===end-group===


~~~end-file~~~