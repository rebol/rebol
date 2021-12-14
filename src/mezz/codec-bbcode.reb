REBOL [
	Name:    bbcode
	Type:    module
	Options: [delay]
	Version: 0.3.0
	Title:   "BBcode codec"
	Purpose: {Basic BBCode implementation. For more info about BBCode check http://en.wikipedia.org/wiki/BBCode}
	File:    https://raw.githubusercontent.com/Oldes/Rebol3/master/src/mezz/codec-bbcode.reb
	Date:    24-Apr-2020
	Author:  "Oldes"
	History: [
		0.1.0  5-Jan-2009 "initial version"
		0.2.0 19-Feb-2012 "review"
		0.2.1 22-Aug-2012 "added [hr] and [anchor]"
		0.3.0 24-Apr-2020 "ported to Rebol3"
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
	parse/all value [
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

enabled-tags: [
	"b" "i" "s" "u" "del" "h1" "h2" "h3" "h4" "h5" "h6" "span" "class"
	"ins" "dd" "dt" "ol" "ul" "li" "url" "list" "br" "hr"
	"color" "quote" "img" "size" "rebol" "align" "email" "ignore"
]
	
bbcode: func [
	"Converts BBCode markup into HTML"
	code [string! binary!] "Input with BBCode tags"
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
	]
	if error? err [
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
		if any [file? code url? code][code: read code]
		result: bbcode code

	]
]