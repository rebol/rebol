REBOL [
	Title:   "Codec: MIME (media) types"
	Name:    mime-types
	Type:    module
	Options: [delay]
	Version: 1.2.0
	Date:    01-Apr-2025
	File:    %codec-mime-types.reb
	Author:  @Oldes
	Rights:  "Copyright (C) 2022-2025 Oldes. All rights reserved."
	License: MIT
	Exports: [mime-type?]
]

; temporary function used just for the initialization...
mime-type?: func[/local types type files][
	unless map? types: select system/catalog 'mime-types [
		put system/catalog 'mime-types types: make map! 111
	]
	parse [
		;- collected from https://github.com/nginx/nginx/blob/master/conf/mime.types
		;; full list: https://www.iana.org/assignments/media-types/media-types.xhtml
		"text/html"                                        %html %htm %shtml
		"text/css"                                         %css
		"text/xml"                                         %xml
		"text/mathml"                                      %mml
		"text/plain"                                       %txt
		"text/vnd.sun.j2me.app-descriptor"                 %jad
		"text/vnd.wap.wml"                                 %wml
		"text/x-component"                                 %htc
		"image/gif"                                        %gif
		"image/jpeg"                                       %jpeg %jpg
		"image/jxl"                                        %jxl
		"image/jp2"                                        %jp2
		"image/jpx"                                        %jpx
		"image/avif"                                       %avif
		"image/dds"                                        %dds
		"image/heic"                                       %heic
		"image/vnd.dwg"                                    %dwg
		"image/png"                                        %png
		"image/apng"                                       %apng
		"image/qoi"                                        %qoi
		"image/svg+xml"                                    %svg %svgz
		"image/tiff"                                       %tif %tiff
		"image/vnd.wap.wbmp"                               %wbmp
		"image/vnd.adobe.photoshop"                        %psd
		"image/vnd.ms-photo"                               %jxd
		"image/webp"                                       %webp
		"image/x-icon"                                     %ico
		"image/x-jng"                                      %jng
		"image/x-ms-bmp"                                   %bmp
		"image/x-xcf"                                      %xcf
		"image/x-canon-cr2"                                %cr2
		"font/woff"                                        %woff
		"font/woff2"                                       %woff2
		"application/gzip"                                 %gz
		"application/javascript"                           %js
		"application/atom+xml"                             %atom
		"application/rss+xml"                              %rss
		"application/java-archive"                         %jar %war %ear
		"application/json"                                 %json
		"application/mac-binhex40"                         %hqx
		"application/msword"                               %doc
		"application/dicom"                                %dcm
		"application/pdf"                                  %pdf
		"application/postscript"                           %ps %eps %ai
		"application/rtf"                                  %rtf
		"application/vnd.apple.mpegurl"                    %m3u8
		"application/vnd.google-earth.kml+xml"             %kml
		"application/vnd.google-earth.kmz"                 %kmz
		"application/vnd.ms-cab-compressed"                %cab
		"application/vnd.ms-excel"                         %xls
		"application/vnd.ms-fontobject"                    %eot
		"application/vnd.ms-powerpoint"                    %ppt
		"application/vnd.oasis.opendocument.graphics"      %odg
		"application/vnd.oasis.opendocument.presentation"  %odp
		"application/vnd.oasis.opendocument.spreadsheet"   %ods
		"application/vnd.oasis.opendocument.text"          %odt
		"application/vnd.openxmlformats-officedocument.presentationml.presentation"  %pptx
		"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"          %xlsx
		"application/vnd.openxmlformats-officedocument.wordprocessingml.document"    %docx
		"application/vnd.wap.wmlc"                         %wmlc
		"application/wasm"                                 %wasm
		"application/zstd"                                 %zst %zstd
		"application/x-7z-compressed"                      %7z
		"application/x-brotli"                             %br
		"application/x-bzip2"                              %bz2
		"application/x-cocoa"                              %cco
		"application/x-deb"                                %deb
		"application/x-executable"                         %elf
		"application/x-google-chrome-extension"            %crx
		"application/x-java-archive-diff"                  %jardiff
		"application/x-java-jnlp-file"                     %jnlp
		"application/x-lz4"                                %lz4
		"application/x-lzip"                               %lz
		"application/x-lzop"                               %lzop
		"application/x-makeself"                           %run
		"application/x-msdownload"                         %exe
		"application/x-nintendo-nes-rom"                   %nes
		"application/x-perl"                               %pl %pm
		"application/x-pilot"                              %prc %pdb
		"application/x-rar-compressed"                     %rar
		"application/x-rpm"                                %rpm
		"application/x-tar"                                %tar
		"application/x-redhat-package-manager"             %rpm
		"application/x-sea"                                %sea
		"application/x-shockwave-flash"                    %swf
		"application/x-sqlite3"                            %sqlite
		"application/x-stuffit"                            %sit
		"application/x-tcl"                                %tcl %tk
		"application/x-unix-archive"                       %ar
		"application/x-x509-ca-cert"                       %der %pem %crt
		"application/x-xpinstall"                          %xpi
		"application/x-xz"                                 %xz
		"application/x-compress"                           %Z
		"application/xhtml+xml"                            %xhtml
		"application/xspf+xml"                             %xspf
		"application/zip"                                  %zip
		"application/octet-stream"                         %bin %exe %dll
		                                                   %deb
		                                                   %dmg
		                                                   %iso %img
		                                                   %msi %msp %msm
		"application/epub+zip"                             %epub
		"audio/midi"                                       %mid %midi %kar
		"audio/mpeg"                                       %mp3
		"audio/ogg"                                        %ogg
		"audio/x-m4a"                                      %m4a
		"audio/x-realaudio"                                %ra
		"audio/x-wav"                                      %wav
		"video/3gpp"                                       %3gpp %3gp
		"video/mp2t"                                       %ts
		"video/mp4"                                        %mp4
		"video/mpeg"                                       %mpeg %mpg
		"video/quicktime"                                  %mov
		"video/webm"                                       %webm
		"video/x-flv"                                      %flv
		"video/x-m4v"                                      %m4v
		"video/x-mng"                                      %mng
		"video/x-ms-asf"                                   %asx %asf
		"video/x-ms-wmv"                                   %wmv
		"video/x-msvideo"                                  %avi
		"message/rfc822"                                   %eml ;=> https://www.w3.org/Protocols/rfc1341/7_3_Message.html
	][
		some [
			set type string! copy files some file! (
				protect type
				foreach file files [ types/:file: type ]
			)
		]
	]
]
; initialize...
mime-type?
; redefine the function into something useful...
mime-type?: func[
	"Returns file's MIME's content-type"
	file [file! word! none!]
	return: [string! none!]
][
	unless file [return none]
	file: either word? file [
		to file! file
	][	any [find/last/tail file #"." file]]
	select system/catalog/mime-types file
]