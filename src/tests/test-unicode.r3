REBOL [
	Title: "runicode4"
	Author: @ldci
	Version: 4.2
	History: [
		4.0 "Original"
		4.2 "Updated by Oldes"
	]
]

print-unicode-codes: function/with [
	"Prints chars of the given Unicode group"
	group [integer!] "Unicode group (1 - 95)"
][
	unless parse codes [
		group [thru LF]          ;; Move to the right group line (by skipping lines).
		thru SP                  ;; Skip Unicode hex start (eg. 0x0000-0x007F) to first space
		copy start: to #"-" skip ;; Grab the start decimal code (eg. 0)
		copy tail:  to SP   skip ;; Grab the end decimal code (eg. 127)
		copy label: to LF to end ;; Grab the label (eg. "Latin Basic")
	][ exit ]                    ;; If parsing failed, exit the function.
	start: to integer! start
	tail:  to integer! tail
	if start < 32 [start: 32]    ;; Skip unprintable chars (< 32)
	if find [55296 56192 56320] start  [
		;; Surrogate pairs are used in UTF-16 encoding
		;; and are not allowed in UTF-8!
		print ajoin as-purple ["Reserved UTF-16 chars:" label "(" start #"-" tail ")"]
		exit
	]
	i: start
	while [i <= tail] [
		print ajoin ["Unicode " i ": " to char! i " [" label "]"]
		++ i
	]
][
	codes: {
0x0000-0x007F 0-127 Latin Basic
0x0080-0x00FF 128-255 Latin-1 Supplement
0x0100-0x017F 256-383 Latin Extended-A
0x0180-0x024F 384-591 Latin Extended-B
0x02B0-0x02FF 688-767 Spacing Modifier Letters
0x0300-0x036F 768-879 Combining Diacritical Marks
0x0370-0x03FF 880-1023 Greek and Copte
0x0400-0x04FF 1024-1279 Cyrillic Basic
0x0500-0x052F 1280-1327 Cyrillic Supplement"
0x0530-0x058F 1328-1423 Armenian
0x0590-0x05FF 1424-1535 Hebrew
0x0600-0x06FF 1536-1791 Arabic
0x0700-0x074F 1792-1871 Syriac
0x0780-0x07BF 1920-1983 Thaana
0x0900-0x097F 2304-2431 Devanagari
0x0980-0x09FF 2432-2559 Bengali
0x0A00-0x0A7F 2560-2687 Gurmukhi
0x0A80-0x0AFF 2688-2815 Gujarati
0x0B00-0x0B7F 2816-2943 Oriya
0x0B80-0x0BFF 2944-3071 Tamil
0x0C00-0x0C7F 3072-3199 Telugu
0x0C80-0x0CFF 3200-3327 Kannada
0x0D00-0x0D7F 3328-3455 Malayalam
0x0D80-0x0DFF 3456-3583 Sinhala
0x0E00-0x0E7F 3584-3711 Thai
0x0E80-0x0EFF 3712-3839 Lao
0x0F00-0x0FFF 3840-4095 Tibetan
0x1000-0x109F 4096-4255 Myanmar
0x10A0-0x10FF 4256-4351 Georgian
0x1100-0x11FF 4352-4607 Hangul Jamo
0x1200-0x137F 4608-4991 Ethiopic
0x13A0-0x13FF 5024-5119 Cherokee
0x1400-0x167F 5120-5759 Unified Canadian Aboriginal Syllabics
0x1680-0x169F 5760-5791 Ogham
0x16A0-0x16FF 5792-5887 Runic
0x1780-0x17FF 6016-6143 Khmer
0x1800-0x18AF 6144-6319 Mongolian
0x1E00-0x1EFF 7680-7935 Latin Extended Additional
0x1F00-0x1FFF 7936-8191 Greek Extended
0x2000-0x206F 8192-8303 General Punctuation
0x2070-0x209F 8304-8351 Superscripts and Subscripts
0x20A0-0x20CF 8352-8399 Currency Symbols
0x20D0-0x20FF 8400-8447 Combining Marks for Symbols
0x2100-0x214F 8448-8527 Letterlike Symbols
0x2150-0x218F 8528-8591 Number Forms
0x2190-0x21FF 8592-8703 Arrows
0x2200-0x22FF 8704-8959 Mathematical Operators
0x2300-0x23FF 8960-9215 Miscellaneous Technical
0x2400-0x243F 9216-9279 Control Pictures
0x2440-0x244A 9280-9290 Optical Character Recognition"
0x2460-0x24FF 9312-9471 Enclosed Alphanumerics
0x2500-0x257F 9472-9599 Box Drawing
0x2580-0x259F 9600-9631 Block Elements
0x25A0-0x25FF 9632-9727 Geometric Shapes
0x2600-0x26FF 9728-9983 Miscellaneous Symbols
0x2700-0x27BF 9984-10175 Dingbats
0x2800-0x28FF 10240-10495 Braille Patterns
0x2E80-0x2EFF 11904-12031 CJK Radicals Supplement
0x2F00-0x2FDF 12032-12255 Kangxi Radicals
0x2FF0-0x2FFF 12272-12287 Ideographic Description Characters
0x3000-0x303F 12288-12351 CJK Symbols and Punctuation
0x3040-0x309F 12352-12447 Hiragana
0x30A0-0x30FF 12448-12543 Katakana
0x3100-0x312F 12544-12591 Bopomofo
0x3130-0x318F 12592-12687 Hangul Compatibility Jamo
0x3190-0x319F 12688-12703 Kanbun
0x31A0-0x31BF 12704-12735 Bopomofo Extended
0x3200-0x32FF 12800-13055 Enclosed CJK Letters and Months
0x3300-0x33FF 13056-13311 CJK Compatibility
0x3400-0x4DB5 13312-19893 CJK Unified Ideographs Extension A
0x4E00-0x9FFF 19968-40959 CJK Unified Ideographs
0xA000-0xA48F 40960-42127 Yi Syllables
0xA490-0xA4CF 42128-42191 Yi Radicals
0xAC00-0xD7A3 44032-55203 Hangul Syllables
0xD800-0xDB7F 55296-56191 High Surrogates
0xDB80-0xDBFF 56192-56319 High Private Use Surrogates
0xDC00-0xDFFF 56320-57343 Low Surrogates
0xE000-0xF8FF 57344-63743 Private Use
0xF900-0xFAFF 63744-64255 CJK Compatibility Ideographs
0xFB00-0xFB4F 64256-64335 Alphabetic Presentation Forms
0xFB50-0xFDFF 64336-65023 Arabic Presentation Forms-A
0xFE20-0xFE2F 65056-65071 Combining Half Marks
0xFE30-0xFE4F 65072-65103 CJK Compatibility Forms
0xFE50-0xFE6F 65104-65135 Small Form Variants
0xFE70-0xFEFE 65136-65278 Arabic Presentation Forms-B
0xFEFF-0xFEFF 65279-65279 Specials
0xFF00-0xFFEF 65280-65519 Halfwidth and Fullwidth Forms
0xFFF0-0xFFFD 65520-65533 Specials
0x1F300-0x1F3C0 127744-127936 Miscellaneous Pictograms 1
0x1F3C0-0x1F483 127937-128129 Miscellaneous Pictograms 2
0x1F3C1-0x1F542 128130-128322 Miscellaneous Pictograms 3
0x1F543-0x1F5FF 128323-128511 Miscellaneous Pictograms 4
0x1F600-0x1F64F 128512-128591 Emoticons
0x1F680-0x1F6FF 128640-128767 Transport/Map
0x1F900-0X1F9FF 129280-129535 Supplemental Symbols and Pictographs
}
]

;****************** test program ********************
;t: dt [print-unicode-codes 93] ;--max: 95
;print ["Generated in" (third t * 1000 )"ms"]

repeat i 95 [print-unicode-codes i]