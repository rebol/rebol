Rebol [
	title:   "Thru Cache functions"
	purpose: "Functions for accessing remote files through a local cache"
	name:    thru-cache
	type:    module
	options: [delay]
	version: 0.2.1
	exports: [path-thru read-thru load-thru do-thru clear-thru list-thru exists-thru? delete-thru]
	author:  @Oldes
	file:    %thru-cache.reb
	home:    https://src.rebol.tech/modules/thru-cache.reb
]

;; Initialize the directory for storing local files
so: system/options
so/log/thru-cache: 2

unless select so 'thru-cache [
	put so 'thru-cache join to-real-file any [
		get-env "TEMP"
		so/data
	] %thru-cache/
	sys/log/info 'THRU-CACHE ["Content directory:" mold so/thru-cache]
]

path-thru: func [
	{Returns the local disk cache path of a remote file} 
	url [url!] "Remote file address" 
	return: [file!] 
	/local hash file path
][
	unless find so 'thru-cache [return none]
	hash: checksum mold url 'MD5 
	file: head (remove back tail remove remove (form hash)) 
	path: dirize append copy so/thru-cache copy/part file 2 
	unless exists? path [make-dir/deep path] 
	append path file
]

read-thru: func [
	"Reads a remote file through local disk cache" 
	url [url!] "Remote file address" 
	/update "Force a cache update"
	/string "Try convert result to string"
	/local path data code
][
	path: path-thru url 
	either all [not update exists? path] [
		data: read/binary path
	][
		data: read/binary/all url
		code: any [all [block? data data/1] 200]
		if all [
			block?  data
			object? data/2
			binary? data/3
		][
			data: any [
				attempt [decompress data/3 to word! data/2/Content-Encoding]
				data/3
			]
		]
		either code == 200 [
			try [
				write/binary path data
				log-thru-file path url
				sys/log/more 'THRU-CACHE ["Stored:" mold url]
			]
		][
			sys/log/error 'THRU-CACHE ["Response" code " Not stored:" mold url]
		]
	]
	if string [try [data: to string! data]]
	data
]

load-thru: func [
	"Loads a remote file through local disk cache" 
	url [url!] "Remote file address" 
	/update "Force a cache update" 
	/as {Specify the type of data; use NONE to load as code} 
	type [word! none!] "E.g. text, markup, jpeg, unbound, etc." 
][
	load/as read-thru/:update url type
]

do-thru: func [
    {Evaluates a remote Rebol script through local disk cache} 
    url [url!] "Remote file address" 
    /update "Force a cache update"
][
    do read-thru/:update url
]

clear-thru: func [
	"Removes local disk cache files"
	/only filter [string!] "Delete only the files that match the filter"
	/test "Only print files to be deleted"
	/local temp dir log
][
	unless exists? dir: select so 'thru-cache [exit]
	log: dir/read-thru.log
	if test [
		unless exists? log [exit]
		filter: any [filter "*"]
		foreach [path url] transcode read log [
			if all [
				exists? dir/:path
				find/any/match url filter
			][	print [as-green skip path 3 mold url] ]
		]
		exit
	]
	either only [
		unless exists? log [exit]
		;; Go through the log file and decide which files should be deleted
		temp: transcode read log
		delete log
		foreach [path url] temp [
			if exists? dir/:path [
				either find/any url filter [
					try [delete dir/:path]   ;; delete the local file
				][	log-thru-file path url ] ;; write the path to the new log file
			]
		]
	][	;; Delete everything
		delete-dir dir
	]
	()
]

exists-thru?: func[
	"Returns true if the remote file is present in the local disk cache"
	url [url! file!] "Remote file address"
][
	exists? any [all [file? url url] path-thru url]
]

delete-thru: func[
	"Deletes a local disk cache file"
	url [url!] "Remote file address"
	/local path
][
	if exists? path: path-thru url [
		sys/log/more 'THRU-CACHE ["Removed:" mold url]
		delete path
	]
]

list-thru: func[
	"Prints localy stored URLs"
	/only filter [string!] "List only the files that match the filter"
][
	sys/log/info 'THRU-CACHE ["Content directory:" mold so/thru-cache]
	clear-thru/:only/test filter
]

log-thru-file: func[path url][
	write/append so/thru-cache/read-thru.log ajoin [
		#"%" copy/part tail path -32 SP mold url LF 
	]
]