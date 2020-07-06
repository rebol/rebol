Rebol [
	Title: "Spotify"
	Purpose: "Spotify Web API access (experiment)"
	Date: 2-Jul-2020
	Author: "Oldes"
	File: %spotify.reb
	Name: 'spotify
	Type: 'module
	Version: 0.1.1
	Require: 'httpd
	Note: {
		Useful info:
		https://developer.spotify.com/documentation/general/guides/authorization-guide/
		https://aaronparecki.com/oauth-2-simplified/
	}
]
unless system/modules/httpd [
	print "Importing HTTPD module"
	do %httpd.reb
]

; context template as provided to user as a main access point to API
spotify: object [
	client-id:     none
	client-secret: none
	scope:         ""
	port-id:       8989
	token:         none
	get: func [what [any-string!]][request self 'GET what none]
	put: func [what [any-string!] /with data][request self 'PUT what data]
	del: func [what [any-string!]][request self 'DELETE what none]
	request: none
]

authorize: function [
	"OAuth2 Spotify authorization used to get the main context"
	ctx [block! object!] "Data used for initialization (at least client-id is needed)"
][
	ctx: make spotify ctx
	ctx/request: :request

	unless ctx/client-id [ 
		print ajoin ["*** `" value "` is needed to authorize with Spotify!"]
		return none
	]
	unless string?  ctx/client-id [ ctx/client-id: form ctx/client-id ]
	unless integer? ctx/port-id   [ ctx/port-id:   8989               ]
	unless string?  ctx/scope     [ ctx/scope:     form ctx/scope     ]

	; url-encode spaces in scopes 
	parse ctx/scope [any [change some #[bitset! #{0064000080}] #"+" | skip]]

	redirect-uri: rejoin [
		"http%3A%2F%2Flocalhost:" ctx/port-id "%2Fspotify-callback%2F"
	]

	unless ctx/client-secret [
		code-verifier: form random/secure checksum/method join ctx/client-id now/precise 'sha256
		code-challenge: enbase/url checksum/method code-verifier 'sha256 64
		trim/with code-challenge #"="
	]
	;-- 1. Have your application request authorization; the user logs in and authorizes access
	; build request url
	url: rejoin [
		https://accounts.spotify.com/authorize?
		"response_type=code&show_dialog=false"
		"&client_id="    ctx/client-id
		"&scope="        ctx/scope
		"&redirect_uri=" redirect-uri
	]
	; if client-secret was not specified, create challenge for PKCE extension
	if code-challenge [
		append url reduce [
			"&state=" state: form random 99999999999
			"&code_challenge_method=S256&code_challenge=" code-challenge
		]
	]
	; and open the url in user's default browser
	browse url

	; Result from the server is returned as a redirect, so let's start simple server
	; listening on specified port (limited to accept only local requests, as the redirect is
	; going from the browser actually.. it automaticaly close itself once data are received
	result: system/modules/httpd/http-server/config/actor ctx/port-id [
		root:       #[false] ; we are not serving any content!
		keep-alive: #[false]
	] [
		
		;- Server's actor functions

		On-Accept: func [info [object!]][
			; allow only connections from localhost
			; TRUE = accepted, FALSE = refuse
			find [ 127.0.0.1 ] info/remote-ip 
		]
		On-Header: func [ctx [object!]][
			? ctx/inp/target/file
			switch/default ctx/inp/target/file [
				%spotify-callback/ [
					ctx/out/status: 200
					ctx/out/content: ajoin [
						"<h1>OAuth2 Spotify Callback</h1>"
						"<br/>Request header:<pre>" mold ctx/inp/header </pre>
						"<br/>Values:<pre>" mold ctx/inp/target/values </pre>
						"<h2>You can close this window and return back to Rebol</h2>"
					]
					ctx/done?: ctx/inp/target/values
				]
			][
				ctx/out/status: 405
			]
		]
	]

	?? result

	; validate result from first step
	if any [
		not block? result
		none? result/code
		state <> result/state
	][
		print {*** Unexpected result from Spotify authorization!}
		return none
	]

	;-- 2. Have your application request refresh and access tokens; Spotify returns access and refresh tokens

	try/except [
		time: now
		ctx/token: load-json write https://accounts.spotify.com/api/token probe compose [
			POST [
				Content-Type: "application/x-www-form-urlencoded"
			] ( rejoin [
				"grant_type=authorization_code"
				"&code="          result/code
				"&scope="         any [result/scope ""]
				"&redirect_uri="  :redirect-uri
				"&client_id="     ctx/client-id
				either code-verifier [
					; PKCE version
					join "&code_verifier=" code-verifier
				][	; original version
					join "&client_secret=" ctx/client-secret
				]
			])
		]
		ctx/token/expires_in: time + (to time! ctx/token/expires_in)
	][
		print "*** Failed to receive Spotify token!"
		probe system/state/last-error
		return none
	]
	; return Spotify context 
	ctx
]

refresh: function[
	ctx [object!]
][
	ctx/token: load-json write https://accounts.spotify.com/api/token compose [
		POST [
			Content-Type: "application/x-www-form-urlencoded"
		]( rejoin [
			"grant_type=refresh_token"
			"&refresh_token=" ctx/token/refresh_token
			"&client_id="     ctx/client-id
			either ctx/client-secret [
				join "&client_secret=" ctx/client-secret
			][]
		])
	]
]

request: func [
	ctx    [object!]
	method [word!]
	what   [any-string!]
	data   [any-type!]
	/local header
][
	try/except [
		if now >= ctx/token/expires_in [ refresh ctx ]
		header: compose [
			Authorization: (join "Bearer " ctx/token/access_token)
		] 
		if map? data [
			append header [Content-Type: "application/json"]
			data: to-json data
		]
		write join https://api.spotify.com/v1/ what reduce [
			method header any [data ""]
		]
	][
		print ["*** Spotify" method "of" mold what "failed!"]
		none
	]
]
