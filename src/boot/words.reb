REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Canonical words"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Purpose: {
		These words are used internally by REBOL and must have specific canon
		word values in order to be correctly identified.
	}
]

any-type!
any-word!
any-path!
any-function!
number!
scalar!
series!
any-string!
any-object!
any-block!

;- numeric types (used for "short" vector! construction)
;- like: #[u8! 4] which is same like: make vector! [unsigned integer! 8 4]
;  these numeric files are used in l-types.c and t-vector.c sources
;  and can be used for future struct! and routine! implementations
i8!
i16!
i32!
i64!

u8!
u16!
u32!
u64!

f32!
f64!

int8!
int16!
int32!
int64!

uint8!
uint16!
uint32!
uint64!

float!
double!
byte!
; end of numeric types (code in l-types.c expects, that next word is `datatypes`)

datatypes 

native
self
none
true
false
on
off
yes
no
pi

rebol

system

;boot levels
base
sys
mods

;reflectors:
spec
body
words
values
types
title

x
y
+
-
*
unsigned
-unnamed- 	; lambda (unnamed) functions
-apply-		; apply func
code		; error field
delect

; Secure:  (add to system/state/policies object too)
secure
protect
net
call
envr
eval
memory
debug
browse
extension
;dir - below
;file - below

; Date & time:
year
month
day
time
date
zone
hour
minute
second
weekday
yearday
timezone
;week
;isoweek
utc
julian


; Parse: - These words must not reserved above!!
parse
|	 ; must be first
; prep words:
set  
copy
some
any
opt
not
and
ahead
then
remove
insert
change
if
fail
reject
while
return
limit
??
accept
break
; match words:
skip
to
thru
quote
do
into
only
end  ; must be last

; Event:
type
key
port
mode
window
double
control
shift

; Filters (used in image resizing so far)
point
box
triangle
hermite
hanning
hamming
blackman
gaussian
quadratic
cubic
catrom
mitchell
lanczos
bessel
sinc

; Checksum
hash
adler32
crc24
crc32
md4
md5
ripemd160
sha1
sha224
sha256
sha384
sha512

; Codec actions
identify
decode
encode

; Schemes
console
file
dir
event
callback
dns
tcp
udp
clipboard

; Gobs:
gob
offset
size
pane
parent
image
draw
text
effect
color
flags
rgb
alpha
data
resize
no-title
no-border
dropable
transparent
popup
modal
on-top
hidden
owner

*port-modes*     ;@@ modes are defined in modes.r file and these
*console-modes*  ;@@ placeholders are replaced here by make-boot.r script

bits
crash
crash-dump
watch-recycle
watch-obj-copy
stack-size

;call/info
id
exit-code

pub-exp
priv-exp

r-buffer
w-buffer
r-mask
w-mask
not-bit
unixtime-now
unixtime-now-le
random-bytes
length?
buffer-cols
buffer-rows
window-cols
window-rows

devices-in
devices-out

msdos-datetime
msdos-date
msdos-time
octal-bytes
string-bytes