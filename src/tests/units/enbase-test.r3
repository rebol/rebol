Rebol [
	Title:   "Rebol3 enbase/debase test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %enbase-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "enbase"

===start-group=== "enbase basic"
	;from https://tools.ietf.org/html/rfc4648#section-10
	--test-- "enbase-1" --assert ""         = enbase "" 64
	--test-- "enbase-2" --assert "Zg=="     = enbase "f" 64
	--test-- "enbase-3" --assert "Zm8="     = enbase "fo" 64
	--test-- "enbase-4" --assert "Zm9v"     = enbase "foo" 64
	--test-- "enbase-5" --assert "Zm9vYg==" = enbase "foob" 64
	--test-- "enbase-6" --assert "Zm9vYmE=" = enbase "fooba" 64
	--test-- "enbase-7" --assert "Zm9vYmFy" = enbase "foobar" 64

	--test-- "enbase-8"  --assert ""             = enbase "" 16
	--test-- "enbase-9"  --assert "66"           = enbase "f" 16
	--test-- "enbase-10" --assert "666F"         = enbase "fo" 16
	--test-- "enbase-11" --assert "666F6F"       = enbase "foo" 16
	--test-- "enbase-12" --assert "666F6F62"     = enbase "foob" 16
	--test-- "enbase-13" --assert "666F6F6261"   = enbase "fooba" 16
	--test-- "enbase-14" --assert "666F6F626172" = enbase "foobar" 16
===end-group===

===start-group=== "enbase with position not at head"
	;@@ https://github.com/Oldes/Rebol-issues/issues/319
	bin: #{FF00FF00}
	--test-- "enbase-2"  --assert #{00FF00} = debase enbase next bin 2 2
	--test-- "enbase-16" --assert #{00FF00} = debase enbase next bin 16 16
	--test-- "enbase-64" --assert #{00FF00} = debase enbase next bin 64 64
===end-group===

===start-group=== "debase 64"
	--test-- "debase 64 1"          
		--assert strict-equal? "A simple string" to string! debase "QSBzaW1wbGUgc3RyaW5n" 64
	--test-- "debase 64 2"          
		--assert strict-equal? "A multi-line\nstring" to string! debase "QSBtdWx0aS1saW5lXG5zdHJpbmc=" 64   
    
===end-group===

===start-group=== "debase 64 - safe URL variant"

	--test-- "debase 64 url 1" 
		--assert binary? try [debase/url {
			cC4hiUPoj9Eetdgtv3hF80EGrhuB__dzERat0XF9g2VtQgr9PJbu3XOiZj5RZmh7
			AAuHIm4Bh-0Qc_lF5YKt_O8W2Fp5jujGbds9uJdbF9CUAr7t1dnZcAcQjbKBYNX4
			BAynRFdiuB--f_nZLgrnbyTyWzO75vRK5h6xBArLIARNPvkSjtQBMHlb1L07Qe7K
			0GarZRmB_eSN9383LcOLn6_dO--xi12jzDwusC-eOkHWEsqtFZESc6BfI7noOPqv
			hJ1phCnvWh6IeYI2w9QOYEUipUTI8np6LbgGY9Fs98rqVt5AXLIhWkWywlVmtVrB
			p0igcN_IoypGlUPQGe77Rw
		} 64]
		--assert binary? try [debase {
			cC4hiUPoj9Eetdgtv3hF80EGrhuB__dzERat0XF9g2VtQgr9PJbu3XOiZj5RZmh7
			AAuHIm4Bh-0Qc_lF5YKt_O8W2Fp5jujGbds9uJdbF9CUAr7t1dnZcAcQjbKBYNX4
			BAynRFdiuB--f_nZLgrnbyTyWzO75vRK5h6xBArLIARNPvkSjtQBMHlb1L07Qe7K
			0GarZRmB_eSN9383LcOLn6_dO--xi12jzDwusC-eOkHWEsqtFZESc6BfI7noOPqv
			hJ1phCnvWh6IeYI2w9QOYEUipUTI8np6LbgGY9Fs98rqVt5AXLIhWkWywlVmtVrB
			p0igcN_IoypGlUPQGe77Rw
		} 64]
	--test-- "debase 64 url 2 - missing padding in the input" 
		;should be ok with the padding
		--assert binary? try [debase {
		    eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFt
		    cGxlLmNvbS9pc19yb290Ijp0cnVlfQ==
		} 64]
		;should throw error when missing padding and not /url refinement is used
		--assert error? try [debase {
		    eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFt
		    cGxlLmNvbS9pc19yb290Ijp0cnVlfQ
		} 64]
		--assert binary? try [debase/url {
		    eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFt
		    cGxlLmNvbS9pc19yb290Ijp0cnVlfQ
		} 64]
	--test-- "debase 64 url 3"
		key1: "qL8R4QIcQ_ZsRqOAbeRfcZhilN_MksRtDaErMA=="
		bin: try [debase/url key1 64]
		--assert true? all [binary? bin key1 = enbase/url bin 64]
		;debase is working also when input is missing the padding
		key2: "qL8R4QIcQ_ZsRqOAbeRfcZhilN_MksRtDaErMA"
		--assert bin = try [debase/url key2 64]

===end-group===

===start-group=== "debase 16"

	--test-- "debase 16 1"          
		--assert strict-equal? 
			"A simple string" 
			to string! debase "412073696d706c6520737472696e67" 16
	--test-- "debase 16 2"          
		--assert strict-equal? 
			"A multi-line\nstring" 
			to string! debase "41206d756c74692d6c696e655c6e737472696e67" 16
	--test-- "debase other strings"
	;@@ https://github.com/Oldes/Rebol-issues/issues/2453
		--assert #{1234} = debase "1234" 16
		--assert #{1234} = debase <1234> 16
		--assert #{1234} = debase @1234  16
		--assert #{01}   = debase @00000001 2


===end-group===

===start-group=== "debase 2"

	--test-- "debase 2 1"          
		--assert strict-equal? 
			"^(04)^(01)" 
			to string! debase "0000010000000001" 2

===end-group===

===start-group=== "enbase/part"
	--test-- "enbase/part 2"
		--assert ""  = enbase/part #{0401} 2 0
		--assert "00000100"  = enbase/part #{0401} 2 1
		--assert "0000010000000001"  = enbase/part #{0401} 2 2
		--assert "0000010000000001"  = enbase/part #{0401} 2 3
		--assert "0000010000000001"  = enbase/part #{0401FF} 2 2
		--assert "00000001"  = enbase/part skip #{0401FF} 1 2 1
	--test-- "enbase/part 16"
		--assert ""  = enbase/part #{0401} 16 0
		--assert "04"  = enbase/part #{0401} 16 1
		--assert "0401"  = enbase/part #{0401} 16 2
		--assert "0401"  = enbase/part #{0401} 16 3
		--assert "0401"  = enbase/part #{0401FF} 16 2
		--assert "01"  = enbase/part skip #{0401FF} 1 16 1
	--test-- "enbase/part 64"
		--assert ""  = enbase/part #{666F6F666F6F} 64 0
		--assert "Zm9v"  = enbase/part #{666F6F666F6F} 64 3
		--assert "Zm9vZm9v"  = enbase/part #{666F6F666F6F} 64 6
		--assert "Zm9v"  = enbase/part skip #{666F6F666F6F} 3 64 3
		--assert "Zm9v"  = enbase/part "foofoo" 64 3
		--assert "Zm9vZm9v"  = enbase/part "foofoo" 64 6
		--assert "Zm9v"  = enbase/part "foošřž" 64 3
		--assert "Zm9v"  = enbase/part skip "šřžfoo" 3 64 3
===end-group===

===start-group=== "debase/part"
	--test-- "debase/part 2"
		--assert #{}  = debase/part "0000010000000001" 2 0
		--assert #{04} = debase/part "0000010000000001" 2 8
		--assert #{0401} = debase/part "0000010000000001" 2 16
		--assert #{0401} = debase/part "0000010000000001" 2 32
		--assert #{01} = debase/part skip "0000010000000001" 8 2 8
		--assert #{01} = debase/part skip "čččččččč00000001" 8 2 8
	--test-- "debase/part 16"
		--assert #{}  = debase/part "01020304" 16 0
		--assert #{01} = debase/part "01020304" 16 2
		--assert #{010203} = debase/part "01020304" 16 6
		--assert #{01020304} = debase/part "01020304" 16 100
		--assert #{02} = debase/part skip "01020304" 2 16 2
		--assert #{02} = debase/part skip "čč020304" 2 16 2
	--test-- "debase/part 64"
		--assert #{}  = debase/part "Zm9vZm9v" 64 0
		--assert #{666F6F}  = debase/part "Zm9vZm9v" 64 4
		--assert #{666F6F666F6F}  = debase/part "Zm9vZm9v" 64 8
		--assert #{666F6F666F6F}  = debase/part "Zm9vZm9v!!!" 64 8
		--assert #{666F6F666F6F}  = debase/part "Zm9vZm9vščř" 64 8
===end-group===

if any [
	not error? err: try [enbase "a" 85]
	err/id <> 'feature-na
][
	base85-str-tests: [
		""             ""
		"h"            "BE"
		"he"           "BOq"
		"hel"          "BOtu"
		"hell"         "BOu!r"
		"hello"        "BOu!rDZ"
		"hello "       "BOu!rD]f"
		"hello w"      "BOu!rD]j6"
		"hello wo"     "BOu!rD]j7B"
		"hello wor"    "BOu!rD]j7BEW"
		"hello worl"   "BOu!rD]j7BEbk"
		"hello world"  "BOu!rD]j7BEbo7"
		"hello world!" "BOu!rD]j7BEbo80"
	]
	base85-bin-tests: [
		#{00}                 {!!}
		#{0000}               {!!!}
		#{000000}             {!!!!}
		#{00000000}           {z}
		#{0000000000000000}   {zz}
		#{000000000000000000} {zz!!}
		#{0100000000}         {!<<*"!!}
		#{ffd8ffe0}           {s4IA0}
		#{ffffffff}           {s8W-!}
	]
	base85-spaces-tests: [
		#{68}         "B E"
		#{68}         "B^-E"
		#{68}         "B^/E"
		#{68}         "B^ME"
		#{68656C6C6F} "BOu!rDZ "
		#{68656C6C6F} "BOu!rDZ^-"
		#{68656C6C6F} "BOu!rDZ^/"
		#{68656C6C6F} "BOu!rDZ^M"
	]

	
	===start-group=== "enbase-85"
		--test-- "enbase str 85"    
			foreach [inp out] base85-str-tests [
				--assert out = enbase inp 85
			]
		--test-- "enbase bin 85"    
			foreach [inp out] base85-bin-tests [
				--assert out = enbase inp 85
			]
		--test-- "enbase/part 85"
			--assert "BOu!rDZ" = enbase/part "hello world!" 85 5

	===end-group===

	===start-group=== "debase-85"
		--test-- "debase str 85"    
			foreach [out inp] base85-str-tests [
				--assert out = to-string debase inp 85
			]
		--test-- "debase 85 with spaces"
			foreach [out inp] base85-spaces-tests [
				--assert out = debase inp 85
			]
		--test-- "invalid debase85 input"
			--assert error? try [debase "abcx"  85]
			--assert error? try [debase "~>"    85]
			--assert error? try [debase {s8W-"} 85]
		--test-- "debase/part 85"
			--assert #{68656C6C6F} = debase/part "BOu!rDZ!!!!!" 85 7
	===end-group===
]

~~~end-file~~~