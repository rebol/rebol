Rebol [title: "Test script"]

data: transcode sys/read-decode %tmp.data none
either find data/2 #"^@" [
	prin "NULL found! Test FAILED!"
][	prin "Test OK"]
