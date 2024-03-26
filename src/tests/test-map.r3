Rebol [
	Title:    "MAP hashing performance tests"
	Purpose:  "Counts multiple histograms with different key types to count map key insert/select performance"
	Date:     21-Mar-2024
	Author:   "Oldes"
	File:     %test-map.r3
	Version:  1.1.0
]

if system/version < 3.16.1 [
	print as-purple "*** This Rebol version does not have hash collision counter! ***"
]

form-time: function[dt][
    dt: 1000.0 * to decimal! dt
    unit: either dt < 1 [dt: dt * 1000.0 " μs"] [" ms"] 
    parse form dt [
        0 3 [opt #"." skip] opt [to #"."] dt: (dt: head clear dt)
    ]
    ajoin [dt unit]
]
print-result: function [count time coll data][
	printf [
		"Iterations: " $32.1  8 $0
		"Keys: "       $32.1  7 $0
		"Collisions: " $32.1 10 $0
		"Time: "       $32.1  7 $32.2 -10 $0
		
	] reduce [
		count
		length? data
		init-collisions - coll
		form-time time
		form-time time / count
		
	]
	? data
	data
]
init-collisions: function[][
	stat: stats/profile
	any [select stat 'collisions 0]
]
;-------------------------------------------------------------------------

test-tuple-map: function[img [image!]][
	recycle
	print as-yellow "^/Testing map with tuple keys..."
	n: to integer! img/size/x * img/size/y
	data: make map! n
	time: 0:0:0
	coll: init-collisions
	foreach clr img [
		start: stats/timer
		data/:clr: 1 + any [data/:clr 0]
		time: time + stats/timer - start
	]
	print-result n time coll
	data
]
test-integer-map: function[img [image!]][
	recycle
	print as-yellow "^/Testing map with integer keys..."
	n: to integer! img/size/x * img/size/y
	data: make map! n
	time: 0:0:0
	coll: init-collisions
	foreach clr img [
		clr: to integer! to binary! clr
		start: stats/timer
		data/:clr: 1 + any [data/:clr 0]
		time: time + stats/timer - start
	]
	print-result n time coll
	data
]

test-binary-map: function [text][
	recycle
	print as-yellow "^/Testing map with binary keys..."
	data: make map! 30000
	char: system/catalog/bitsets/alpha
	time: 0:0:0
	coll: init-collisions
	n: 0
	parse to binary! text [any [
		to char copy word: some char (
			start: stats/timer
			data/:word: 1 + any [data/:word 0]
			end: stats/timer
			time: time + end - start
			++ n
		)
	]]
	print-result n time coll
	data
]
test-string-map: function [text][
	recycle
	print as-yellow "^/Testing map with string keys..."
	data: make map! 30000
	char: system/catalog/bitsets/alpha
	time: 0:0:0
	coll: init-collisions
	n: 0
	parse text [any [
		to char copy word: some char (
			start: stats/timer
			data/:word: 1 + any [data/:word 0]
			end: stats/timer
			time: time + end - start
			++ n
		)
	]]
	print-result n time coll
	data
]
test-word-map: function [text][
	recycle
	print as-yellow "^/Testing map with word keys..."
	data: make map! 30000
	char: system/catalog/bitsets/alpha
	time: 0:0:0
	coll: init-collisions
	n: 0
	parse text [any [
		to char copy word: some char (
			word: to word! word
			start: stats/timer
			data/:word: 1 + any [data/:word 0]
			end: stats/timer
			time: time + end - start
			++ n
		)
	]]
	print-result n time coll
	data
]
test-block-map: function [text][
	recycle
	print as-yellow "^/Testing map with block keys..."
	print "Probably useless, but why not to test it too."
	print "With old hashing this takes REALLY LONG TIME!"
	data: make map! 30000
	char: system/catalog/bitsets/alpha
	time: 0:0:0
	coll: init-collisions
	n: 0
	parse text [any [
		to char copy word: some char (
			blk: reduce [to word! word]
			start: stats/timer
			data/:blk: 1 + any [data/:blk 0]
			end: stats/timer
			time: time + end - start
			++ n
		)
	]]
	print-result n time coll
	data
]
test-char-map: function [text][
	recycle
	print as-yellow "^/Testing map with char keys..."
	data: make map! 100
	char: system/catalog/bitsets/alpha
	time: 0:0:0
	coll: init-collisions
	n: 0
	parse text [any [
		to char set key: skip (
			start: stats/timer
			data/:key: 1 + any [data/:key 0]
			end: stats/timer
			time: time + end - start
			++ n
		)
	]]
	print-result n time coll
	data
]

test-char-map-2: function [text][
	recycle
	print as-yellow "^/Testing map with char keys (group words by first letter)"
	data: make map! 100
	char: system/catalog/bitsets/alpha
	time: 0:0:0
	coll: init-collisions
	n: 0
	parse text [any [
		to char copy word: some char (
			ch: word/1
			start: stats/timer
			append data/:ch: any [data/:ch copy []] word
			end: stats/timer
			time: time + end - start
			++ n
		)
	]]
	foreach [k v] data [deduplicate v]
	print-result n time coll
	data
]



img:  %units/files/koule.jpg
text: %units/files/pg100.txt

;- Using an image to test tuple and integer keys
unless exists? img [
	print as-purple "Downloading image data..."
	write img read https://raw.githubusercontent.com/Siskin-Framework/Rebol-BlurHash/main/test/koule.jpg
]

;- Using The Complete Works of William Shakespeare as text data
unless exists? text [
	print as-purple "Downloading text data..."
	write text read https://www.gutenberg.org/cache/epub/100/pg100.txt
]

img:  load img
text: load text

profile/times [[test-tuple-map   img]] 3
profile/times [[test-integer-map img]] 3
profile/times [[test-binary-map text]] 3
profile/times [[test-string-map text]] 3
profile/times [[test-word-map   text]] 3
profile/times [[test-block-map  copy/part text 100000]] 3 ;limit size of the text as it is really slow with old hashing
profile/times [[test-char-map   text]] 3
profile/times [[test-char-map-2 text]] 3


()