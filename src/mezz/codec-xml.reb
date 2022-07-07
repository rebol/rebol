;@@ Oldes:                                                                             
;-- This XML codec is based on Gavin's (Brian's?) script %xml-parse.r downloaded from: 
;-- http://www.rebol.org/view-script.r?script=xml-parse.r                              
;-- Rebol2 had built-in `parse-xml` function and this script was enhanced variant      
;-- Following Rebol header is from original script!                                    
REBOL [
	Name:  xml
	Type:  module
	Options: [delay]
	Version: 0.8.1
	Title: "Codec: XML"
	File:  https://raw.githubusercontent.com/Oldes/Rebol3/master/src/mezz/codec-xml.reb
	Date:  24-Apr-2020
	Author: ["Gavin F. McKenzie" "Oldes"]
	Email:  %brianwisti--yahoo--com
	Purpose: {
		REBOL's built-in parse-xml function lacks a number of 
		XML 1.0 compliant features, including:
		- support for CDATA sections
		- support for XML Namespaces
		- exposure of the internal DTD subset

		The intent of this script is to create an XML parser
		that can operate either via an event/callback mechanism,
		or produce a block structure similar to REBOL's built-in
		parse-xml function.

		This XML parser is designed to call-back into a 
		'parse-handler' object that has been designed to be similar
		to the well-known XML parsing interface known as "SAX"
		(Simple API for XML) by David Megginson.

		For more information on SAX, see David's website at:
		http://www.megginson.com/SAX/index.html

		Several parse-handlers are included here:
		- the base 'class' xml-parse-handler that contains only empty 
		  callback stubs
		- the debugging aid echo-handler that prints out the callback
		  event stream
		- the block-handler that produces a superset of the block structure
		  created by REBOL's built-in parse-xml function

		Alternatively, you may choose to build your own parse
		event handler rather than use the functionality provided
		here.

		A new function is defined "parse-xml+" that represents
		the enhanced counterpart to the built-in REBOL parse-xml.

		Additional features provided by this parser:
		1.  Document prolog information

			The built-in REBOL parse-xml function returns a set of
			nested blocks where the first two items in the outermost
			block are the words document and none, such as:
			[document none [...]]

			The parse-xml+ function provided herein can produce a
			set of nested blocks where the second item of the outermost
			block is used to represent prolog and document type
			information.

			An example of this block is:

			[   version "1.0" encoding "utf-8" standalone "yes"
				doctype none pubid none sysid none subset none
			]

		2. CDATA Section Processing

		   XML provides for enclosing data content within CDATA
		   sections for the convenience of avoiding the need to
		   escape certain XML sensitive characters in the data
		   such as the ampersand (&) and less-than-sign (<).

		   An example of a CDATA section:

		   <foo>abc <![CDATA[Jack & Jill]]> xyz</foo>

		   A compliant XML parser would report that the content 
		   of element 'foo' is "abc Jack & Jill xyz". 

		   CDATA sections are also useful when putting text samples
		   of XML within the content of an XML document.

		   <example><![CDATA[<foo>bar</foo>]]></example>

		   Here the value of element 'example' is the text
		   "<foo>bar</foo>"

		3. Comments

		   This parser provides the opportunity to process 
		   comments embedded within the XML.

		4. Processing Instructions

		   This parser provides the opportunity to process 
		   processing instructions embedded within the XML.

		   <foo><?my-app this is a processing instruction?></foo>

		5. Automatic Character Entity Expansion

		   In XML document it is common to encounter "character
		   entities" within the content of the document.  These
		   entities are the means for escaping sensitive XML 
		   characters so that the character will be processed as
		   data rather than markup.  The most common characters
		   that are subjected to this treatment are the 
		   ampersand (&) and less-than-sign (<).

		   This parser recognizes these common entities and
		   automatically converts them to their character 
		   equivalents.

		   For example:

		   <foo>Jack &amp; Jill</foo>

		   This parser will automatically replace the &amp;
		   character entity reference to the ampersand (&)
		   character; hence, the value of element 'foo' is
		   "Jack & Jill".

		   Character entities can also be encoded with their
		   Unicode numeric equivalent rather than the symbolic
		   name in either decimal or hex form, such as:

		   <foo>Ampersands: &amp;&#38;&#x26;</foo>

		   The value of element 'foo' is "Ampersands: &&&".

		6. Namespace Processing

		   Namespace processing is vital to handling real-world
		   XML.

		   @@TBD: say more here
	}
	History: [
	0.8.1 { Oldes: fixed Prolog parsing in some cases}
	0.8.0 { Oldes: used original script as a Rebol3 codec}
	0.7.6 { Version from 1-jul-2009 downloaded from rebol.org}
	0.7.4 { Fixed a defect to allow optional space around
			the '=' on an attribute.  
			Thanks to Brett Handley for reporting the defect.}
	0.7.3 { Fixed bug where attr-ns-prefix wasn't getting cleared
			when processing an un-prefixed attribute.}
	0.7.2 { Changed the start-document in the block-handler
		to perform a copy/deep, fixing a bug that occurred
		on successive invocations of parse-xml+.}
	0.7.1 { First public release. }

	]
 Acknowledgements: {
	Gavin F. MacKenzie wrote the original releases of this file,
	so it just plain wouldn't have happened without him. I hope he
	knows we're grateful and we hope he's doing well - wherever he's
	disappeared to!
	}
]

; TO DO
; - ** WARNING ** Namespace processing is not ready for primetime!
; - anything that uses xmlQuote is wrong; there is the potential for 
;   uncaught mismatched quotes
; - do some start/end-tag matching and error-checking
; - process entities defined in the internal DTD subset
; - add comments, comments, comments!!

;---------------------------------------------------------------------------------
;-- CODEC's code follows:
;------------------------

register-codec [
	name:  'xml
	type:  'text
	title: "Extensible Markup Language"
	suffixes: [%.xml %.pom]

	decode: function [
		"Parses XML code and returns a tree of blocks"
		data [binary! string!] "XML code to parse"
		/trim "Removes whitespaces (from head of strings)"
	][
		if binary? data [data: to string! data]
		parser/xmlTrimSpace: any [
			trim
			select options 'trim
		]
		parser/parse-xml data
	]

	verbose: 0 ;not used so far, but could be later
	options: object [trim: false]


	;-- follows content of the original xml-parse object:

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;
	; XML PARSE CONTENT HANDLERS
	;
	; This XML parser is designed to call-back into a 'parse-handler' object
	; that has been designed to be similar to the well-known XML parsing 
	; interface known as "SAX" (Simple API for XML) by David Megginson.
	;
	; For more information on SAX, see David's website at:
	;   http://www.megginson.com/SAX/index.html
	;
	; Several parse-handlers are included here:
	; - the base 'class' xml-parse-handler that contains only empty 
	;   callback stubs
	; - the debugging aid echo-handler that prints out the callback
	;   event stream
	; - the block-handler that produces a superset of the block structure
	;   created by REBOL's built-in parse-xml function
	;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	;
	; This is an 'empty' xml-parse-handler that is provided as a base-object
	; for extending into custom xml-parse-handlers.
	;
	xml-parse-handler: make object! [
		start-document: func [
		][
		]
		xml-decl: func [
			version-info [string! none!] 
			encoding [string! none!] 
			standalone [string! none!]
		][
		]
		document-type: func [
			document-type [string!] 
			public-id [string! none!] 
			system-id [string! none!] 
			internal-subset [string! none!]
		][
		]
		start-element: func [
			ns-uri [string! none!] 
			local-name [string! none!] 
			q-name [string!] 
			attr-list [block!]
		][
		]
		end-element: func [
			ns-uri [string! none!] 
			local-name [string! none!] 
			q-name [string!]
		][
		]
		characters: func [
			characters [string! char! none!]
		][
		]
		pi: func [
			pi-target [string! none!]
			pi [string! none!]
		][
		]
		comment: func [
			comment [string! none!]
		][
		]
		end-document: func [] [
		]
		start-prefix-mapping: func [
			ns-prefix-uri-pairs [block!] 
		][
		]
		end-prefix-mapping: func [
			ns-prefix-uri-pairs [block!] 
		][
		]
		get-parse-result: func [{
			This function can be used to return a specific result from
			the parse operation, such as returning the parse XML as a 
			series of blocks similar to REBOL's built-in parse-xml.
			By default, returning none from this function will cause the
			return code from the REBOL parse function to be passed back
			to the caller of the parse.}
		][
			none
		]
	]

	;
	; This xml-parse-handler simply echoes the parsing to the console.
	; This was primarily useful as a debugging aid during the development
	; of the XML parse production rules.
	;
	echo-handler: make xml-parse-handler [
		start-document: func [
		][
			print remold ['start-doc]
		]
		xml-decl: func [
			version-info [string! none!] 
			encoding [string! none!] 
			standalone [string! none!]
		][
			print remold ['xml-decl 'version-info version-info 
						  'encoding encoding 'standalone standalone
						 ]
		]
		document-type: func [
			document-type [string!] 
			public-id [string! none!] 
			system-id [string! none!] 
			internal-subset [string! none!]
		][
			print remold ['doc-type document-type
						  'public-id public-id 
						  'system-id system-id 
						  'internal-subset internal-subset
						 ]
		]
		start-element: func [
			ns-uri [string! none!] 
			local-name [string! none!] 
			q-name [string!] 
			attr-list [block!]
		][
			print remold ['start-elem ns-uri local-name q-name
						  'attr-list attr-list
						 ]
		]
		end-element: func [
			ns-uri [string! none!] 
			local-name [string! none!] 
			q-name [string!]
		][
			print remold ['end-elem ns-uri local-name q-name]
		]
		characters: func [
			characters [string! char! none!]
		][
			print remold ['characters characters]
		]
		pi: func [
			pi-target [string! none!] 
			pi [string! none!]
		][
			print remold ['pi pi-target pi]
		]
		comment: func [
			comment [string! none!]
		][
			print remold ['comment comment]
		]
		end-document: func [
		][
			print remold ['end-doc]
		]
		start-prefix-mapping: func [
			ns-prefix-uri-pairs [block!] 
		][
			print remold ['start-prefix ns-prefix-uri-pairs]
		]
		end-prefix-mapping: func [
			ns-prefix-uri-pairs [block!] 
		][
			print remold ['end-prefix ns-prefix-uri-pairs]
		]
	]

	;
	; This xml-parse-handler produces a set of nested blocks representing
	; the parsed XML content.  The blocks structure is a compatible superset
	; of the block structure produced by REBOL's built-in parse-xml function.
	; Extensions to the structure are appended to any applicable block; 
	; hence, existing code intended for use with REBOL's existing post-parse
	; block structure should continue to work.
	;
	block-handler: make xml-parse-handler [
		xml-doc: copy []
		xml-block: none
		xml-content: copy ""

		start-document: does [
			;
			; Seed the document
			;
			xml-block: reduce ['document make map! [] none]
		]
		xml-decl: func [
			version-info [string! none!] 
			encoding [string! none!] 
			standalone [string! none!]
		][
			xml-block/2/version:    version-info
			xml-block/2/encoding:   encoding
			xml-block/2/standalone: standalone
		]
		document-type: func [
			document-type [string!] 
			public-id [string! none!]
			system-id [string! none!] 
			internal-subset [string! none!]
		][
			xml-block/2/doctype: document-type
			xml-block/2/pubid:   public-id
			xml-block/2/sysid:   system-id
			xml-block/2/subset:  internal-subset
		]
		start-element: func [
			ns-uri [string! none!] 
			local-name [string! none!] 
			q-name [string!] 
			attr-list [block!]
		][
			;
			; Is there any pending content to add before
			; we start a new element?
			;
			if not empty? xml-content [
				add-child copy xml-content
				clear head xml-content
			]
			insert/only tail xml-doc xml-block
			xml-block: add-child copy reduce [local-name none none]
			;
			; Add the attribute list
			;
			if not empty? attr-list [
				xml-block/2: copy attr-list
			]
		]
		characters: func [
			characters [string! char! none!]
		][
			;
			; Accumulate more character data
			;
			if not none? characters [
				append xml-content characters
			]
		]
		end-element: func [
			ns-uri [string! none!] 
			local-name [string! none!] 
			q-name [string!]
		][
			;
			; Is there any pending content to add before
			; we terminate this element?
			;
			if not empty? xml-content [
				add-child copy xml-content
				clear head xml-content
			]
			;
			; Basic well-formedness check
			;
;           while [q-name <> first xml-block] [
;               if empty? xml-doc [
;                   print ["End tag error:" q-name]
;                   halt
;               ]
;               pop-xml-block
;           ]
			pop-xml-block
		]
		add-child: func [child] [
			if none? third xml-block [xml-block/3: make block! 1]
			insert/only tail third xml-block child
			child
		]
		pop-xml-block: func [] [
			xml-block: last xml-doc
			remove back tail xml-doc
		]
		get-parse-result: func [] [
			xml-block
		]
	]

	;
	; This xml-parse-handler enhances the block-handler with namespace 
	; processing.  It should only be used with a parser that has been
	; set to namespace-aware true.
	;
	ns-block-handler: make block-handler [
		nsinfo-stack: copy []

		start-element: func [
			ns-uri [string! none!] 
			local-name [string! none!] 
			q-name [string!] 
			attr-list [block!]
		][
			;
			; Is there any pending content to add before
			; we start a new element?
			;
			if not empty? xml-content [
				add-child copy xml-content
				clear head xml-content
			]
			insert/only tail xml-doc xml-block
			xml-block: add-child copy reduce [local-name 
											  none 
											  none 
											  ns-uri
											 ]
			;
			; Add the attribute list
			;
			if not empty? attr-list [
				xml-block/2: copy attr-list
			]
		]

		start-prefix-mapping: func [
			ns-prefix-uri-pairs [block!] 
		][
			insert/only nsinfo-stack ns-prefix-uri-pairs
		]
		end-prefix-mapping: func [
			ns-prefix-uri-pairs [block!] 
		][
			remove nsinfo-stack
		]
	]



	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;
	; ACTUAL XML PARSER OBJECT
	;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	parser: make object! [
		element-q-name:     none
		element-local-name: none
		document-type:      none
		system-id:          none
		public-id:          none
		internal-subset:    none
		encoding:           none
		characters:         none
		entity-ref:         none
		char-ref-value:     none
		comment:            none
		pi-target:          none
		pi:                 none
		version-info:       none
		encoding:           none
		standalone:         none
		attr-name:          none
		attr-value:         none
		attr-list:          copy []

		attr-ns-prefix:     none
		element-ns-prefix:  none
		ns-uri:             none
		nsinfo-stack:       copy []
		nsinfo:             copy []

		; 
		; Set a default xml-parse-handler
		;
		handler:            block-handler

		;
		; Namespace processing, set true to process namespaces
		;
		namespace-aware:    no

		;
		; XML 1.0 Production Rules
		;
		xmlLetter:          charset [#"A" - #"Z" #"a" - #"z"]
		xmlDigit:           charset [#"0" - #"9"]
		xmlHexDigit:        charset [#"0" - #"9" #"a" - #"f" #"A" - #"F"]
		xmlAlpha:           charset [#"A" - #"Z" #"a" - #"z"]
		xmlAlphaNum:        charset [#"0" - #"9" #"A" - #"Z" #"a" - #"z"]
		xmlQuote:           charset [#"^"" #"^'"]
		xmlSpace:           charset [#"^(20)" #"^(09)" #"^(0D)" #"^(0A)"]
		xmlNotMarkupChar:   complement charset [#"<" #"&"] ;fix this
		xmlS:               [some xmlSpace]
		xmlEq:              [[any xmlSpace] "=" [any xmlSpace]]
		xmlChar:            [any [xmlAlphaNum | xmlSpace]]
		xmlCharData:        [   copy characters 
								some xmlNotMarkupChar
								(handler/characters characters)
							]
		xmlNameProd:        [[xmlLetter | #"_" | #":"] any xmlNameChar]
		xml10Name:          xmlNameProd
		xmlName:            xml10Name
		xmlNameChar:        [   xmlLetter | xmlDigit |
								#"." | #"-" | #"_" | #":"
							]
		xmlNames:           [xmlName any [xmlS xmlName]]
		xmlNMToken:         [some xmlNameChar]
		xmlNMTokens:        [xmlNMToken any [xmlS xmlNMToken]]
		xmlMisc:            [xmlComment | xmlPI | xmlS]
		xmlPERef:           [#"%" xmlNameProd #"%"]
		xmlEntIntro:        charset [#"^^" #"^%" #"^&" #"^""]
		xmlEntityVal:       [#"^"" any [xmlEntIntro | xmlPERef] #"^""]
		xmlContent:         [any [  xmlElement | xmlComment | xmlPI |
									xmlCDSect | xmlCharData | xmlReference
								 ]
							] ;fix this
		xmlAttValueStrict:  [   [#"^"" copy attr-value to #"^"" #"^""] | 
								[#"'" copy attr-value to #"'" #"'"]
							] ; fix this
		xmlAttValue:        xmlAttValueStrict
		xmlAttType:         [   xmlStringType |
								xmlTokenizedType |
								xmlEnumeratedType
							]
		xml10AttrStrict:    [   copy attr-name xmlName
								xmlEq
								xmlAttValue
								(append append 
									attr-list attr-name attr-value
								)
							]
		xmlAttribute:       xml10AttrStrict
		xmlSTag:            [   #"<" 
								copy element-q-name xmlName 
								[   (clear head attr-list clear head nsinfo)
									any [xmlS xmlAttribute]
								]
								[any xmlSpace]
								#">" xmlTrimSpace
								(either namespace-aware [
									handler/start-prefix-mapping nsinfo
									insert/only nsinfo-stack copy nsinfo
									handler/start-element
										ns-uri
										element-local-name
										element-q-name
										attr-list
								 ][
									handler/start-element
										none
										element-q-name
										element-q-name
										attr-list
								 ]
								)
							]
		xmlETag:            [   "</" 
								copy element-q-name xmlName 
								[any xmlSpace]
								#">" xmlTrimSpace
								(either namespace-aware [
									handler/end-element
										ns-uri
										element-local-name
										element-q-name
									handler/end-prefix-mapping
										first nsinfo-stack
									remove nsinfo-stack
								 ][
									handler/end-element
										none
										element-q-name
										element-q-name
								 ]
								)
							]
		xmlEmptyElemTag:    [   #"<"
								copy element-q-name xmlName
								[   (clear head attr-list clear head nsinfo)
									any [xmlS xmlAttribute]
								]
								[any xmlSpace]
								"/>" xmlTrimSpace
								(either namespace-aware [
									handler/start-prefix-mapping nsinfo
									insert/only nsinfo-stack copy nsinfo
									handler/start-element
										ns-uri
										element-local-name
										element-q-name
										attr-list
								 ][
									handler/start-element
										none
										element-q-name
										element-q-name
										attr-list
								 ]
								 characters: none 
								 handler/characters characters 
								 either namespace-aware [
									handler/end-element
										ns-uri
										element-local-name
										element-q-name
									handler/end-prefix-mapping
										first nsinfo-stack
									remove nsinfo-stack
								 ][
									handler/end-element
										none
										element-q-name
										element-q-name
								 ]
								)
							]
		xmlEmptyElem:       [   #"<"
								copy element-q-name xmlName
								[   (clear head attr-list clear head nsinfo)
									any [xmlS xmlAttribute]
								]
								[any xmlSpace]
								"></"
								xmlName
								[any xmlSpace]
								#">" xmlTrimSpace
								(either namespace-aware [
									handler/start-prefix-mapping nsinfo
									insert/only nsinfo-stack copy nsinfo
									handler/start-element
										ns-uri
										element-local-name
										element-q-name
										attr-list
								 ][
									handler/start-element
										none
										element-q-name
										element-q-name
										attr-list
								 ]
								 characters: none
								 handler/characters characters
								 either namespace-aware [
									handler/end-element
										ns-uri
										element-local-name
										element-q-name
									handler/end-prefix-mapping
										first nsinfo-stack
									remove nsinfo-stack
								 ][
									handler/end-element
										none
										element-q-name
										element-q-name
								 ]
								)
							]
		xmlElementStrict:   [   xmlEmptyElemTag |
								xmlEmptyElem | 
								[xmlSTag opt xmlContent xmlETag]
							]
		xmlElement:         xmlElementStrict
		xmlPI:              [   "<?"
								copy pi-target xmlPITarget
								copy pi to "?>" "?>"
								(handler/pi pi-target pi)
							]
		xmlPITarget:        [xmlNameProd]
		xmlComment:         [   "<!--" copy comment to  "-->" "-->"
								(handler/comment comment)
							]
		xmlDecl:            [   (version-info: encoding: standalone: none)
								"<?xml"
								any xmlSpace
								xmlVersionInfo
								opt [xmlS xmlEncodingDecl]
								opt [xmlS xmlSDDecl]
								any xmlSpace
								"?>" xmlTrimSpace
								(handler/xml-decl
									version-info
									encoding
									standalone
								)
							]
		xmlVersionInfo:     [   "version"
								xmlEq 
								[   #"^"" copy version-info xmlVersionNum "^"" |
									#"^'" copy version-info xmlVersionNum "^'" 
								]
							]
		xmlVersionNum:      [some [xmlVersionNumChars | "-"]]
		xmlVersionNumChars: charset [   #"0" - #"9"
										#"A" - #"Z"
										#"a" - #"z"
										"_.:"
									]
		xmlEncodingDecl:    [   (encoding: none)
								"encoding"
								xmlEq 
								[   #"^"" copy encoding xmlEncName "^"" |
									#"^'" copy encoding xmlEncName "^'" 
								]
							]
		xmlEncName:         [xmlAlpha any [xmlEncNameChars]]
		xmlEncNameChars:    charset [   #"0" - #"9"
										#"A" - #"Z"
										#"a" - #"z"
										"_.-"
									]
		xmlDocument:        [   (handler/start-document)
								xmlProlog
								xmlElement
								any xmlMisc
								(handler/end-document)
							]
		xmlProlog:          [   opt xmlDecl
								any xmlMisc
								opt [xmlDocTypeDecl any xmlMisc]
							]
		xmlDocTypeDecl:     [   (public-id: system-id: internal-subset: none)
								"<!DOCTYPE" xmlS
								copy document-type xmlName
								opt [xmlS xmlExternalID]
								any xmlSpace
								opt [#"[" copy internal-subset to #"]" 1 skip] ;@@ this can be unsafe!
								any xmlSpace ">"
								(handler/document-type
									document-type
									public-id
									system-id
									internal-subset
								)
							]
		xmlSDDecl:          [   (standalone: none)
								"standalone"
								xmlEq
								[   xmlQuote
									copy standalone ["yes" | "no"]
									xmlQuote
								]
							]
		xmlStringType:      "CDATA"
		xmlTokenizedType:   [   "ID" | "IDREF" | "IDREFS" |
								"ENTITY" | "ENTITIES" |
								"NMTOKEN" | "NMTOKENS"
							]
		xmlEnumeratedType:  [] ; fix this
		xmlReference:       [xmlCharRef | xmlEntityRef]
		xmlEntityRef:       [   ["&" copy entity-ref xmlNameProd ";"]
								(   char-ref-value: 
										convert-character-entity entity-ref
									either none? char-ref-value [
										;
										; couldn't convert the
										; chararacter-entity, so pass
										; it through as character data,
										; unconverted
										;
										handler/characters
											rejoin ["&" entity-ref ";"]
									][
										;
										; converted the chararacter-entity
										; to a character
										;
										handler/characters char-ref-value
									]
								)
							]
		xmlCharRef:         [   [   [   "&" 
										[copy entity-ref 
											["#" some xmlDigit]
										]
										";"
									] | 
									[   "&"
										[copy entity-ref
											["#x" some xmlHexDigit]
										]
										";"
									]
								]  
								(   char-ref-value:
										convert-character-entity entity-ref
									either none? char-ref-value [
										;
										; couldn't convert the
										; chararacter-entity, so pass
										; it through as character data,
										; unconverted
										;
										handler/characters
											rejoin ["&" entity-ref ";"]
									][
										;
										; converted the chararacter-entity
										; to a character
										;
										handler/characters char-ref-value
									]
								)
							]
		xmlExternalID:      [   ["SYSTEM" xmlS xmlSystemLiteral] | 
								["PUBLIC" xmlS xmlPubIDLiteral xmlS xmlSystemLiteral]
							]
		xmlSystemLiteral:   [   [#"^"" copy system-id to #"^"" 1 skip] | 
								[#"'"  copy system-id to #"'"  1 skip]
							] 
		xmlPubIDLiteral:    [   [#"^"" copy public-id to #"^"" 1 skip] |
								[#"'"  copy public-id to #"'"  1 skip]
							] 
		xmlNDataDecl:       [xmlS "NDATA" xmlS xmlNameProd]
		xmlCDSect:          [   "<![CDATA[" 
								copy characters to "]]>" 
								"]]>" 
								(handler/characters characters)
							]

		; XML Namespace-Specific Production Rules
		;
		xmlNSAttribute:     [   [   copy attr-name xmlPrefixedAttName
									xmlEq xmlAttValue
									(ns-uri: copy attr-value
									 append nsinfo
										reduce [attr-ns-prefix
												attr-value
											   ]
									)
								] | 
								[   copy attr-name xmlDefaultAttName
									xmlEq
									xmlAttValue
									(ns-uri: copy attr-value
									 append nsinfo
										reduce [attr-ns-prefix
												attr-value
											   ]
									)
								] |
								[   xmlAQName
									xmlEq
									xmlAttValue
									(append attr-list
										reduce [attr-name 
												attr-value
												attr-ns-prefix
											   ]
									)
								]
							]
		xmlPrefixedAttName: ["xmlns:" copy attr-ns-prefix xmlNCName]
		xmlDefaultAttName:  ["xmlns" (attr-ns-prefix: none)]
		xmlNCName:          [[xmlLetter | #"_"] any xmlNCNameChar]
		xmlNCNameChar:      [xmlLetter | xmlDigit | #"." | #"-" | #"_"]
		xmlAQName:          [   [   copy attr-ns-prefix xmlNCName
									#":"
									copy attr-name xmlNCName
								] | 
								[   copy attr-name xml10Name
									(attr-ns-prefix: none)
								]
							]
		xmlQName:           [   [   copy element-ns-prefix xmlNCName
									#":"
									copy element-local-name xmlNCName
									(element-q-name: copy rejoin
										[element-ns-prefix
										 ":"
										 element-local-name
										]
									)
								] |
								[   copy element-local-name xml10Name
									(element-q-name: element-ns-prefix: "")
								]
							]       
		xmlTrimSpace: none ;- no trim by default
		;
		;
		; Private XML Parser Methods
		;
		convert-character-entity: func [{
			Accepts the name reference portion of an entity
			reference and attempts to return the actual character
			referenced by the entity.
			If the conversion is not successful, the value of 
			none is returned.
			For example, for the ampersand character this function
			could accept a entity-ref parameter of either "amp",
			"#38" or "#x26".
		}
			entity-ref [string!]
		][
			switch/default entity-ref [
				"lt"        [ return #"<" ]
				"gt"        [ return #">" ]
				"amp"       [ return #"&" ]
				"quot"      [ return #"^"" ]
				"apos"      [ return #"'" ]
			][
				either (first entity-ref) = #"#" [
					to char! to integer! either (second entity-ref) = #"x" [
						to issue! skip entity-ref 2
					][	skip entity-ref 1 ]
				][
					none
				]
			]
		]
		
		;--
		;-- Public XML Parser Methods
		;--
		parse-xml: func [{
			Parses XML code and executes an associated event handler
			during processing.
			This is a more XML 1.0 compliant parse than the built-in
			REBOL parse-xml function.
		}
			xml-string [string!] 
			/local parse-result
		][

			if true? xmlTrimSpace [xmlTrimSpace: [any xmlSpace]]

			;
			; Parse the document and capture the return code from the REBOL
			; parse.
			;
			parse-result: parse/case xml-string xmlDocument
			;
			; If the handler doesn't return a specific parse result, then  
			; return the parse-result we obtained from the REBOL parse.
			;
			either handler/get-parse-result = none [
				parse-result
			][
				handler/get-parse-result
			]
		]

		set-parse-handler: func [
			arg-handler [object!]
		][
			handler: arg-handler
		]

		get-parse-handler: does [
			handler
		]

		set-namespace-aware: func [{
			This function enables the namespace processing
			of the parser.  As a result, the parser will
			process xmlns attributes and namespace prefixes.
			The parse-handlers will receive additional
			namespace specific information.
		}
			arg-namespace-aware [logic!]
		][
			namespace-aware: arg-namespace-aware
			either arg-namespace-aware [
				xmlName:        xmlQName
				xmlAttribute:   xmlNSAttribute
			][
				xmlName:        xml10Name
				xmlAttribute:   xml10AttrStrict
			]
			namespace-aware
		]

		get-namespace-aware: does [
			namespace-aware
		]
	]
]

