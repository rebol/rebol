REBOL [
	Name:    bbcode
	Type:    module
	Options: [delay]
	Version: 0.3.4
	Title:   "Codec: BBcode"
	Purpose: {Basic BBCode implementation. For more info about BBCode check http://en.wikipedia.org/wiki/BBCode}
	File:    https://raw.githubusercontent.com/Oldes/Rebol3/master/src/mezz/codec-bbcode.reb
	Date:    13-Dec-2023
	Author:  "Oldes"
	History: [
		0.1.0  5-Jan-2009 "initial version"
		0.2.0 19-Feb-2012 "review"
		0.2.1 22-Aug-2012 "added [hr] and [anchor]"
		0.3.0 24-Apr-2020 "ported to Rebol3"
		0.3.1 11-Dec-2023 "FIX: `bbcode` must accept only string input"
		0.3.2 12-Dec-2023 "FEAT: csv table emitter"
		0.3.3 13-Dec-2023 "FEAT: image gallery emitter"
	]
]

opened-tags: copy []
allow-html-tags?: false
attr: copy ""
short-attr: copy ""
attributes: make map! 20
html: copy ""
tmp: pos: none

;--------------------
;- charsets & rules -
;--------------------

ch_space:      #[bitset! #{7FFFFFFF800000000000000000000001}]    ; charset [#"^A" - #" " #"^(7F)"]
ch_normal:     #[bitset! [not bits #{002400000000000800000010}]] ; complement charset "[<^M^/"
ch_attribute:  #[bitset! [not bits #{000000002100000A00000004}]] ; complement charset {"'<>]}
ch_attribute1: #[bitset! [not bits #{000000000100000A00000004}]] ; complement charset {'<>]}
ch_attribute2: #[bitset! [not bits #{000000002000000A00000004}]] ; complement charset {"<>]}
ch_attribute3: #[bitset! [not bits #{000000008000000A00000004}]] ; complement charset { <>]}
ch_digits: charset [#"0" - #"9"]
ch_hexa:   charset [#"a" - #"f" #"A" - #"F" #"0" - #"9"]
ch_name:   charset [#"a" - #"z" #"A" - #"Z" #"*" #"0" - #"9"]
ch_url:    charset [#"a" - #"z" #"A" - #"Z" #"0" - #"9" "./:~+-%#\_=&?@"]
ch_crlf:   charset CRLF
ch_safe-value-chars: complement charset {'"}

rl_newline: [CRLF | LF]
rl_attribute: [
	(clear short-attr)
	any ch_space #"=" any ch_space [
		  #"'"  copy short-attr any ch_attribute1 #"'"
		| #"^"" copy short-attr any ch_attribute2 #"^""
		|       copy short-attr any ch_attribute3
	] any ch_space
]
rl_attributes: [
	(clear attributes)
	opt rl_attribute
	any [
		any ch_space
		copy tmp some ch_name any ch_space #"=" any ch_space [
			  #"^"" copy attr any ch_attribute2 #"^""
			| #"'"  copy attr any ch_attribute1 #"'"
			|       copy attr any ch_attribute3
		] any ch_space
		(
			put attributes tmp attr
			tmp: attr: none
		)
	]
]

	
get-attribute: func[name /default value /local tmp][
	all [
		tmp: pick attributes name
		tmp: encode-value tmp
		default
		try [tmp: to type? value tmp]
	]
	any [tmp value]	
]

form-attribute: func[name /default value][
	either value: either default [
		get-attribute/default name value
	][	get-attribute name ][
		rejoin [#" " name {="} value {"}]
	][	""]
]

encode-value: func[value [any-string!] /local out tmp][
	out: copy ""
	parse value [
		any [
			;pos: ;(probe pos)
			[ 
				#"'"  (append out "&apos;") |
				#"^"" (append out "&quot;")
			]
			|
			copy tmp some ch_safe-value-chars (append out tmp)
		]
	]
	out
]

close-tags: func[/only to-tag /local tag][
	opened-tags: tail opened-tags
	while [not empty? opened-tags: back opened-tags][
		tag: opened-tags/1
		append html case [
			tag = "url" ["</a>"]
			find ["list" "color" "quote" "size" "align" "email"] tag [""]
			true [rejoin ["</" tag ">"]]
		]
		remove opened-tags
		if tag = to-tag [break]
	]
	opened-tags: head opened-tags
]

form-size: func[/local size out][
	out: copy ""
	case/all [
		all [
			empty? short-attr
			empty? attributes
		][	return out ]
		
		any [
			all [
				size: get-attribute "size"
				not error? try [size: to pair! size]
			]
			all [
				short-attr
				not error? try [size: to pair! short-attr]
				size <> 0x0
			]
		][
			return rejoin [
				either size/x > 0 [ join " width="  to integer! size/x][""]
				either size/y > 0 [ join " height=" to integer! size/y][""]
			]
		]
		
		all [
			size: get-attribute "resize"
			not error? try [size: to pair! size]
		][
			return rejoin [
				either size/x > 0 [ join " width="  size/x][""]
				either size/y > 0 [ join " height=" size/y][""]
			]
		]
		
		all [
			not error? try [size: to integer! get-attribute "width"]
			size > 0
		][
			append out rejoin [" width=" size]
		]
		
		all [
			not error? try [size: to integer! get-attribute "height"]
			size > 0
		][
			append out rejoin [" height=" size]
		]
	]
	any [out ""]
]

close-p-if-possible: func[ /local p] [
	if all[
		not empty? opened-tags
		"p" = last opened-tags
	][
		close-tags/only "p"
		if "<p></p>" = p: skip tail html -7 [
			clear p
		]
	]
]
emit-tag-p: does [
	append html "<p>"
	append opened-tags "p"
]
emit-tag: func[tag][
	insert tail html either block? tag [rejoin tag][tag]
]

emit-tag-csv: function/with [spec [string!]][
	row: "" ;; no copy, it is cleared each time
	trim/head/tail spec
	
	close-p-if-possible
	close-tags
	emit-tag [{<table} form-attribute "class" form-attribute "align" form-attribute "style" {>^/}]
	all [
		widths: get-attribute "widths"
		widths: transcode widths
	]
	if align: get-attribute "coltype" [
		parse align [
			some [
				  #"c" (emit-tag {<col align="center">^/})
				| #"l" (emit-tag {<col align="left">^/}) 
				| #"r" (emit-tag {<col align="right">^/})
				| #"j" (emit-tag {<col align="justify">^/})
			]
		]
	]
	ch_divider: charset get-attribute/default "divider" TAB
	ch_notDivider: complement union ch_divider ch_crlf
	rl_data: [copy data any ch_notDivider]
	
	data: align: none
	row-num: col-num: col-width: 0
	datatag: "th" ;; first row is always used for headers!
	parse spec [
		some [
			(
				clear row
				++ row-num
			)
			any ch_space
			some [
				rl_data
				1 ch_divider
				(
					append row ajoin [{<} datatag get-col-width {>} data {</} datatag {>}]
				)
			]
			rl_data	[rl_newline | end] (
				append row ajoin [{<} datatag get-col-width {>} data {</} datatag {>}]
				datatag: "td"
				emit-tag ["<tr>" row "</tr>^/"]
			)
		]
	]
	emit-tag "</table>"
] [
	data: widths: align: row-num: col-num: col-width: none
	get-col-width: does [
		++ col-num
		either all [
			row-num = 1
			block? widths
			col-width: pick widths col-num
			integer? col-width
		][ ajoin [" width=" col-width] ][ "" ]
	]
]

;-- like: [images dir="foto/" alt="some text" maxWidth=680]
emit-tag-images: function/with [][
	close-tags
	if attr [repend attributes ["dir" copy attr]]
	;; maximum allowed width of the image (width of the gallery)
	max-width:  to integer! any [get-attribute "width"  get-attribute "maxWidth"  680]
	;; requested height of images on the row (may be higher!)
	row-height: to integer! any [get-attribute "height" get-attribute "rowHeight" 300]
	;; requested spacing between images on the row (may differ)
	space:  get-attribute/default "space" 6
	alt:    get-attribute/default "alt" ""
	unless empty? alt [insert alt SP]

	row-width: columns: num: 0
	temp: clear []
	files: none
	dir: to-rebol-file get-attribute "dir"
	files: read dir

	foreach file files [
		if any [
			;use only jpegs...
			none? find file %.jpg
			;don't use files with names like: photo_150x.jpg or photo_x150.jpg or photo_150x150.jpg  
			parse any [find/last file #"_" ""][
				#"_" any ch_digits #"x" any ch_digits #"." to end
			]
		][continue]

		img: load path: to-relative-file dir/:file
		size: img/size
		w: size/x
		h: size/y
		rw: to integer! (w * (row-height / h))
		size-scaled: as-pair rw row-height

		bgimg: enbase encode 'png resize img 6x3 64
		replace/all bgimg LF ""

		++ num

		row-width: row-width + rw + space
		title: ajoin [num #"." alt]
		either row-width > (1.5 * max-width) [ ;; the value 1.5 is there to get more images on a row (which is then scaled down)
			row-width: row-width - rw - space
			emit-row
			row-width: rw
			columns: 1
			append temp reduce [path size size-scaled bgimg title]
		][
			++ columns
			append temp reduce [path size size-scaled bgimg title]
		]
	]
	if columns > 0 [emit-row]
][
	temp: clear []
	dir: files: none
	max-width: row-width: 0

	emit-img: func[
		bgimg file size title
		/local nw nh
	][
		nw: to integer! size/x
		nh: to integer! size/y
		append html ajoin [
			{^/<div style="display:block;width:} nw {px;height:} nh {px;background-size:} nw {px } nh {px;background-image:url('data:image/png;base64,} bgimg {">}
			{<a href=} file { rel="images">}
			{<img src=} file { width=} nw { height=} nh { loading=lazy alt="} title {"/></a></div>}
		]
	]
	emit-row: func[/local scale][
		;; the final row is scaled to fit the maximal width
		scale: max-width / row-width
		append html "^/<div class=row>"
		foreach [file size size-scaled bgimg title] temp [
			emit-img bgimg file size-scaled * scale title
		]
		append html "^/</div>"
		clear temp
	]
]

enabled-tags: [
	"b" "i" "s" "u" "del" "h1" "h2" "h3" "h4" "h5" "h6" "span" "class"
	"ins" "dd" "dt" "ol" "ul" "li" "url" "list" "br" "hr"
	"color" "quote" "img" "size" "rebol" "align" "email" "ignore"
]
	
bbcode: func [
	"Converts BBCode markup into HTML"
	code [string!] "Input with BBCode tags"
	/local tag err
][
	err: try [
		emit-tag-p
		parse code [
			any [
				(attr: none)
				copy tmp some ch_normal (append html tmp)
				|
				"[url]" copy tmp some ch_url opt "[/url]" (
					emit-tag [{<a href="} encode-value tmp {">} tmp {</a>}]
				)
				|
				"[anchor]" copy tmp any ch_url opt "[/anchor]" (
					emit-tag [{<a name="} encode-value tmp {"></a>}]
				)
				|
				"[email]" copy tmp some ch_url opt "[/email]" (
					emit-tag [{<a href="mailto:} encode-value tmp {">} tmp {</a>}]
				)
				|
				"[img" opt rl_attributes #"]" copy tmp some ch_url opt "[/img]" (
					emit-tag [{<img} form-size { src="} encode-value tmp {"} form-attribute/default "alt" ""  {>}]
				) 
				|
				"[code]" copy tmp to "[/code]" thru "]" (
					emit-tag [{<code>} tmp {</code>}]
				)
				|
				"[rebol]" copy tmp to "[/rebol]" thru "]" (
					emit-tag [{<code>} tmp {</code>}]
					;TODO: add REBOL code colorizer
				)
				|
				"[/]" (
					close-tags
				)
				|
				"[br]" (emit-tag "<br>")
				|
				"[hr" any ch_space copy tmp [any ch_digits opt #"%"] any ch_space "]" (
					emit-tag either empty? tmp ["<hr>"][
						rejoin [{<hr style="width:} tmp {">}]
					]
				)
				|
				"[images" opt rl_attributes #"]" (emit-tag-images)
				|
				"[csv" opt rl_attributes #"]" copy tmp to "[/csv" (emit-tag-csv tmp)
				|
				"[ignore]" thru {[/ignore]}
				|
				#"[" [
					;normal opening tags
					copy tag some ch_name opt rl_attributes			
					#"]" (
						if tag = "*" [tag: "li"]
						append html either find enabled-tags tag [
							if find ["li"] tag [
								;closed already opened tag
								if all [
									tmp: find/last opened-tags tag 
									none? find tmp "ol"
									none? find tmp "ul"
								][
									close-tags/only tag
								]
							]
							
							switch/default tag [
								"url"  [
									append opened-tags "a"
									ajoin [{<a href="} encode-value short-attr {"} form-attribute "rel" {>}]
								]
								"color" [
									either all [short-attr parse short-attr [
										#"#" [6 ch_hexa | 3 ch_hexa]
									]][
										append opened-tags "span"
										ajoin [{<span style="color: } short-attr {;">}]
									][
										;;Should the invalid tag be visible?
										;rejoin either attr [
										;	["[" tag "=" attr "]"]
										;][	["[" tag "]"] ]
										""
									]
								]
								"quote" [
									append opened-tags ["fieldset" "blockquote"]
									either empty? short-attr [
										{<fieldset><blockquote>}
									][
										ajoin [{<fieldset><legend>} short-attr {</legend><blockquote>}]
									]
								]
								"list" [
									close-p-if-possible
									parse/case short-attr [
										[     "a" (tmp: {<ol style="list-style-type: lower-alpha;">})
											| "A" (tmp: {<ol style="list-style-type: upper-alpha;">})
											| "i" (tmp: {<ol style="list-style-type: lower-roman;">})
											| "I" (tmp: {<ol style="list-style-type: upper-roman;">})
											| "1" (tmp: {<ol style="list-style-type: decimal;">})
										] (append opened-tags "ol")
										| (append opened-tags "ul" tmp: {<ul>})
									]
									tmp
								]
								"size" [
									if none? short-attr [short-attr: ""]
									parse short-attr [
										[
											  ["-2" | "tiny" | "xx-small"] (tmp: {<span style="font-size: xx-small;">})
											| ["-1" | "x-small"]           (tmp: {<span style="font-size: x-small;">})
											| [ "0" | "small" | "normal"]  (tmp: {<span style="font-size: small;">})
											| [ "1" | "medium"]            (tmp: {<span style="font-size: medium;">})
											| [ "2" | "large" ]            (tmp: {<span style="font-size: large;">})
											| [ "3" | "x-large" | "huge"]  (tmp: {<span style="font-size: x-large;">})
											| [ "4" | "xx-large"]          (tmp: {<span style="font-size: xx-large;">})
											
										] end
										;TODO: other number values (pt,px,em)?
										| to end (tmp: {<span>})
									]
									append opened-tags "span"
									tmp
								]
								"align" [
									if none? short-attr [short-attr: ""]
									parse short-attr [
										[
											  ["right"   | "r"] (tmp: {<div style="text-align: right;">})
											| ["left"    | "l"] (tmp: {<div style="text-align: left;">})
											| ["center"  | "c"] (tmp: {<div style="text-align: center;">})
											| ["justify" | "j"] (tmp: {<div style="text-align: justify;">})
										] end
										| to end (tmp: {<div>})
									]
									append opened-tags "div"
									tmp
								]
								"email" [
									either error? try [tmp: to-email short-attr][""][
										append opened-tags "a"
										ajoin [{<a href="mailto:} encode-value tmp {">}]
									]
								]
								"class" [
									if none? short-attr [short-attr: ""]
									tmp: ajoin [{<span class="} short-attr {">}]
									append opened-tags "span"
									tmp
								]
							][
								if find ["h1" "h2" "h3" "h4" "h5" "h6" "ul" "ol"] tag [
									close-p-if-possible
								]
								append opened-tags tag
								ajoin either empty? short-attr [
									[#"<" tag #">"]
								][	[#"<" tag { class="} encode-value short-attr {">}] ]
							]
							
						][
							ajoin [#"[" tag #"]"]
						]
					)
					
					|
					;closing tags
					#"/" 
					copy tag some ch_name
					#"]" (
						either tmp: find/last opened-tags tag [
							close-tags/only tag
						][
							;;unopened tag, hidden by default, uncomment next line if you don't want to hide it
							;append html rejoin [{[/} tag {]}] 
						]
					)
					| (append html "[")
				]
				|
				#"<" (append html either allow-html-tags? ["<"]["&lt;"])
				|
				2 rl_newline (
					either any [
						pos: find/last opened-tags "ul"
						pos: find/last opened-tags "ol"
					][
						if all [
							pos: find/last pos "li"
							none? find pos "p"
						][
							insert next pos "p" 
							insert find/last/tail html "<li>" "<p>"
							close-tags/only "p"
						]
					][
						close-tags/only "p"
					]
					
					append html "^/^/"
					emit-tag-p
				)
				|
				pos: rl_newline (
					case [
						#" " = pos/(-1) [append html "<br>^/"]
						true [append html lf]
					]
				)
			]
		]
		unless empty? opened-tags [	close-tags ]
		html
	][
		probe err: system/state/last-error
		; send possible trimmed error in the result instead of throwing it!
		append html ajoin ["^/#[ERROR! [code: " err/code " type: " err/type " id: " err/id #"]"]
	]
	; clear internal states and return copy of the result
	tmp: pos: none 
	clear opened-tags
	also 
		copy html
		clear html
]

register-codec [
	name:  'bbcode
	type:  'text
	title: "Bulletin Board Code"
	suffixes: [%.bbcode]

	decode: func [
		"Converts BBCode markup into HTML"
		code [string! binary! file! url!] "Input with BBCode tags"
		/local result
		return: [string!]
	][
		switch type?/word code [
			binary!    [code: to string! code]
			file! url! [code: read/string code]
		] 
		result: bbcode code
	]
]