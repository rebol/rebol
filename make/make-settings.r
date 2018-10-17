REBOL [
	purpose: {User defined building properties}
]

TOOLS-Win32: "x:/MingW/mingw32/bin/"
TOOLS-Win64: "x:/MingW/mingw64/bin/"

;@@ If you add or remove some of these defines bellow, you should `make clean`
;@@ to make sure that all affected files will be recompiled!
Defines: [
	USE_BMP_CODEC
	USE_PNG_CODEC
	USE_GIF_CODEC
	USE_JPG_CODEC
	;USE_WAV_CODEC   ;-- deprecated; using Rebol codec instead
	;USE_NO_INFINITY ;-- use when you don't want to support IEEE infinity
]
