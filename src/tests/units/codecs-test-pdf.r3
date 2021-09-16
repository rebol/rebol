Rebol [
	Title:   "Rebol codecs PDF test script"
	Author:  "Oldes"
	File: 	 %codecs-test-pdf.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "Codecs PDF"


if find codecs 'pdf [
	===start-group=== "PDF codec"
	foreach file [
		%hello-1.pdf
		%hello-2.pdf
		%hello-linearized.pdf
		%pdf-maker-doc.pdf
		%flower-from-photoshop.pdf
		%flower-from-xnview.pdf
	][
		--test-- join "load/save: " file
		--assert object? try [p1: load join %units/files/ file]
		--assert not error?  try [save %tmp.pdf p1]
		--assert object? try [p2: load %tmp.pdf]
		--assert (keys-of p1/objects) = (keys-of p2/objects)
		try [delete %tmp.pdf]
	]

	===end-group===
]

~~~end-file~~~