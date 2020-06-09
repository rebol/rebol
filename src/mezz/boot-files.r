REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "REBOL 3 Boot: System Contexts"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Note: "Used by tools/make-boot.r"
]

;-- base: low-level boot in lib context:
[
	%base-constants.r
	%base-funcs.r
	%base-series.r
	%base-files.r
	%base-debug.r
	%base-defs.r
	%base-collected.r ; contains automatically collected code from C files
]

;-- sys: low-level sys context:
[
	%sys-base.r
	%sys-ports.r
	%sys-codec.r ; export to lib!
	%sys-load.r
	%sys-start.r
]

;-- lib: mid-level lib context:
[
	%mezz-secure.r
	%mezz-types.r
	%mezz-func.r
	%mezz-debug.r
	%mezz-control.r
	%mezz-save.r
	%mezz-series.r
	%mezz-files.r
	%mezz-shell.r
	%mezz-math.r
	%mezz-help.r ; move dump-obj!
	%mezz-banner.r
	%mezz-colors.r
	%mezz-date.r ; Internet date support
	;%mezz-tag.r  ; build-tag
	%mezz-tail.r
	%codec-unixtime.r
	;- cryptographic
	%codec-utc-time.r
	%codec-pkix.r
	%codec-der.r
	%codec-crt.r
	%codec-ssh-key.r
	;- compression
	%codec-gzip.r
	%codec-zip.r
	%codec-tar.r
	;- other
	%codec-json.r
	%codec-xml.r
	%codec-html-entities.r
	; optional files added in make-boot.r per os and product
	;%codec-wav.r
	;%codec-swf.r
	;%codec-image.r
]

;-- protocols:
[
	%prot-http.r
	%prot-tls.r
	%prot-whois.r
	%prot-mysql.r
]
