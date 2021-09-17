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
		--assert attempt [(keys-of p1/objects) = (keys-of p2/objects)]
		try [delete %tmp.pdf]
	]

	--test-- "Minimal PDF file"
		empty-pdf: object [
			trailer: #(
				Info: 1x0
				Root: 2x0
			)
			objects: #(
				;- Document Information Dictionary
				1x0 #(
					Producer: "Rebol"
				)
				;- Document Catalog
				2x0 #(
					Type: Catalog
					Pages: 3x0
				)
				;- Root of the document's page tree
				3x0 #(
					Type: Pages
					Kids: [5x0]
					Count: 1
				)
				;- Procedure Sets
				4x0 [
					PDF    ; Painting and graphics state
				]
				;- First page
				5x0 #(
					Type: Page
					Parent: 3x0
					MediaBox: [0 0 612 792]
					Contents: 6x0
					Resources: #(
						ProcSet: 4x0
					)
				)
				;- Empty content
				6x0 #[object! [
					data: ""
				]]
			)
		]
		--assert not error? try [save %tmp-empty.pdf pdf]
		--assert object?    try [p2: load %tmp-empty.pdf]
		--assert attempt [(keys-of empty-pdf/objects) = (keys-of p2/objects)]
		try [delete %tmp-empty.pdf]

	--test-- "Save Simple Text PDF"
		pdf: copy/deep empty-pdf
		; include text processing..
		append pdf/objects/4x0 'Text
		; include font
		put pdf/objects 7x0 #(
			Type:     Font
			Subtype:  Type1
			Name:     F1
			BaseFont: Helvetica
			Encoding: MacRomanEncoding
		)
		pdf/objects/5x0/Resources/Font: #(F1: 7x0)
		; make some content
		pdf/objects/6x0/Data: {
			BT
				/F1 12 Tf 0 0 Td (0x0) Tj
				/F1 24 Tf 100 750 Td (Hello World) Tj
			ET
		}
		--assert not error? try [save %tmp-text.pdf pdf]
		--assert object?    try [p2: load %tmp-text.pdf]
		--assert attempt [(keys-of pdf/objects) = (keys-of p2/objects)]
		try [delete %tmp-text.pdf]


	===end-group===
]

~~~end-file~~~