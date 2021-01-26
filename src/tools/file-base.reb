REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Source File Database"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Author: "Carl Sassenrath"
	Purpose: {
		Lists of files used for creating makefiles.
	}
]

core: [
	a-constants.c
	a-globals.c
	a-lib.c
	b-boot.c
	b-init.c
	c-do.c
	c-error.c
	c-frame.c
	c-function.c
	c-handle.c
	c-port.c
	c-task.c
	c-word.c
	d-crash.c
	d-dump.c
	d-print.c
	f-blocks.c
	f-deci.c
	f-dtoa.c
	f-enbase.c
	f-extension.c
	f-int.c
	f-math.c
	f-modify.c
	f-qsort.c
	f-random.c
	f-round.c
	f-series.c
	f-stubs.c
	l-scan.c
	l-types.c
	m-gc.c
	m-pools.c
	m-series.c
	n-control.c
	n-crypt.c
	n-data.c
	n-image.c
	n-io.c
	n-loop.c
	n-math.c
	n-sets.c
	n-strings.c
	n-system.c
	p-checksum.c
	p-clipboard.c
	p-console.c
	p-dir.c
	p-dns.c
	p-event.c
	p-file.c
	p-net.c
	p-midi.c
	s-cases.c
	s-crc.c
	s-file.c
	s-find.c
	s-make.c
	s-mold.c
	s-ops.c
	s-trim.c
	s-unicode.c
	t-bitset.c
	t-block.c
	t-char.c
	t-datatype.c
	t-date.c
	t-decimal.c
	t-event.c
	t-function.c
	t-gob.c
	t-image.c
	t-integer.c
	t-logic.c
	t-map.c
	t-money.c
	t-none.c
	t-object.c
	t-pair.c
	t-port.c
	t-string.c
	t-time.c
	t-tuple.c
	t-typeset.c
	t-utype.c
	t-vector.c
	t-word.c
	u-aes.c
	u-bigint.c     ; needed for RSA which is needed in TLS protocol (HTTPS)
	u-bincode.c
	u-bmp.c
	u-chacha20.c
	u-compress.c
	u-dh.c
	u-dialect.c
	u-gif.c
	u-iconv.c
	u-image-resize.c
	u-jpg.c
	u-lzma.c
	u-parse.c
	u-png.c
	u-poly1305.c
	u-rc4.c
	u-rsa.c       ; needed in TLS protocol (HTTPS)
	u-uECC.c
	u-zlib.c
	u-wav.c

	;- Mbed TLS related files (only checksums used so far)
	u-mbedtls.c
	; platform
	mbedtls/platform.c       ; Platform abstraction layer
	mbedtls/platform_util.c  ; Common and shared functions used by multiple modules in the Mbed TLS
	; checksums:
	mbedtls/md4.c
	mbedtls/md5.c
	mbedtls/ripemd160.c
	mbedtls/sha1.c
	mbedtls/sha256.c
	mbedtls/sha512.c

	;- deprecated checksum files:
	;u-md5.c
	;u-sha1.c
	;u-sha2.c
]

made: [
	make-boot.reb			core/b-boot.c
	make-headers.reb		include/tmp-funcs.h
	make-host-ext.reb		include/host-ext-graphics.h
	make-host-init.reb	    include/host-init.h
	make-os-ext.reb		    include/host-lib.h
	make-reb-lib.reb		include/reb-lib.h
]

os: [
	host-main.c
	host-args.c
	host-device.c
	host-stdio.c
	dev-net.c
	dev-dns.c

	host-ext-test.c
]

os-win32: [
	host-lib.c
	dev-stdio.c
	dev-file.c
	dev-event.c
	dev-clipboard.c
	dev-midi.c

;	host-event.c
;	host-window.c
;	host-compositor.c
;	host-image.c
]

os-win32g: [
	host-event.c
	host-window.c
	host-compositor.c
	host-image.c

	sys-utils.c
	sys-codecs.cpp

	;host-graphics.c
	;host-draw.c
	;host-text.c
]

os-posix: [
	host-lib.c
	host-readline.c
	dev-stdio.c
	dev-event.c
	dev-file.c
	;dev-clipboard.c
	
	dev-midi-osx.c
]

boot-files: [
	version.reb
	graphics.reb
	draw.reb
	shape.reb
	text.reb
]

mezz-files: [
;	prot-http.reb
;	view-colors.reb
	view-funcs.reb
]

agg-files: [
	agg_arc.cpp
	agg_arrowhead.cpp
	agg_bezier_arc.cpp
	agg_bspline.cpp
	agg_curves.cpp
	agg_image_filters.cpp
	agg_line_aa_basics.cpp
	agg_path_storage.cpp
	agg_rasterizer_scanline_aa.cpp
	agg_rounded_rect.cpp
	agg_sqrt_tables.cpp
	agg_trans_affine.cpp
	agg_trans_single_path.cpp
	agg_vcgen_bspline.cpp
	agg_vcgen_contour.cpp
	agg_vcgen_dash.cpp
	agg_vcgen_markers_term.cpp
	agg_vcgen_smooth_poly1.cpp
	agg_vcgen_stroke.cpp
	agg_vpgen_segmentator.cpp
	agg_compo.cpp
	agg_graphics.cpp
;	agg_font_freetype.cpp
	agg_font_win32_tt.cpp
	agg_truetype_text.cpp
;	agg_effects.cpp
	compositor.cpp
	graphics.cpp
	rich_text.cpp
]

tools: [
	make-host-init.reb
	make-host-ext.reb
	form-header.reb
]

