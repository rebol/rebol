Rebol [
	Title:   "Rebol3 unicode test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %unicode-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]



~~~start-file~~~ "UNICODE"

===start-group=== "rejoin"
	--test-- "rejoin to binary"
		--assert #{C5A1} == rejoin [#{}  "š"]
		--assert #{C5A1} == rejoin [#{} #"š"]

	--test-- "rejoin to string"
		--assert "šáh" == rejoin [""  "š" "áh"]
		--assert "šáh" == rejoin ["" #"š" "áh"]

===end-group===

===start-group=== "to"
	--test-- "to word!"
		--assert 'š = to word! "š"
		--assert 'š = to word! #"š"
		--assert 'š = to word! " š"
		--assert 'š = to word! " š "
		--assert '🙂 = to word!  "🙂"
		--assert '🙂 = to word! #"🙂"
		--assert '🙂 = to word! #"^(1F642)"
	--test-- "to binary!"
		--assert #{C5A1} == to binary!  "š"
		--assert #{C5A1} == to binary! #"š"
		--assert #{F09F9982} == to binary!  "🙂"
		--assert #{F09F9982} == to binary! #"🙂"
	--test-- "to string!"
		--assert "š" == to string! #{C5A1}
		--assert "🙂" == to string! #{F09F9982}
		--assert "" == to string! #{}
		--assert all [
			error? e: try [to string! #{C5A1C5}]
			e/id = 'invalid-utf
			e/arg1 = #{C5}
		]
		--assert all [
			error? e: try [to string! #{C5A1C500}]
			e/id = 'invalid-utf
			e/arg1 = #{C500}
		]
		;; surrogates...
		--assert "𝄢" == to string! #{EDA0B4EDB4A2}
		
 	--test-- "to block!"
		--assert [šáh 🙂] == make block! "šáh 🙂"
	--test-- "to char!"
		--assert #"š" == to char! #{C5A1}
		--assert #"🙂" == to char! #{F09F9982}
		--assert 65533 == try [to-integer to-char #{EFBFBD}] ;; replacement char is valid
		--assert all [
			error? e: try [to char! #{C5}]
			e/id = 'bad-make-arg ;@@ or use invalid-utf8 error instead?
			e/arg2 = #{C5}
		]
		--assert all [
			error? e: try [to char! #{F09F99}]
			e/id = 'bad-make-arg
			e/arg2 = #{F09F99}
		]
	--test-- "to string! char!"
		--assert #{F09F9982} == to binary! copy/part to string! #"🙂" 1

===end-group===


===start-group=== "natives"
	--test-- "lowercase char!"
		--assert #"🙂" == lowercase #"🙂"
		--assert #"š" == lowercase #"š"
		--assert #"š" == lowercase #"Š"
	--test-- "uppercase char!"
		--assert #"🙂" == uppercase #"🙂"
		--assert #"Š" == uppercase #"š"
		--assert #"Š" == uppercase #"Š"
	
	--test-- "lowercase string"
		--assert "šáh: 🙂" == lowercase "ŠÁH: 🙂"
	--test-- "uppercase string"
		--assert "ŠÁH: 🙂" == uppercase "šáh: 🙂"

	--test-- "split-lines"
		--assert ["š" "á"] == split-lines "š^/á"
		--assert ["🙂" "🙂"] == split-lines "🙂^/🙂"
		
	--test-- "to-local-file"
		--assert "š" == to-local-file %š
		--assert "🙂" == to-local-file %🙂

	--test-- "entab/detab"
		--assert "^-ábč^/^-" == entab "    ábč^/    "
		--assert "    ábč^/    " == detab "^-ábč^/^-"

	--test-- "wildcard?"
		--assert not wildcard? %ačb🙂č
		--assert     wildcard? %ačb🙂č*
		--assert     wildcard? %ač?🙂č

	--test-- "iconv"
		--assert #{00E1D83DDE42} == iconv/to to binary! "á🙂" 'UTF8 'UTF-16BE
		--assert #{E1003DD842DE} == iconv/to to binary! "á🙂" 'UTF8 'UTF-16LE
		--assert "á🙂" == iconv #{00E1D83DDE42} 'UTF-16BE
		--assert "á🙂" == iconv #{E1003DD842DE} 'UTF-16LE

	--test-- "call"
		exe: to-local-file system/options/boot
		--assert all [
			out: ""
			not error? try [call/shell/output rejoin [exe { --do "print 'á🙂"}] out]
			out == "á🙂^/"
		]
		--assert all [
			out: #{}
			not error? try [call/shell/output rejoin [exe { --do "print 'á🙂"}] out]
			out == to binary! "á🙂^/"
		]

	--test-- "first+"
		str: "🙂čb"
		--assert all [#"🙂" == first+ str  str == "čb"]
		--assert all [#"č" == first+ str  str == "b"]
		--assert all [#"b" == first+ str  str == ""]
		--assert all [none? first+ str  str == ""]

	--test-- "++"
		str: "🙂čb"
		--assert all ["🙂čb" == ++ str  str == "čb"]
		--assert all ["čb" == ++ str  str == "b"]
		--assert all ["b" == ++ str  str == ""]
		--assert all ["" == ++ str  str == ""]

	--test-- "--"
		str: tail "🙂čb"
		--assert all ["" == -- str  str == "b"]
		--assert all ["b" == -- str  str == "čb"]
		--assert all ["čb" == -- str  str == "🙂čb"]
		--assert all ["🙂čb" == -- str  str == "🙂čb"]

	--test-- "last"
		--assert #"🙂" == last "ab🙂"

	--test-- "first"
		--assert #"🙂" == first "🙂ab"
		--assert #"🙂" == first next "a🙂b"
	
	--test-- "foreach"
		--assert all [
			str: "áb🙂"
			out: copy ""
			foreach c str [append out c]
			str == out
		]
		--assert all [
			str: "a🙂čb"
			out: copy ""
			foreach [a b] str [append append out b a]
			out == "🙂abč"
		]

	--test-- "remove-each"
		--assert all [
			str: "a🙂čb"
			out: copy ""
			"ab" == remove-each c str [append out c  c > 0#7f]
			"ab" == str
			out  == "a🙂čb"
		]

	--test-- "remove-each/count"
		--assert all [
			str: "a🙂čb"
			2 == remove-each/count c str [c > 0#7f]
			"ab" == str
		]

	--test-- "forall"
		--assert all [
			str: next "xáb🙂"
			out: copy ""
			n: 0
			3 == forall str [append out str/1 n: n + 1]
			out == str
		]
		--assert all [
			str: tail "xáb🙂"
			out: copy ""
			n: 0
			none? forall str [append out str/1 n: n + 1]
			empty? out
		]

	--test-- "forall + take"
		--assert all [
			str: "ááb"
			out: copy ""
			n: 0
			2 == forall str [if str/1 >= 127 [append out take str] n: n + 1]
			str == "áb" ;; by design that only one "á" was removed
			out == "á"  ;; because `take` modifies the index!
		]


	--test-- "forskip"
		--assert all [
			str: next "xáb🙂d"
			out: copy ""
			n: 0
			2 == forskip str 2 [append out str/1 n: n + 1]
			out == "á🙂"
		]

		--assert all [
			str: back tail "xáb🙂d"
			out: copy ""
			n: 0
			1 == forskip str 2 [str append out str/1 n: n + 1]
			out == "d"
		]

		--assert all [
			s1: "abcd"
			s2: "ABCDEF"
			out: copy ""
			n: 0
			3 == forskip s1 2 [append out s1/1 s1: at s2 index? s1 n: n + 1]
			out == "aCE"
		]
		--assert all [
			s1: "abcd"
			s2: "ABČDEF"
			out: copy ""
			n: 0
			3 == forskip s1 2 [probe s1 append out s1/1 s1: at s2 index? s1 n: n + 1]
			out == "aČE"
		]

	--test-- "repeat"
		--assert all [
			str: next "xáb🙂d"
			out: copy ""
			repeat x str [append out x/1]
			out == "áb🙂d"
		]

	--test-- "for"
		--assert all [
			str: next "xáb🙂d"
			out: copy ""
			none? for x str 1 1 [append out x/1]
		]
		--assert all [
			str: next "xáb🙂d"
			out: copy ""
			for x str 2 1 [append out x/1]
			out == "á"
		]
		--assert all [
			str: next "xáb🙂d"
			out: copy ""
			for x str 100 1 [append out x/1]
			out == "áb🙂dnone" ;; none, because the last value is empty
		]
		--assert all [
			str: back tail "xáb🙂d"
			out: copy ""
			for x str 1 -1 [append out x/1]
			out == "d🙂báx"
		]

	--test-- "to-hex"
		--assert #01F642 = to-hex #"🙂"
		--assert #01F642 = to-hex #"^(01F642)"

	--test-- "difference"
		--assert (difference "ač" "čbš") == "abš"
		--assert (difference "ač🙂" "čbš") == "a🙂bš"
		--assert (difference "ač🙂" "🙂čbš") == "abš"
		--assert (difference "ab" "čbš🙂") == "ačš🙂"
		--assert (difference "čbš🙂" "ab") == "čš🙂a"
	--test-- "difference/skip"
		--assert (difference/skip "ač" "čbš" 2) == "aččbš"
		--assert (difference/skip "ač" "čbčbš" 2) == "aččbš"
		--assert (difference/skip "a🙂" "čbčba🙂" 2) == "čb"

	--test-- "union"
		--assert (union "ač" "čbš") == "ačbš"
		--assert (union "ač🙂" "čbš") == "ač🙂bš"
		--assert (union "ač🙂" "🙂čbš") == "ač🙂bš"
		--assert (union "ab" "čbš🙂") == "abčš🙂"
		--assert (union "čbš🙂" "ab") == "čbš🙂a"
	--test-- "union/skip"
		--assert "ábča"   = unique/skip "ábábča" 2
		--assert "ábabca" = unique/skip s: "ábabcaába" 3
		--assert s = "ábabcaába"

	--test-- "exclude"
		--assert (exclude "ač" "čbš") == "a"
		--assert (exclude "ač🙂" "čbš") == "a🙂"
		--assert (exclude "ač🙂" "🙂čbš") == "a"
		--assert (exclude "ab" "čbš🙂") == "a"
		--assert (exclude "čbš🙂" "ab") == "čš🙂"
	--test-- "exclude/skip"
		--assert (exclude/skip "aččb" "čbš" 2) == "ač"
		--assert (exclude/skip "aččb" "čbčbš" 2) == "ač"
		--assert (exclude/skip "a🙂" "čbčba🙂" 2) == ""
		--assert (exclude/skip "a🙂🙂a" "čbčba🙂" 2) == "🙂a"

	--test-- "intersect"
		--assert (intersect "ač" "čbš") == "č"
		--assert (intersect "ač🙂" "čbš") == "č"
		--assert (intersect "ač🙂" "🙂čbš") == "č🙂"
		--assert (intersect "ab" "čbš🙂") == "b"
		--assert (intersect "čbš🙂" "ab🙂") == "b🙂"
	--test-- "intersect/skip"
		--assert (intersect/skip "aččb" "čbš" 2) == "čb"
		--assert (intersect/skip "aččb" "čbčbš" 2) == "čb"
		--assert (intersect/skip "a🙂" "čbčba🙂" 2) == "a🙂"
		--assert (intersect/skip "a🙂🙂a" "čbčba🙂" 2) == "a🙂"

	--test-- "unique"
		--assert (unique "ača") == "ač"
		--assert (unique "ač🙂🙂č") == "ač🙂"
	--test-- "unique/skip"
		--assert (unique/skip "baaččbač" 2) == "baaččb"
		--assert (unique/skip "b🙂aččb🙂č" 2) == "b🙂aččb🙂č"

	--test-- "mold/part"
		str: "═════"
		--assert {"}   == mold/part str 1
		--assert {"═}  == mold/part str 2
		--assert {"══} == mold/part str 3
		--assert {"══} == copy/part mold str 3
	--test-- "mold/part with LF"
		str: "^/═════"
		--assert {"}     == mold/part str 1
		--assert {"^^}   == mold/part str 2
		--assert {"^^/}  == mold/part str 3
		--assert {"^^/═} == mold/part str 4
		--assert {"^^/═} == copy/part mold str 4
	--test-- "mold/part with {"
		str: "{═════"
		--assert {"}     == mold/part str 1
		--assert {"^{}   == mold/part str 2
		--assert {"^{═}  == mold/part str 3
		--assert {"^{══} == mold/part str 4
		--assert {"^{══} == copy/part mold str 4
	--test-- "mold/part long"
		str: "{═════{═════{═════{═════{═════{═════{═════^""
		--assert {"}     == mold/part str 1
		--assert {"^{}   == mold/part str 2
		--assert {"^{═}  == mold/part str 3
		--assert {"^{══} == mold/part str 4
		--assert "{^^{═" == copy/part mold str 4
	--test-- "copy/part mold/part"
		str: "═════"
		--assert {"═} == copy/part mold/part str 4 2
		--assert {"══} == copy/part mold/part str 4 3
		--assert {"═══} == copy/part mold/part str 4 4
		--assert {"═══} == copy/part mold/part str 4 5
		--assert {"════} == copy/part mold/part str 6 5
		--assert {"═════"} == copy/part mold/part str 8 8


===end-group===


===start-group=== "lexer"
	--test-- "raw string"
		--assert "🙂" == transcode/one "%{🙂}%"
	--test-- "email!"
		--assert šč@ab == transcode/one #{C5A1C48D406162}
		--assert šč@🙂b == transcode/one #{C5A1C48D40F09F998262}
	--test-- "ref!"
		--assert @šč == transcode/one #{40C5A1C48D}
		--assert @🙂b == transcode/one #{40F09F998262}

	--test-- "string with surrogates"
		;; UTF-16 surrogates are not allowed in UTF-8
		;; so this is not a valid string: "^(D834)"
		--assert all [
			error? e: transcode/one/error #{225E28443833342922}
			e/id = 'invalid
		]
		--assert all [
			error? e: transcode/one/error #{225E2844383334295E28444432322922} ;; "^(D834)^(DD22)"
			e/id = 'invalid
		]
	--test-- "string with a char over Unicode range"
		--assert all [
			error? e: transcode/one/error #{225E28313130303030292922} ;; "^(110000)"
			e/id = 'invalid
		]

===end-group===


===start-group=== "paths"
	--test-- "get path"
		--assert #"á" == all [s: "áb" probe s/1]
		--assert #"b" == all [s: "áb" s/2]
		--assert #"🙂" == all [s: "🙂b" s/1]
		--assert #"b" == all [s: "🙂b" s/2]
	--test-- "get path with negative index"
		--assert #"á" == all [s: tail "áb" s/-2]
		--assert #"b" == all [s: tail "áb" s/-1]
		--assert #"🙂" == all [s: tail "🙂b" s/-2]
		--assert #"b" == all [s: tail "🙂b" s/-1]
	--test-- "get path over limits"
		--assert none? all [s: "áb" s/3]
		--assert none? all [s: "🙂b" s/3]
		--assert none? all [s: tail "áb" s/-3]
		--assert none? all [s: tail "🙂b" s/-3]

	--test-- "modify ascii string using unicode char"
		--assert "šb" == all [s: "ab" s/1: #"š" s]
		--assert "aš" == all [s: "ab" s/2: #"š" s]
		--assert error? try [all s: "ab" s/3: #"š"]

	--test-- "modify utf8 string using ascii char"
		--assert "sb" == all [s: "áb" s/1: #"s" s]
		--assert "ás" == all [s: "áb" s/2: #"s" s]
		--assert error? try [all s: "ab" s/3: #"š"]



===start-group=== "actions"
	--test-- "find char!"
		--assert "č" == find "abč" #"č"
		--assert "ččá" == find "🙂ččá" #"č"
		--assert "čá" == find/tail "🙂ččá" #"č"
		--assert "čá" == find/skip "🙂ččá" #"č" 2
		--assert "Čá" == find/case "🙂čČá" #"Č"
		--assert "čá" == find/last/skip "ačb🙂čá" #"č" 2
		--assert "č"  == find/last/skip "ačb🙂č"  #"č" 2
		--assert "🙂ččá" == find/match "🙂ččá" #"🙂"
		--assert none? find/match "x🙂ččá" #"🙂"

	--test-- "find string!"
		--assert "čb🙂čb" == find "áčb🙂čb" "čb"
		--assert "🙂čb" == find/tail "áčb🙂čb" "čb"
		--assert "čb" == find/skip "áčbxčb" "čb" 2
		--assert "čb" == find/skip "áčb🙂čb" "čb" 2
		--assert "" == find/skip/tail "áčb🙂čb" "čb" 2
		--assert "čb🙂čx" == find/last "ačb🙂čx" "čb"
		--assert "čb" == find/last "ačb🙂čb" "čb"

	--test-- "find tag!"
		--assert "<b>🙂čx" == find "ač<b>🙂čx" <b>
		--assert "<b>č" == find "🙂<<b>č" <b>
		--assert "<🙂>čx" == find "ačb<🙂>čx" <🙂>
		--assert "<b🙂>čx" == find "ač<b🙂>čx" <b🙂>
		--assert "🙂čx" == find/tail "ač<b>🙂čx" <b>
		--assert "čx" == find/tail "ačb<🙂>čx" <🙂>
		--assert "čx" == find/tail "ač<b🙂>čx" <b🙂>
		--assert "<🙂>čx" == find/match "<🙂>čx" <🙂>
		--assert none? find/match "a<🙂>čx" <🙂>
		--assert "<b>čx" == find/last "a<b>🙂<b>čx" <b>
		--assert "<a>🙂" == find/skip "🙂<a><b>🙂<a>🙂" <a> 4

	--test-- "find bitset!"
		bits: charset #"x"
		--assert "xčá" == find "🙂čxčá" bits
		--assert "čá" == find/tail "🙂čxčá" bits
		bits: charset #"č"
		--assert "čxčá" == find "🙂čxčá" bits
		--assert "xčá" == find/tail "🙂čxčá" bits
		;bits: charset "🙂č"
		;--assert "🙂čxčá" == find "x🙂čxčá" bits
		;--assert "čxčá" == find/tail "x🙂čxčá" bits

	--test-- "find in binary!"
		bin: to binary! "ačb"
		--assert #{C48D62} == find bin #"č"
		--assert #{C48D62} == find bin #"č"
		--assert     #{62} == find bin #"b"
		--assert     #{62} == find bin #"b"
		--assert #{C48D62} == find bin "čb"
		--assert #{C48D62} == find bin "čb"

	--test-- "find in block!"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2706
		--assert did find [1 "ce"] "ce"
		--assert did find [1 "ce"] "Ce"
		--assert did find [1 "če"] "če"
		--assert did find [1 "če"] "Če"
		--assert did find/case [1 "ce"] "ce"
		--assert not find/case [1 "ce"] "Ce"
		--assert did find/case [1 "če"] "če"
		--assert not find/case [1 "če"] "Če"

	--test-- "pick"
		--assert #"á" == all [s: "áb" pick s 1]
		--assert #"b" == all [s: "áb" pick s 2]
		--assert #"🙂" == all [s: "🙂b" pick s 1]
		--assert #"b" == all [s: "🙂b" pick s 2]
	--test-- "pick with negative index"
		--assert #"á" == all [s: tail "áb" pick s -2]
		--assert #"b" == all [s: tail "áb" pick s -1]
		--assert #"🙂" == all [s: tail "🙂b" pick s -2]
		--assert #"b" == all [s: tail "🙂b" pick s -1]
	--test-- "pick over limits"
		--assert none? all [s: "áb" pick s 3]
		--assert none? all [s: "🙂b" pick s 3]
		--assert none? all [s: tail "áb" pick s -3]
		--assert none? all [s: tail "🙂b" pick s -3]

	--test-- "poke"
		--assert "šb" == all [s: "áb"  #"š" == poke s 1 #"š"  s]
		--assert "áš" == all [s: "áb"  #"š" == poke s 2 #"š"  s]
		--assert "šb" == all [s: "🙂b"  #"š" == poke s 1 #"š"  s]
		--assert "🙂š" == all [s: "🙂b"  #"š" == poke s 2 #"š"  s]

	--test-- "append string"
		--assert "abcš" == append "abc" "š"
		--assert "abc🙂" == append "abc" "🙂"
		--assert "ábčš" == append "ábč" "š"
		--assert "ábč🙂" == append "ábč" "🙂"

	--test-- "append char"
		--assert "abcš" == append "abc" #"š"
		--assert "abc🙂" == append "abc" #"🙂"
		--assert "ábčš" == append "ábč" #"š"
		--assert "ábč🙂" == append "ábč" #"🙂"

	--test-- "append to binary"
		--assert #{C3A1} == append #{} #"á"
		--assert #{F09F9982} == append #{} #"🙂"
		--assert #{C3A1F09F9982} == append #{} "á🙂"
		--assert #{C3A1} == append/part #{} "á" 1

	--test-- "insert string"
		--assert "šabc" == head insert "abc" "š"
		--assert "🙂abc" == head insert "abc" "🙂"
		--assert "ašbc" == head insert next "abc" "š"
		--assert "a🙂bc" == head insert next "abc" "🙂"

		--assert "šábč" == head insert "ábč" "š"
		--assert "🙂ábč" == head insert "ábč" "🙂"
		--assert "ášbč" == head insert next "ábč" "š"
		--assert "á🙂bč" == head insert next "ábč" "🙂"

	--test-- "insert/part string"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2659
		--assert all [(insert/part o: "éee" "a" 1) == "éee"  o == "aéee"]
		--assert all [(insert/part o: "éee" "á" 1) == "éee" o == "áéee"]
		--assert all [(insert/part o: "éee" "🙂" 1) == "éee"  o == "🙂éee"]
		--assert all [(insert/part o: "éee" "aaa" 2) == "éee"  o == "aaéee"]
		--assert all [(insert/part o: "éee" "ááá" 2) == "éee"  o == "ááéee"]
		--assert all [(insert/part o: "éee" "🙂🙂" 2) == "éee"  o == "🙂🙂éee"]
		--assert all [(insert/part o: "éee" "a" 20) == "éee"  o == "aéee"]
		--assert all [(insert/part o: "éee" "á" 20) == "éee"  o == "áéee"]
		--assert all [(insert/part o: "éee" "🙂" 20) == "éee"  o == "🙂éee"]

	--test-- "insert char"
		--assert "šábč" == head insert "ábč" #"š"
		--assert "🙂ábč" == head insert "ábč" #"🙂"
		--assert "ášbč" == head insert next "ábč" #"š"
		--assert "á🙂bč" == head insert next "ábč" #"🙂"

	--test-- "change string!"
		--assert all [(change o: "ábč" "č") == "bč"  o == "čbč"]
		--assert all [(change o: "abc" "🙂") == "bc"  o == "🙂bc"]
		--assert all [(change o: "ábč" "🙂") == "bč"  o == "🙂bč"]
		--assert all [(change o: "abc" "x🙂") == "c"  o == "x🙂c"]
		--assert all [(change o: "ábč" "x🙂") == "č"  o == "x🙂č"]
		--assert all [(change o: "🙂bc" "a") == "bc"  o == "abc"]
		--assert all [(change o: next "a🙂c" "b") == "c"  o == "bc"  "abc" == head o]

		--assert all [(change o: tail "abc" "🙂") == ""  o == "🙂" (head o) == "abc🙂"]
		--assert all [(change o: tail "ábč" "🙂") == ""  o == "🙂" (head o) == "ábč🙂"]
		--assert all [(change o: tail "abc" "x🙂") == ""  o == "x🙂" (head o) == "abcx🙂"]
		--assert all [(change o: tail "ábč" "x🙂") == ""  o == "x🙂" (head o) == "ábčx🙂"]

		--assert #{78787878}       == to binary! head change "═" "xxxx"
		--assert #{78787878}       == to binary! head change "═══" "xxxx"
		--assert #{78787878}       == to binary! head change "════" "xxxx"
		--assert #{78787878E29590} == to binary! head change "═════" "xxxx"

	--test-- "change/dup string!"
		--assert all [(change/dup o: "abc" "🙂" 2) == "c"  o == "🙂🙂c"]
		--assert all [(change/dup o: "abc" "🙂" 3) == ""   o == "🙂🙂🙂"]
		--assert all [(change/dup o: "ábč" "🙂" 2) == "č"  o == "🙂🙂č"]
		--assert all [(change/dup o: "ábč" "🙂" 3) == ""   o == "🙂🙂🙂"]
		--assert all [(change/dup o: "ábč" "🙂" 4) == ""   o == "🙂🙂🙂🙂"]
		--assert all [(change/dup o: "🙂bc" "a" 2) == "c"  o == "aac"]
		--assert all [(change/dup o: "🙂bc" "a" 3) == ""   o == "aaa"]
		--assert all [(change/dup o: "🙂bc" "a" 4) == ""   o == "aaaa"]

	--test-- "change/part string!"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2659
		--assert all [(change/part o: "éee" "a" 1) == "ee"  o == "aee"]
		--assert all [(change/part o: "éee" "á" 1) == "ee"  o == "áee"]
		--assert all [(change/part o: "éee" "🙂" 1) == "ee"  o == "🙂ee"]
		--assert all [(change/part o: "éee" "a" 2) == "e"  o == "ae"]
		--assert all [(change/part o: "éee" "á" 2) == "e"  o == "áe"]
		--assert all [(change/part o: "éee" "🙂" 2) == "e"  o == "🙂e"]
		--assert all [(change/part o: "éee" "a" 20) == ""  o == "a"]
		--assert all [(change/part o: "éee" "á" 20) == ""  o == "á"]
		--assert all [(change/part o: "éee" "🙂" 20) == ""  o == "🙂"]
	;@@ https://github.com/Oldes/Rebol-issues/issues/2665
		--assert all [(change/part o: next "éeeé" "🙂" 2) == "é"  o == "🙂é"  (head o) == "é🙂é"]

	--test-- "change char!"
		--assert all [(change o: "---" #"╔") == "--"  o == "╔--"]
		--assert all [(change o: "---" #"🙂") == "--"  o == "🙂--"]

	--test-- "take"
		--assert all [
			#"á" == take s: "áb"
			s == "b"
		]
		--assert all [
			#"🙂" == take s: "🙂b"
			s == "b"
		]
		--assert all [
			#"🙂" == take next s: "á🙂b"
			s == "áb"
		]

	--test-- "take/all"
		--assert "áb" == take/all "áb"
		--assert "🙂b" == take/all "🙂b"
		--assert "🙂b" == take/all next "á🙂b"

	--test-- "take/part"
		--assert all [
			"áb" == take/part s: "ábx" 2
			s == "x"
		]
		--assert all [
			"🙂b" == take/part s: "🙂bx" 2
			s == "x"
		]
		--assert all [
			"🙂b" == take/part next s: "á🙂bx" 2
			s == "áx"
		]

	--test-- "remove"
		--assert "b" == remove "áb"
		--assert "b" == remove "🙂b"
		--assert "b" == remove/part "áb" 1
		--assert "b" == remove/part "🙂b" 1
		--assert "b" == remove/part "xáb" 2
		--assert "b" == remove/part "x🙂b" 2
		--assert "x" == head remove/part tail "xáb" -2
		--assert "x" == head remove/part tail "x🙂b" -2

	--test-- "trim"
		--assert "á^/b^/🙂" == trim " á^/  b^/   🙂"
	--test-- "trim/head/tail"
		--assert "áb čb🙂^/" == trim/head "^- áb čb🙂^/"
		--assert "^-áb čb🙂^/" == head trim/head next "^- áb čb🙂^/"
		--assert "^- áb čb🙂" == trim/tail "^- áb čb🙂^/"
		--assert "áb čb🙂" == trim/head/tail "^- áb čb🙂^/"
	--test-- "trim/all"
		--assert "ábčb🙂" == trim/all "^- áb čb🙂^/"
	--test-- "trim/with"
		--assert "áč🙂" == trim/with "ábčb🙂" "b"
		--assert "áč🙂" == trim/with "ábčb🙂" #"b"
		--assert "bb🙂" == trim/with "ábčb🙂" "áč"
	--test-- "trim/lines"
		--assert "á b 🙂" == trim/lines " á^/  b^/   🙂"
	--test-- "trim/auto"
		--assert "á^/b^/ 🙂" == trim/auto "  á^/  b^/   🙂"
		--assert "á^/ b^/  🙂" == trim/auto next "  á^/  b^/   🙂"

	--test-- "swap"
		a: "ab" b: "á🙂"
		--assert all ["áb" == swap a b  a/1 == #"á"  b/1 == #"a"]
		--assert all ["🙂b" == swap a next b  a/1 == #"🙂"  b == "aá"]
		--assert all ["🙂b" == swap a tail b  a/1 == #"🙂"  b == "aá"]

	--test-- "random"
		s: "ábč🙂" random/seed 1
		--assert "🙂báč" == random s
		
	--test-- "random/only"
		s: "ábč🙂"
		--assert loop 10 [unless find s random/only s [break/return false] true]

	--test-- "sort"
		--assert "aAabč🙂" == sort "bač🙂Aa"
		--assert "Aaabč🙂" == sort/case "bač🙂Aa"
	--test-- "sort/skip"
		--assert "aa bb ba " == sort/skip "bb ba aa " 3
		--assert "aa b🙂 ba " == sort/skip "b🙂 ba aa " 3
	--test-- "sort/skip/all"
		--assert "aa ba bb " == sort/skip/all "bb ba aa " 3
		--assert "aa ba b🙂 " == sort/skip/all "b🙂 ba aa " 3
		--assert "Aa aa ab aá ba " == sort/skip/all/case "ab aa Aa aá ba " 3
	--test-- "sort/compare"
		comp: func [a b] [a > b]
		--assert "🙂čbaaA" == sort/compare "bač🙂Aa" :comp
		--assert "Aaabč🙂" == sort/compare/reverse "bač🙂Aa" :comp
		comp: func [a b] [a <= b]
		--assert "Aa aá aa ab ba " == sort/compare/skip     "ab aa Aa aá ba " :comp 3 ;; compares only the first char
		--assert "Aa aa ab aá ba " == sort/compare/skip/all "ab aa Aa aá ba " :comp 3
	--test-- "sort block of strings"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2716
		--assert ["abå" "cd" "ef"] == sort ["ef" "cd" "abå"]
		--assert ["abå" "cd" "ef"] == sort/unstable ["ef" "cd" "abå"]
		--assert ["ef" "cd" "abå"] == sort/reverse ["ef" "cd" "abå"]
		--assert ["ef" "cd" "abå"] == sort/reverse/unstable ["ef" "cd" "abå"]

===end-group===


===start-group=== "/part refines with utf8 strings"
	--test-- "copy/part"
		--assert "áb" == copy/part "ábč" 2
		--assert "ábč" == copy/part "ábč" 6
		--assert "" == copy/part "ábč" -6
		--assert all [a: "ábč" b: tail a "ábč" == copy/part a b]
	--test-- "copy/part tail"
		--assert "č" == copy/part tail "ábč" -1
		--assert "bč" == copy/part tail "ábč" -2
		--assert "ábč" == copy/part tail "ábč" -3
		--assert "ábč" == copy/part tail "ábč" -6
		--assert "" == copy/part tail "ábč" 6
		--assert "č" == copy/part back tail "ábč" 6
		--assert all [a: "ábč" b: next a "bč" == copy/part tail a b]

	--test-- "take/part"
		--assert all [s: "ábč"  "áb"  == take/part s 2  s == "č"]
		--assert all [s: "ábč"  "ábč" == take/part s 6  s == ""]
		--assert all [s: "ábč"  ""    == take/part s -6  s == "ábč"]
		--assert all [s: "ábč" b: next s "á" == take/part s b s == "bč"]
		--assert all [s: "ábč" b: tail s "ábč" == take/part s b s == ""]
	--test-- "take/part/last"
		;@@ https://github.com/Oldes/Rebol-issues/issues/2721
		--assert (take/last/part "monēre" 3) == "ēre"
		--assert all [s: "aábbcč" "č" == take/part/last s 1 s == "aábbc"]
		--assert all [s: "aábbcč" "cč" == take/part/last s 2 s == "aább"]
		--assert all [s: "aábbcč" "bcč" == take/part/last s 3 s == "aáb"]
		--assert all [s: "aábbcč" "bbcč" == take/part/last s 4 s == "aá"]
		--assert all [s: "aábbcč" "ábbcč" == take/part/last s 5 s == "a"]
		--assert all [s: "aábbcč" "aábbcč" == take/part/last s 6 s == ""]
	--test-- "take/part/last with string not at head"
		--assert  "c" == take/last/part next "abc" 1
		--assert  "č" == take/last/part next "ábč" 1
		--assert "bc" == take/last/part next "abc" 2
		--assert "bč" == take/last/part next "ábč" 2
		--assert "bc" == take/last/part next "abc" 3
		--assert "bč" == take/last/part next "ábč" 3

	--test-- "take/last" [
		--assert all [
			s: "aábbcč"
			x: copy ""
			while [c: take/last s][ append x c]
			x == "čcbbáa"
			s == ""
		]
	]
	--test-- "take/part tail"
		--assert all [s: "ábč"  "č" == take/part tail :s -1 s == "áb"]
		--assert all [s: "ábč"  "bč" == take/part tail :s -2 s == "á"]
		--assert all [s: "ábč"  "ábč" == take/part tail :s -3 s == ""]
		--assert all [s: "ábč"  "ábč" == take/part tail :s -6 s == ""]
		--assert all [s: "ábč"  "" == take/part tail :s 6 s == "ábč"]
		--assert all [s: "ábč"  "č" == take/part back tail s 6  s == "áb"]
		--assert all [s: "ábč" b: next s "bč" == take/part tail s b  s == "á"]

	--test-- "lowercase/part"
		--assert "ábČ" == lowercase/part "ÁBČ" 2
		--assert "Ábč" == head lowercase/part next "ÁBČ" 2
		--assert "Ábč" == head lowercase/part next "ÁBČ" 6
		--assert "Ábč" == head lowercase/part tail "ÁBČ" -2
	--test-- "uppercase/part"
		--assert "ÁBč" == uppercase/part "ábč" 2
		--assert "áBČ" == head uppercase/part next "ábč" 2
		--assert "áBČ" == head uppercase/part next "ábč" 6
		--assert "áBČ" == head uppercase/part tail "ábč" -2
===end-group===


===start-group=== "parse"
	--test-- "parse skip"
		--assert "áb" == parse "ábč" [return 2 skip]
		--assert "bč" == parse "ábč" [1 skip return to end]
	--test-- "parse set"
		--assert all [parse "ábč" [set x to end] x == #"á"]
	--test-- "parse copy"
		--assert all [parse "ábč" [copy x 2 skip to end] x == "áb"]
		--assert all [parse "🙂bč" [copy x 2 skip to end] x == "🙂b"]
		--assert all [parse "🙂bč" [skip copy x 2 skip] x == "bč"]

	--test-- "parse keep"
		--assert [#"š"] == parse "š" [collect [keep skip]]
		--assert ["áb"] == parse "áb🙂" [collect [keep 2 skip]]
		--assert ["áb" #"🙂"] == parse "áb🙂" [collect [keep 2 skip keep skip]]
		--assert [#"á" #"b" #"🙂"] == parse "áb🙂" [collect [keep skip keep skip keep skip]]
	--test-- "parse keep pick"
		--assert [#"á" #"b" #"🙂"] == parse "áb🙂" [collect some [keep pick 3 skip]]

	--test-- "parse collect set"
		--assert all [parse "áb🙂" [collect set blk [keep skip] to end]  blk == [#"á"]]
		--assert all [parse "áb🙂" [collect set blk [2 skip keep skip]]  blk == [#"🙂"]]
		--assert all [parse skip "áb🙂" 2 [collect set blk [keep skip]]  blk == [#"🙂"]]

	--test-- "parse collect set pick"
		--assert all [parse "áb🙂" [collect set blk [keep pick skip] to end]  blk == [#"á"]]
		--assert all [parse "áb🙂" [collect set blk [keep pick 3 skip]]       blk == [#"á" #"b" #"🙂"]]

	--test-- "parse collect into block"
		--assert all [blk: [] parse "š" [collect into blk [keep skip]] blk == [#"š"]]
		--assert all [blk: [] parse "áb🙂" [collect into blk [keep skip keep skip keep skip]] blk == [#"á" #"b" #"🙂"]]

	--test-- "parse collect into string"
		--assert all [str: "" parse "š" [collect into str [keep skip]] str == "š"]
		--assert all [str: "" parse "áb🙂" [collect into str [keep skip keep skip keep skip]] str == "áb🙂"]
		--assert all [str: "" parse "áb🙂" [collect into str [2 skip keep skip]] str == "🙂"]
		--assert all [str: "X" parse "š" [collect into str [keep skip]] str == "šX"]
		--assert all [str: "X" parse "áb🙂" [collect into str [keep skip keep skip keep skip]] str == "áb🙂X"]
		--assert all [str: "X" parse "áb🙂" [collect into str [2 skip keep skip]] str == "🙂X"]

	--test-- "parse collect any"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2537
		--assert [#"á" "11" #"🙂" "22"] == parse "á11🙂22" [collect any [keep skip keep 2 skip]]
		--assert all [parse "á11🙂22" [collect set o any [keep skip keep 2 skip]]  o == [#"á" "11" #"🙂" "22"]]
		--assert all [o: "" parse "á11🙂22" [collect into o any [keep skip 2 skip]]  o == "á🙂"]
		--assert all [o: "X" parse "á11🙂22" [collect into o any [keep skip 2 skip]]  o == "á🙂X"]

	--test-- "parse collect after string"
		--assert all [str: "XX" parse "á11🙂22" [collect after str any [keep skip 2 skip]]  str == "XXá🙂"]
		--assert all [str: next "XX" parse "á11🙂22" [collect after str any [keep skip 2 skip]]  str == "Xá🙂"]

	--test-- "parse collect from block"
		--assert [["áb" #"🙂"]] == parse ["áb" #"🙂"] [collect [keep 2 skip]]
		--assert [ "áb" #"🙂" ] == parse ["áb" #"🙂"] [collect [keep pick 2 skip]]

		--assert all [parse ["áb" #"🙂"] [collect set o [keep 2 skip]]       o == [["áb" #"🙂"]] ]
		--assert all [parse ["áb" #"🙂"] [collect set o [keep pick 2 skip]]  o == [ "áb" #"🙂" ] ]

	--test-- "parse with bitset"
		bits: charset "á🙂"
		--assert parse "á🙂" [some bits]
		--assert "xx" == parse "á🙂xx" [some bits return to end]
		--assert [#"á" #"🙂"] == parse "áxx🙂xx" [collect some [keep some bits 2 skip]]
		not-bits: complement bits
		--assert [#"á" #"🙂"] == parse "áxx🙂x" [collect some [keep some bits some not-bits]]

	--test-- "parse with chars"
		--assert parse "á" [#"á"]
		--assert parse "á🙂" [#"á" #"🙂"]
		--assert parse "á🙂" [thru #"🙂"]
		--assert parse "á🙂" [to #"🙂" skip]

	--test-- "parse with string"
		--assert parse "á" ["á"]
		--assert parse "á🙂" ["á" "🙂"]
		--assert parse "á🙂" [thru "🙂"]
		--assert parse "á🙂" [to "🙂" skip]

	--test-- "parse with tag"
		--assert parse "<á>" [<á>]
		--assert parse "<á🙂>" [thru <á🙂>]
		--assert parse "<á🙂>" [to <á🙂> to end]

	--test-- "parse skip"
		--assert parse "핕" [skip]


===end-group===

===start-group=== "char column (terminal) width"
	--test-- "zero width chars"
		c: #"^(02)"
		--assert 0 = c/width
		c: #"^(82)"
		--assert 0 = c/width
		c: #"^(200b)"
		--assert 0 = c/width
		s: "a​b"
		--assert 1 = s/1/width
		--assert 0 = s/2/width
		--assert 1 = s/3/width
	--test-- "wide chars"
		c: #"🙂"
		--assert 2 = c/width
		s: "a⚡中"
		--assert 1 = s/1/width
		--assert 2 = s/2/width
		--assert 2 = s/3/width
===end-group===

===start-group=== "char utf8 size (number of bytes)"
	--test-- "zero width chars"
		c: #"^(200b)"
		--assert 3 = c/size
		s: "a​b"
		--assert 1 = s/1/size
		--assert 3 = s/2/size
		--assert 1 = s/3/size
	--test-- "wide chars"
		c: #"🙂"
		--assert 4 = c/size
		s: "a⚡中"
		--assert 1 = s/1/size
		--assert 3 = s/2/size
		--assert 3 = s/3/size
===end-group===

===start-group=== "string column (terminal) width"
	--test-- "zero width chars"
		s: "a​b"
		--assert 2 = s/width
		s: next s
		--assert 1 = s/width
		s: next s
		--assert 1 = s/width
		s: next s
		--assert 0 = s/width
	--test-- "wide chars"
		s: "a⚡中"
		--assert 5 = s/width
		s: next s
		--assert 4 = s/width
		s: next s
		--assert 2 = s/width
		s: next s
		--assert 0 = s/width
===end-group===

===start-group=== "string utf8 size (number of bytes)"
	--test-- "zero width chars"
		s: "a​b"
		--assert 5 = s/size
		s: next s
		--assert 4 = s/size
		s: next s
		--assert 1 = s/size
		s: next s
		--assert 0 = s/size
	--test-- "wide chars"
		s: "a⚡中"
		--assert 7 = s/size
		s: next s
		--assert 6 = s/size
		s: next s
		--assert 3 = s/size
		s: next s
		--assert 0 = s/size
===end-group===

===start-group=== "string codepoints length"
	--test-- "zero width chars"
		s: "a​b"
		--assert 3 = s/length
		s: next s
		--assert 2 = s/length
		s: next s
		--assert 1 = s/length
		s: next s
		--assert 0 = s/length
	--test-- "wide chars"
		s: "a⚡中"
		--assert 3 = s/length
		s: next s
		--assert 2 = s/length
		s: next s
		--assert 1 = s/length
		s: next s
		--assert 0 = s/length
===end-group===
~~~end-file~~~
