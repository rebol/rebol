REBOL [
	Name:    mime-types
	Type:    module
	Options: [delay]
	Version: 1.0.0
	Date:    13-Jul-2022
	File:    %mezz-mime-types.reb
	Title:   "Codec: MIME (media) types"
	Author:  @Oldes
	Rights:  "Copyright (C) 2022 Oldes. All rights reserved."
	License: MIT
	Exports: [mime-type?]
]

; temporary function used just for the initialization...
mime-type?: func[/local types type files][
	unless find system/options 'mime-types [
		put system/options 'mime-types #[none]
	]
	types: any [system/options/mime-types make map! 110]
	parse [
		;- collected from https://github.com/nginx/nginx/blob/master/conf/mime.types
		;; full list: https://www.iana.org/assignments/media-types/media-types.xhtml
		"text/html"                                        %.html %.htm %.shtml
		"text/css"                                         %.css
		"text/xml"                                         %.xml
		"text/mathml"                                      %.mml
		"text/plain"                                       %.txt
		"text/vnd.sun.j2me.app-descriptor"                 %.jad
		"text/vnd.wap.wml"                                 %.wml
		"text/x-component"                                 %.htc
		"image/gif"                                        %.gif
		"image/jpeg"                                       %.jpeg %.jpg
		"image/avif"                                       %.avif
		"image/png"                                        %.png
		"image/svg+xml"                                    %.svg %.svgz
		"image/tiff"                                       %.tif %.tiff
		"image/vnd.wap.wbmp"                               %.wbmp
		"image/webp"                                       %.webp
		"image/x-icon"                                     %.ico
		"image/x-jng"                                      %.jng
		"image/x-ms-bmp"                                   %.bmp
		"font/woff"                                        %.woff
		"font/woff2"                                       %.woff2
		"application/javascript"                           %.js
		"application/atom+xml"                             %.atom
		"application/rss+xml"                              %.rss
		"application/java-archive"                         %.jar %.war %.ear
		"application/json"                                 %.json
		"application/mac-binhex40"                         %.hqx
		"application/msword"                               %.doc
		"application/pdf"                                  %.pdf
		"application/postscript"                           %.ps %.eps %.ai
		"application/rtf"                                  %.rtf
		"application/vnd.apple.mpegurl"                    %.m3u8
		"application/vnd.google-earth.kml+xml"             %.kml
		"application/vnd.google-earth.kmz"                 %.kmz
		"application/vnd.ms-excel"                         %.xls
		"application/vnd.ms-fontobject"                    %.eot
		"application/vnd.ms-powerpoint"                    %.ppt
		"application/vnd.oasis.opendocument.graphics"      %.odg
		"application/vnd.oasis.opendocument.presentation"  %.odp
		"application/vnd.oasis.opendocument.spreadsheet"   %.ods
		"application/vnd.oasis.opendocument.text"          %.odt
		"application/vnd.openxmlformats-officedocument.presentationml.presentation"  %.pptx
		"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"          %.xlsx
		"application/vnd.openxmlformats-officedocument.wordprocessingml.document"    %.docx
		"application/vnd.wap.wmlc"                         %.wmlc
		"application/wasm"                                 %.wasm
		"application/x-7z-compressed"                      %.7z
		"application/x-cocoa"                              %.cco
		"application/x-java-archive-diff"                  %.jardiff
		"application/x-java-jnlp-file"                     %.jnlp
		"application/x-makeself"                           %.run
		"application/x-perl"                               %.pl %.pm
		"application/x-pilot"                              %.prc %.pdb
		"application/x-rar-compressed"                     %.rar
		"application/x-redhat-package-manager"             %.rpm
		"application/x-sea"                                %.sea
		"application/x-shockwave-flash"                    %.swf
		"application/x-stuffit"                            %.sit
		"application/x-tcl"                                %.tcl %.tk
		"application/x-x509-ca-cert"                       %.der %.pem %.crt
		"application/x-xpinstall"                          %.xpi
		"application/xhtml+xml"                            %.xhtml
		"application/xspf+xml"                             %.xspf
		"application/zip"                                  %.zip
		"application/octet-stream"                         %.bin %.exe %.dll
		"application/octet-stream"                         %.deb
		"application/octet-stream"                         %.dmg
		"application/octet-stream"                         %.iso %.img
		"application/octet-stream"                         %.msi %.msp %.msm
		"audio/midi"                                       %.mid %.midi %.kar
		"audio/mpeg"                                       %.mp3
		"audio/ogg"                                        %.ogg
		"audio/x-m4a"                                      %.m4a
		"audio/x-realaudio"                                %.ra
		"video/3gpp"                                       %.3gpp %.3gp
		"video/mp2t"                                       %.ts
		"video/mp4"                                        %.mp4
		"video/mpeg"                                       %.mpeg %.mpg
		"video/quicktime"                                  %.mov
		"video/webm"                                       %.webm
		"video/x-flv"                                      %.flv
		"video/x-m4v"                                      %.m4v
		"video/x-mng"                                      %.mng
		"video/x-ms-asf"                                   %.asx %.asf
		"video/x-ms-wmv"                                   %.wmv
		"video/x-msvideo"                                  %.avi
		"message/rfc822"                                   %.eml ;=> https://www.w3.org/Protocols/rfc1341/7_3_Message.html
	][
		some [
			set type string! copy files some file! (
				foreach file files [ types/:file: type ]
			)
		]
	]
	system/options/mime-types: :types
]
; initialize...
mime-type?
; redefine the function into something useful...
mime-type?: func[
	"Returns file's MIME's content-type"
	file [file!]
][
	select system/options/mime-types find/last file #"."
]