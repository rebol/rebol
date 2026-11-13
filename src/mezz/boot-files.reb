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
	Note: "Used by tools/make-boot.reb"
]

;-- base: low-level boot in lib context:
[
	%base-constants.reb
	%base-funcs.reb
	%base-series.reb
	%base-files.reb
	%base-debug.reb
	%base-defs.reb
	%base-collected.reb ; contains automatically collected code from C files
]

;-- sys: low-level sys context:
[
	%sys-base.reb
	%sys-ports.reb
	%sys-codec.reb ; export to lib!
	%sys-load.reb
	%sys-start.reb
]

;-- lib: mid-level lib context:
[
	%mezz-secure.reb
	%mezz-types.reb
	%mezz-func.reb
	%mezz-debug.reb
	%mezz-control.reb
	%mezz-save.reb
	%mezz-series.reb
	%mezz-files.reb
	%mezz-shell.reb
	%mezz-math.reb
	%mezz-help.reb ; move dump-obj!
	%mezz-banner.reb
	%mezz-colors.reb
	%mezz-date.reb ; Internet date support
	;%mezz-tag.reb  ; build-tag
	%mezz-tail.reb
	%codec-unixtime.reb
	;- cryptographic
	%codec-utc-time.reb
	%codec-pkix.reb
	%codec-der.reb
	%codec-crt.reb
	%codec-ppk.reb
	%codec-ssh-key.reb
	;- compression
	%codec-gzip.reb
	%codec-zip.reb
	%codec-tar.reb
	;- other
	%codec-json.reb
	%codec-xml.reb
	%codec-html-entities.reb
	; optional files added in make-boot.reb per os and product
	;%codec-wav.reb
	;%codec-swf.reb
	;%codec-image.reb
]

;-- protocols:
[
	%prot-http.reb
	%prot-tls.reb
	%prot-whois.reb
	%prot-mysql.reb
]
