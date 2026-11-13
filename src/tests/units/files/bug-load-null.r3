Rebol [title: "Test script"]
print ["[bug-load-null.r3] dir:"  what-dir]
data: transcode sys/read-decode %tmp.data none
either find data/2 #"^@" [
	prin "NULL found! Test FAILED!"
][	prin "Test OK"]
