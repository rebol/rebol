Rebol [
	Title:   "Rebol3 enbase/debase test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %enbase-test.red
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "enbase"

===start-group=== "enbase basic"
	;from https://tools.ietf.org/html/rfc4648#section-10
	--test-- "enbase-1" --assert "" = enbase ""
	--test-- "enbase-2" --assert "Zg==" = enbase "f"
	--test-- "enbase-3" --assert "Zm8=" = enbase "fo"
	--test-- "enbase-4" --assert "Zm9v" = enbase "foo"
	--test-- "enbase-5" --assert "Zm9vYg==" = enbase "foob"
	--test-- "enbase-6" --assert "Zm9vYmE=" = enbase "fooba"
	--test-- "enbase-7" --assert "Zm9vYmFy" = enbase "foobar"

	--test-- "enbase-8"  --assert ""             = enbase/base "" 16
	--test-- "enbase-9"  --assert "66"           = enbase/base "f" 16
	--test-- "enbase-10" --assert "666F"         = enbase/base "fo" 16
	--test-- "enbase-11" --assert "666F6F"       = enbase/base "foo" 16
	--test-- "enbase-12" --assert "666F6F62"     = enbase/base "foob" 16
	--test-- "enbase-13" --assert "666F6F6261"   = enbase/base "fooba" 16
	--test-- "enbase-14" --assert "666F6F626172" = enbase/base "foobar" 16
===end-group===

===start-group=== "debase 64"

	--test-- "debase 64 1"          
		--assert strict-equal? "A simple string" to string! debase "QSBzaW1wbGUgc3RyaW5n"
	--test-- "debase 64 2"          
		--assert strict-equal? "A multi-line\nstring" to string! debase "QSBtdWx0aS1saW5lXG5zdHJpbmc="
	--test-- "debase 64 3"          
		--assert strict-equal? "A simple string" to string! debase/base "QSBzaW1wbGUgc3RyaW5n" 64
	--test-- "debase 64 4"          
		--assert strict-equal? "A multi-line\nstring" to string! debase/base "QSBtdWx0aS1saW5lXG5zdHJpbmc=" 64   
    
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
		}]
		--assert binary? try [debase {
			cC4hiUPoj9Eetdgtv3hF80EGrhuB__dzERat0XF9g2VtQgr9PJbu3XOiZj5RZmh7
			AAuHIm4Bh-0Qc_lF5YKt_O8W2Fp5jujGbds9uJdbF9CUAr7t1dnZcAcQjbKBYNX4
			BAynRFdiuB--f_nZLgrnbyTyWzO75vRK5h6xBArLIARNPvkSjtQBMHlb1L07Qe7K
			0GarZRmB_eSN9383LcOLn6_dO--xi12jzDwusC-eOkHWEsqtFZESc6BfI7noOPqv
			hJ1phCnvWh6IeYI2w9QOYEUipUTI8np6LbgGY9Fs98rqVt5AXLIhWkWywlVmtVrB
			p0igcN_IoypGlUPQGe77Rw
		}]
	--test-- "debase 64 url 2 - missing padding in the input" 
		;should be ok with the padding
		--assert binary? try [debase {
		    eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFt
		    cGxlLmNvbS9pc19yb290Ijp0cnVlfQ==
		}]
		;should throw error when missing padding and not /url refinement is used
		--assert error? try [debase {
		    eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFt
		    cGxlLmNvbS9pc19yb290Ijp0cnVlfQ
		}]
		--assert binary? try [debase/url {
		    eyJpc3MiOiJqb2UiLA0KICJleHAiOjEzMDA4MTkzODAsDQogImh0dHA6Ly9leGFt
		    cGxlLmNvbS9pc19yb290Ijp0cnVlfQ
		}]
	--test-- "debase 64 url 3"
		key1: "qL8R4QIcQ_ZsRqOAbeRfcZhilN_MksRtDaErMA=="
		bin: try [debase/url key1]
		--assert true? all [binary? bin key1 = enbase/url bin]
		;debase is working also when input is missing the padding
		key2: "qL8R4QIcQ_ZsRqOAbeRfcZhilN_MksRtDaErMA"
		--assert bin = try [debase/url key2]

===end-group===

===start-group=== "debase 16"

	--test-- "debase 16 1"          
		--assert strict-equal? 
			"A simple string" 
			to string! debase/base "412073696d706c6520737472696e67" 16
	--test-- "debase 16 2"          
		--assert strict-equal? 
			"A multi-line\nstring" 
			to string! debase/base "41206d756c74692d6c696e655c6e737472696e67" 16

===end-group===

===start-group=== "debase 2"

	--test-- "debase 2 1"          
		--assert strict-equal? 
			"^(04)^(01)" 
			to string! debase/base "0000010000000001" 2

===end-group===

~~~end-file~~~