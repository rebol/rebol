Rebol [
	Name:    html-entities
	Type:    module
	Options: [delay]
	Version: 1.0.0
	Title:   "HTML entities codec"
	Purpose: "To decode HTML entities in a text"
	File:    https://raw.githubusercontent.com/Oldes/Rebol3/master/src/mezz/codec-html-entities.reb
	Date:    12-May-2020
	Author:  "Oldes"
	Rights: http://opensource.org/licenses/Apache-2.0
	Usage: [
		"Test: ♠ & ¢ <a> and Δδ ¾" =
		decode 'html-entities {Test: &spades; & &#162; &lt;a&gt;&#32;and &Delta;&delta; &frac34;}
	]
	TODO: {
		*Encoder
	}
]

html-entities: #[map! [
	;@@ https://eastmanreference.com/list-of-html-entity-names-and-numbers
	;-- Punctuation, programming, and other common symbols
	"lt"       #"^(3C)"     ; 60    Open tag
	"gt"       #"^(3E)"     ; 62    Close tag
	"quot"     #"^(22)"     ; 34    Double quote
	"apos"     #"^(27)"     ; 39    Apostrophe / single quote
	"amp"      #"^(26)"     ; 38    Ampersand
	"nbsp"     #"^(A0)"     ; 160   Space (non-breaking)
	"brvbar"   #"^(A6)"     ; 166   Broken bar
	"iexcl"    #"^(A1)"     ; 161   Upside down exclamation mark
	"iquest"   #"^(BF)"     ; 191   Upside down question mark
	"sect"     #"^(A7)"     ; 167   Section symbol
	"uml"      #"^(A8)"     ; 168   Umlaut
	"ordf"     #"^(AA)"     ; 170   Feminine ordinal indicator
	"ordm"     #"^(BA)"     ; 186   Masculine ordinal indicator
	"laquo"    #"^(AB)"     ; 171   Open double angles
	"raquo"    #"^(BB)"     ; 187   Close double angles
	"not"      #"^(AC)"     ; 172   Not sign
	"shy"      #"^(AD)"     ; 173   Soft hyphen
	"macr"     #"^(AF)"     ; 175   Overline
	"acute"    #"^(B4)"     ; 180   Acute accent
	"para"     #"^(B6)"     ; 182   Pilcrow (paragraph)
	"middot"   #"^(B7)"     ; 183   Georgian comma
	"cedil"    #"^(B8)"     ; 184   Cedilla
	;-- Math symbols
	"minus"    #"^(2212)"   ; 8722  Minus sign (subtraction)
	"times"    #"^(D7)"     ; 215   Multiplication sign
	"divide"   #"^(F7)"     ; 247   Division sign
	"plusmn"   #"^(B1)"     ; 177   Plus / minus
	"le"       #"^(2264)"   ; 8804  Less or equal
	"ge"       #"^(2265)"   ; 8805  Greater or equal
	"sup1"     #"^(B9)"     ; 185   Superscript 1
	"sup2"     #"^(B2)"     ; 178   Superscript 2
	"sup3"     #"^(B3)"     ; 179   Superscript 3
	"frac14"   #"^(BC)"     ; 188   1/4
	"frac12"   #"^(BD)"     ; 189   1/2
	"frac34"   #"^(BE)"     ; 190   3/4
	"forall"   #"^(2200)"   ; 8704  For all
	"part"     #"^(2202)"   ; 8706  Part
	"exist"    #"^(2203)"   ; 8707  Exist
	"empty"    #"^(2205)"   ; 8709  Empty
	"nabla"    #"^(2207)"   ; 8711  Nabla
	"isin"     #"^(2208)"   ; 8712  Is in
	"notin"    #"^(2209)"   ; 8713  Not in
	"ni"       #"^(220B)"   ; 8715  Ni
	"prod"     #"^(220F)"   ; 8719  Product
	"sum"      #"^(2211)"   ; 8721  Sum
	"lowast"   #"^(2217)"   ; 8727  Asterisk (Lowast)
	"radic"    #"^(221A)"   ; 8730  Square root
	"prop"     #"^(221D)"   ; 8733  Proportional to
	"infin"    #"^(221E)"   ; 8734  Infinity
	"ang"      #"^(2220)"   ; 8736  Angle
	"and"      #"^(2227)"   ; 8743  And
	"or"       #"^(2228)"   ; 8744  Or
	"cap"      #"^(2229)"   ; 8745  Cap
	"cup"      #"^(222A)"   ; 8746  Cup
	"int"      #"^(222B)"   ; 8747  Integral
	"there4"   #"^(2234)"   ; 8756  Therefore
	"sim"      #"^(223C)"   ; 8764  Similar to
	"cong"     #"^(2245)"   ; 8773  Congurent to
	"asymp"    #"^(2248)"   ; 8776  Almost equal
	"ne"       #"^(2260)"   ; 8800  Not equal
	"equiv"    #"^(2261)"   ; 8801  Equivalent
	"sub"      #"^(2282)"   ; 8834  Subset of
	"sup"      #"^(2283)"   ; 8835  Superset of
	"nsub"     #"^(2284)"   ; 8836  Not subset of
	"sube"     #"^(2286)"   ; 8838  Subset or equal
	"supe"     #"^(2287)"   ; 8839  Superset or equal
	"oplus"    #"^(2295)"   ; 8853  Circled plus
	"otimes"   #"^(2297)"   ; 8855  Circled times
	;-- Unit of measure symbols
	"deg"      #"^(B0)"     ; 176   Degrees
	"micro"    #"^(B5)"     ; 181   Micro
	;-- Copyright, registered, trademark
	"copy"     #"^(A9)"     ; 169   Copyright
	"reg"      #"^(AE)"     ; 174   Registered trademark
	"trade"    #"^(2122)"   ; 8482  Trademark
	;-- Currency symbols
	"curren"   #"^(A4)"     ; 164   Currency sign
	"cent"     #"^(A2)"     ; 162   Cents
	"pound"    #"^(A3)"     ; 163   British pounds
	"euro"     #"^(20AC)"   ; 8364  Euro
	"yen"      #"^(A5)"     ; 165   Yen
	;-- Greek alphabet
	"Alpha"    #"^(391)"    ; 913   UPPERCASE ALPHA
	"alpha"    #"^(3B1)"    ; 945   lowercase alpha
	"Beta"     #"^(392)"    ; 914   UPPERCASE BETA
	"beta"     #"^(3B2)"    ; 946   lowercase beta
	"Gamma"    #"^(393)"    ; 915   UPPERCASE GAMMA
	"gamma"    #"^(3B3)"    ; 947   lowercase gamma
	"Delta"    #"^(394)"    ; 916   UPPERCASE DELTA
	"delta"    #"^(3B4)"    ; 948   lowercase delta
	"Epsilon"  #"^(395)"    ; 917   UPPERCASE EPSILON
	"epsilon"  #"^(3B5)"    ; 949   lowercase epsilon
	"Zeta"     #"^(396)"    ; 918   UPPERCASE ZETA
	"zeta"     #"^(3B6)"    ; 950   lowercase zeta
	"Eta"      #"^(397)"    ; 919   UPPERCASE ETA
	"eta"      #"^(3B7)"    ; 951   lowercase eta
	"Theta"    #"^(398)"    ; 920   UPPERCASE THETA
	"theta"    #"^(3B8)"    ; 952   lowercase theta
	"thetasym" #"^(3D1)"    ; 977   alternate lowercase theta
	"Iota"     #"^(399)"    ; 921   UPPERCASE IOTA
	"iota"     #"^(3B9)"    ; 953   lowercase iota
	"Kappa"    #"^(39A)"    ; 922   UPPERCASE KAPPA
	"kappa"    #"^(3BA)"    ; 954   lowercase kappa
	"Lambda"   #"^(39B)"    ; 923   UPPERCASE LAMBDA
	"lambda"   #"^(3BB)"    ; 955   lowercase lambda
	"Mu"       #"^(39C)"    ; 924   UPPERCASE MU
	"mu"       #"^(3BC)"    ; 956   lowercase mu
	"Nu"       #"^(39D)"    ; 925   UPPERCASE NU
	"nu"       #"^(3BD)"    ; 957   lowercase nu
	"Xi"       #"^(39E)"    ; 926   UPPERCASE XI
	"xi"       #"^(3BE)"    ; 958   lowercase xi
	"Omicron"  #"^(39F)"    ; 927   UPPERCASE OMICRON
	"omicron"  #"^(3BF)"    ; 959   lowercase omicron
	"Pi"       #"^(3A0)"    ; 928   UPPERCASE PI
	"pi"       #"^(3C0)"    ; 960   lowercase pi
	"piv"      #"^(3D6)"    ; 982   alternative lowercase pi
	"Rho"      #"^(3A1)"    ; 929   UPPERCASE RHO
	"rho"      #"^(3C1)"    ; 961   lowercase rho
	"Sigma"    #"^(3A3)"    ; 931   UPPERCASE SIGMA
	"sigma"    #"^(3C3)"    ; 963   lowercase sigma
	"sigmaf"   #"^(3C2)"    ; 962   final form lowercase sigma
	"Tau"      #"^(3A4)"    ; 932   UPPERCASE TAU
	"tau"      #"^(3C4)"    ; 964   lowercase tau
	"Upsilon"  #"^(3A5)"    ; 933   UPPERCASE UPSILON
	"upsilon"  #"^(3C5)"    ; 965   lowercase upsilon
	"upsih"    #"^(3D2)"    ; 978   alternative lowercase upsilon
	"Phi"      #"^(3A6)"    ; 934   UPPERCASE PHI
	"phi"      #"^(3C6)"    ; 966   lowercase phi
	"Chi"      #"^(3A7)"    ; 935   UPPERCASE CHI
	"chi"      #"^(3C7)"    ; 967   lowercase chi
	"Psi"      #"^(3A8)"    ; 936   UPPERCASE PSI
	"psi"      #"^(3C8)"    ; 968   lowercase psi
	"Omega"    #"^(3A9)"    ; 937   UPPERCASE OMEGA
	"omega"    #"^(3C9)"    ; 969   lowercase omega
	;-- Arrows
	"larr"     #"^(2190)"   ; 8592  Left arrow
	"uarr"     #"^(2191)"   ; 8593  Up arrow
	"rarr"     #"^(2192)"   ; 8594  Right arrow
	"darr"     #"^(2193)"   ; 8595  Down arrow
	"harr"     #"^(2194)"   ; 8596  Left &amp; right arrow
	"crarr"    #"^(21B5)"   ; 8629  Carriage return arrow
	;-- Spade, club, heart, diamond
	"spades"   #"^(2660)"   ; 9824  Spade
	"clubs"    #"^(2663)"   ; 9827  Club
	"hearts"   #"^(2665)"   ; 9829  Heart
	"diams"    #"^(2666)"   ; 9830  Diamond
	;-- Accented letters
	"Agrave"   #"^(C0)"     ; 192   CAPITAL A GRAVE ACCENT
	"agrave"   #"^(E0)"     ; 224   lowercase a grave accent
	"Aacute"   #"^(C1)"     ; 193   CAPITAL A ACUTE ACCENT
	"aacute"   #"^(E1)"     ; 225   lowercase a acute accent
	"Acirc"    #"^(C2)"     ; 194   CAPITAL A CIRCUMFLEX ACCENT
	"acirc"    #"^(E2)"     ; 226   lowercase a circumflex accent
	"Atilde"   #"^(C3)"     ; 195   CAPITAL A TILDE ACCENT
	"atilde"   #"^(E3)"     ; 227   lowercase a tilde accent
	"Auml"     #"^(C4)"     ; 196   CAPITAL A UMLAUT ACCENT
	"auml"     #"^(E4)"     ; 228   lowercase a umlaut accent
	"Aring"    #"^(C5)"     ; 197   CAPITAL A RING ABOVE ACCENT
	"aring"    #"^(E5)"     ; 229   lowercase a ring accent
	"AElig"    #"^(C6)"     ; 198   CAPITAL AE
	"aelig"    #"^(E6)"     ; 230   lowercase ae
	"Ccedil"   #"^(C7)"     ; 199   CAPITAL C CEDILLA ACCENT
	"ccedil"   #"^(E7)"     ; 231   lowercase c cedilla accent
	"Egrave"   #"^(C8)"     ; 200   CAPITAL E GRAVE ACCENT
	"egrave"   #"^(E8)"     ; 232   lowercase e grave accent
	"Eacute"   #"^(C9)"     ; 201   CAPITAL E ACUTE ACCENT
	"eacute"   #"^(E9)"     ; 233   lowercase e acute accent
	"Ecirc"    #"^(CA)"     ; 202   CAPITAL E CIRCUMFLEX ACCENT
	"ecirc"    #"^(EA)"     ; 234   lowercase e circumflex accent
	"ecirc"    #"^(EA)"     ; 234   lowercase e circumflex accent
	"Euml"     #"^(CB)"     ; 203   CAPITAL E UMLAUT ACCENT
	"euml"     #"^(EB)"     ; 235   lowercase e umlaut accent
	"Igrave"   #"^(CC)"     ; 204   CAPITAL I GRAVE ACCENT
	"igrave"   #"^(EC)"     ; 236   lowercase i grave accent
	"Iacute"   #"^(CD)"     ; 205   CAPITAL I ACUTE ACCENT
	"iacute"   #"^(ED)"     ; 237   lowercase i acute accent
	"Icirc"    #"^(CE)"     ; 206   CAPITAL I CIRCUMFLEX ACCENT
	"icirc"    #"^(EE)"     ; 238   lowercase i circumflex accent
	"Iuml"     #"^(CF)"     ; 207   CAPITAL I UMLAUT ACCENT
	"iuml"     #"^(EF)"     ; 239   lowercase i umlaut accent
	"ETH"      #"^(D0)"     ; 208   CAPITAL ICELANDIC ETH
	"eth"      #"^(F0)"     ; 240   lowercase Icelandic eth
	"Ntilde"   #"^(D1)"     ; 209   CAPITAL N TILDE ACCENT
	"ntilde"   #"^(F1)"     ; 241   lowercase n tilde accent
	"Ograve"   #"^(D2)"     ; 210   CAPITAL O GRAVE ACCENT
	"ograve"   #"^(F2)"     ; 242   lowercase o grave accent
	"Oacute"   #"^(D3)"     ; 211   CAPITAL O ACUTE ACCENT
	"oacute"   #"^(F3)"     ; 243   lowercase o acute accent
	"Ocirc"    #"^(D4)"     ; 212   CAPITAL O CIRCUMFLEX ACCENT
	"ocirc"    #"^(F4)"     ; 244   lowercase o circumflex accent
	"Otilde"   #"^(D5)"     ; 213   CAPITAL O TILDE ACCENT
	"otilde"   #"^(F5)"     ; 245   lowercase o tilde accent
	"Ouml"     #"^(D6)"     ; 214   CAPITAL O UMLAUT ACCENT
	"ouml"     #"^(F6)"     ; 246   lowercase o umlaut accent
	"Oslash"   #"^(D8)"     ; 216   CAPITAL O SLASH ACCENT
	"oslash"   #"^(F8)"     ; 248   lowercase o slash
	"Ugrave"   #"^(D9)"     ; 217   CAPITAL U GRAVE ACCENT
	"ugrave"   #"^(F9)"     ; 249   lowercase u grave accent
	"Uacute"   #"^(DA)"     ; 218   CAPITAL U ACUTE ACCENT
	"uacute"   #"^(FA)"     ; 250   lowercase u acute accent
	"Ucirc"    #"^(DB)"     ; 219   CAPITAL U CIRCUMFLEX ACCENT
	"ucirc"    #"^(FB)"     ; 251   lowercase u circumflex accent
	"Uuml"     #"^(DC)"     ; 220   CAPITAL U UMLAUT
	"uuml"     #"^(FC)"     ; 252   lowercase u umlaut accent
	"Yacute"   #"^(DD)"     ; 221   CAPITAL Y ACUTE ACCENT
	"yacute"   #"^(FD)"     ; 253   lowercase y acute accent
	"yuml"     #"^(FF)"     ; 255   lowercase y umlaut accent
	"THORN"    #"^(DE)"     ; 222   CAPITAL ICELANDIC THORN
	"thorn"    #"^(FE)"     ; 254   lowercase Icelandic thorn
	"szlig"    #"^(DF)"     ; 223   lowercase German sharp s
	;-- Miscellaneous
	"bull"     #"^(2022)"   ; 8226  Bullet
	"hellip"   #"^(2026)"   ; 8230  Horizontal ellipsis
	"fnof"     #"^(192)"    ; 402   lowercase Latin f with hook
	"perp"     #"^(22A5)"   ; 8869  Perpendicular
	"sdot"     #"^(22C5)"   ; 8901  Dot operator
	"OElig"    #"^(152)"    ; 338   UPPERCASE LATIN OE LIGATURE
	"oelig"    #"^(153)"    ; 339   lowercase Latin oe ligature
	"Scaron"   #"^(160)"    ; 352   UPPERCASE S WITH CARON
	"scaron"   #"^(161)"    ; 353   lowercase s with caron
	"Yuml"     #"^(178)"    ; 376   CAPITAL Y WITH DIAERES
	"circ"     #"^(2C6)"    ; 710   Circumflex accent
	"tilde"    #"^(2DC)"    ; 732   Tilde (different from the tilde my keyboard generates)
	"ndash"    #"^(2013)"   ; 8211  En dash
	"mdash"    #"^(2014)"   ; 8212  Em dash
	"lsquo"    #"^(2018)"   ; 8216  Left single quotation mark
	"rsquo"    #"^(2019)"   ; 8217  Right single quotation mark
	"sbquo"    #"^(201A)"   ; 8218  Single low-9 quotation mark
	"ldquo"    #"^(201C)"   ; 8220  Left double quotation mark
	"rdquo"    #"^(201D)"   ; 8221  Right double quotation mark
	"bdquo"    #"^(201E)"   ; 8222  Double low-9 quotation mark
	"dagger"   #"^(2020)"   ; 8224  Dagger
	"Dagger"   #"^(2021)"   ; 8225  Double dagger
	"permil"   #"^(2030)"   ; 8240  Per mille
	"prime"    #"^(2032)"   ; 8242  Minutes (Degrees)
	"Prime"    #"^(2033)"   ; 8243  Seconds (Degrees)
	"lsaquo"   #"^(2039)"   ; 8249  Single left angle quotation
	"rsaquo"   #"^(2039)"   ; 8249  Single right angle quotation
	"oline"    #"^(203E)"   ; 8254  Overline
	"lceil"    #"^(2308)"   ; 8968  Left ceiling
	"rceil"    #"^(2309)"   ; 8969  Right ceiling
	"lfloor"   #"^(230A)"   ; 8970  Left floor
	"rfloor"   #"^(230B)"   ; 8971  Right floor
	"loz"      #"^(25CA)"   ; 9674  Lozenge
	"ensp"     #"^(2002)"   ; 8194  En space
	"emsp"     #"^(2003)"   ; 8195  Em space
	"thinsp"   #"^(2009)"   ; 8201  Thin space
	"zwnj"     #"^(200C)"   ; 8204  Zero width non-joiner
	"zwj"      #"^(200D)"   ; 8205  Zero width joiner
	"lrm"      #"^(200E)"   ; 8206  Left-to-right mark
	"rlm"      #"^(200F)"   ; 8207  Right-to-left mark
]]

any-except-&: complement charset "&"
alphanum:  charset [#"0" - #"9" #"a" - #"z" #"A" - #"Z"]
digits:    charset [#"0" - #"9"]

register-codec [
	name:  'html-entities
	type:  'text
	title: "Reserved characters in HTML"

	decode: func [
		{Creates a new string with possible HTML entities converted to chars}
		text [string! binary! file!]
		/local out s e char
	] [
		case [
			file?   text [text: read/string text]
			binary? text [text: to string!  text]
		]
		out: make string! length? text
		parse text [
			any [
				s: some any-except-& e: ( append/part out s e )
				| #"&" [
					#"#" copy char 1 4 digits #";" (
						append out to char! to integer! char
					)
					| s: copy char 1 10 alphanum #";" e: (
						char: select/case html-entities char
						unless char [ char: #"&" e: :s ]
						append out char
					) :e
					| (append out #"&")
				]
			]
		]
		out
	]
]