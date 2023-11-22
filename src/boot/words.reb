REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Canonical words"
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2022 Rebol Open Source Contributors
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
collect
keep
return
limit
??
case
no-case
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
sha1
sha224
sha256
sha384
sha512
ripemd160
sha3-224
sha3-256
sha3-384
sha3-512

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
*serial-modes*
*audio-modes*

local-ip
local-port
remote-ip
remote-port

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

; group IDs for Elliptic Curves over GF(P) (ECP)
; (in the same order as mbedTLS's `mbedtls_ecp_group_id` enumeration)
secp192r1      ; 192-bit curve defined by FIPS 186-4 and SEC1
secp224r1      ; 224-bit curve defined by FIPS 186-4 and SEC1
secp256r1      ; 256-bit curve defined by FIPS 186-4 and SEC1
secp384r1      ; 384-bit curve defined by FIPS 186-4 and SEC1
secp521r1      ; 521-bit curve defined by FIPS 186-4 and SEC1
bp256r1        ; 256-bit Brainpool curve
bp384r1        ; 384-bit Brainpool curve
bp512r1        ; 512-bit Brainpool curve
curve25519     ; Curve25519
secp192k1      ; 192-bit "Koblitz" curve
secp224k1      ; 224-bit "Koblitz" curve
secp256k1      ; 256-bit "Koblitz" curve
curve448       ; Curve448

init-vector

; the order is important!
aes-128-ecb
aes-192-ecb
aes-256-ecb
aes-128-cbc
aes-192-cbc
aes-256-cbc
aes-128-ccm
aes-192-ccm
aes-256-ccm
aes-128-gcm
aes-192-gcm
aes-256-gcm
camellia-128-ecb
camellia-192-ecb
camellia-256-ecb
camellia-128-cbc
camellia-192-cbc
camellia-256-cbc
camellia-128-ccm
camellia-192-ccm
camellia-256-ccm
camellia-128-gcm
camellia-192-gcm
camellia-256-gcm
aria-128-ecb
aria-192-ecb
aria-256-ecb
aria-128-cbc
aria-192-cbc
aria-256-cbc
aria-128-ccm
aria-192-ccm
aria-256-ccm
aria-128-gcm
aria-192-gcm
aria-256-gcm
chacha20
chacha20-poly1305

tag-length
aad-length

file-checksum

xaudio-voice
