Rebol [
	Title:   "Spotify"
	Purpose: "Spotify Web API access"
	Version: 0.2.1
	Author:  @Oldes
	Date:    20-Jul-2023
	File:    %spotify.reb
	Home:    https://github.com/oldes/Rebol-Spotify
	Name:    'spotify
	Type:    'module
	Require: 'httpd
	Note: {
		Useful info:
		https://developer.spotify.com/documentation/general/guides/authorization-guide/
		https://aaronparecki.com/oauth-2-simplified/
	}
	Needs: 3.11.0 ;; using try/with instead of deprecated try/except
]

default-scopes: [
;@@ In real app not all scopes may be required!
;@@ https://developer.spotify.com/documentation/general/guides/scopes/
	;- Images                     
	;@ugc-image-upload             ; Write access to user-provided images.
	;- Spotify Connect            
	@user-read-playback-state     ; Read access to a user’s player state.
	@user-modify-playback-state   ; Write access to a user’s playback state
	@user-read-currently-playing  ; Read access to a user’s currently playing content.
	;- Playback                   
	@streaming                    ; Control playback of a Spotify track. (Web Playback SDK and Premium only)
	@app-remote-control           ; Remote control playback of Spotify. (iOS and Android SDK only)
	;- Users                      
	@user-read-email              ; Read access to user’s email address.
	@user-read-private            ; Read access to user’s subscription details (type of user account).
	;- Playlists                  
	@playlist-read-collaborative  ; Include collaborative playlists when requesting a user's playlists.
	@playlist-modify-public       ; Write access to a user's public playlists.
	@playlist-read-private        ; Read access to user's private playlists.
	@playlist-modify-private      ; Write access to a user's private playlists.
	;- Library                    
	@user-library-modify          ; Write/delete access to a user's "Your Music" library.
	@user-library-read            ; Read access to a user's "Your Music" library.
	;- Listening History          
	@user-top-read                ; Read access to a user's top artists and tracks.
	@user-read-playback-position  ; Read access to a user’s playback position in a content.
	@user-read-recently-played    ; Read access to a user’s recently played tracks.
	;- Follow                     
	@user-follow-read             ; Read access to the list of artists and other users that the user follows.
	@user-follow-modify           ; Write/delete access to the list of artists and other users that the user follows.
]

config: function[
	"Resolve Spotify API config state from user's persistent data storage"
][
	if map? ctx: attempt [user's spotify-api][return ctx]
	unless system/user/name [
		name: ask      "User: "
		pass: ask/hide "Password: "
		;; when user does not exists, create a new one!
		set-user/n/p :name :pass
	]
	;; try again with the user
	if map? ctx: attempt [user's spotify-api][return ctx]
	;; setup new config...
	sys/log/error 'spotify "Client is not configured yet!"
	ctx: make map! 4

	unless all [
		ctx/client-id:     ask/hide "Client ID: "
		ctx/client-secret: ask/hide "Client Secret: "
	][	;; when user hits ESC...
		sys/log/error 'spotify "Missing credentials!"
		return #()
	]
	if empty? ctx/client-secret [ctx/client-secret: none] ;optional

	ctx/scopes: :default-scopes
	store-config ctx
]

add-scope: function[scope [ref!]][
	ctx: config
	unless find ctx/scopes scope [ append ctx/scopes scope ]
	remove/key ctx 'token ;; when scopes changes, token must be reauthorized
	store-config ctx
]

store-config: function[
	"Save the current config state in user's persistent data storage"
	config [map!]
][
	lib/put system/user/data 'spotify-api config 
	if port? system/user/data [
		update system/user/data
	]
	config
]

drop-token: function[
	"Used to force authentication"
][
	sys/log/debug 'SPOTIFY "Releasing a user's token"
	ctx: user's spotify-api
	unless map? ctx [exit]
	remove/key ctx 'token
	store-config ctx
]


authorize: function [
	"OAuth2 Spotify authorization used to get the main context"
	ctx [block! map!] "Data used for initialization (at least client-id is needed)"
][
	sys/log/info 'SPOTIFY "OAuth2 Spotify API authorization."
	import httpd

	if block? ctx [ctx: make map! ctx]

	;; Spotify needs at least client-id for the authorization
	unless client-id: ctx/client-id [ 
		sys/log/error 'SPOTIFY "`client-id` is needed to authorize with Spotify API"
		return none
	]
	unless string? client-id [ client-id: form client-id ]

	unless string? scopes: any [ctx/scopes default-scopes] [ scopes: form scopes ]
	parse scopes [any [change some #[bitset! #{0064000080}] #"+" | skip]] ; url-encode spaces in scopes

	unless integer? ctx/port [ ctx/port: 8989 ]
	;@@ The value of redirect_uri here must exactly match one of the values
	;@@ you entered when you registered your application, including upper 
	;@@ or lowercase, terminating slashes, and such.
	redirect-uri: any [
		ctx/redirect-uri
		ajoin ["http://localhost:" ctx/port %/spotify-callback/]
	]

	;-- 1. Have your application request authorization; the user logs in and authorizes access
	; build request url
	url: rejoin [
		https://accounts.spotify.com/authorize?
		"response_type=code&show_dialog=false"
		"&client_id="    client-id
		"&scope="        scopes
		"&redirect_uri=" redirect-uri
	]
	unless ctx/client-secret [
		;; if client-secret was not specified, create challenge for PKCE extension
		code-verifier: form random/secure checksum join client-id now/precise 'sha256
		code-challenge: enbase/url checksum code-verifier 'sha256 64
		trim/with code-challenge #"="

		append url reduce [
			"&state=" state: form random 99999999999
			"&code_challenge_method=S256&code_challenge=" code-challenge
		]
	]

	; open the url in user's default browser
	browse url

	; Result from the server is returned as a redirect, so let's start simple server
	; listening on specified port (limited to accept only local requests, as the redirect is
	; going from the browser actually.. it automaticaly close itself once data are received
	result: serve-http [
		port: ctx/port
		actor: [
			On-Accept: func [info [object!]][
				; allow only connections from localhost
				; TRUE = accepted, FALSE = refuse
				find [ 127.0.0.1 ] info/remote-ip 
			]
			On-Header: func [ctx [object!]][
				either ctx/inp/target/file == %/spotify-callback/ [
					ctx/out/status: 200
					ctx/out/content: ajoin [
						"<h1>OAuth2 Spotify Callback</h1>"
						"<br/>Request header:<pre>" mold ctx/inp/header </pre>
						"<br/>Values:<pre>" mold ctx/inp/target/values </pre>
						"<h2>You can close this window and return back to Rebol</h2>"
					]
					ctx/done?: ctx/inp/target/values
				][
					ctx/out/status: 405
					ctx/done?: true
				]
			]
		]
	]

	;?? result

	; validate result from first step
	if any [
		not block? result
		none? result/code
		state <> result/state
	][
		print {*** Unexpected result from Spotify authorization!}
		return none
	]

	;-- 2. Request refresh and access tokens using the result code
	try/with [
		time: now
		result: write/all https://accounts.spotify.com/api/token compose [
			POST [
				Content-Type: "application/x-www-form-urlencoded"
			] ( rejoin [
				"grant_type=authorization_code"
				"&code="          result/code
				"&scope="         ;enhex :scopes ;; required here?
				"&redirect_uri="  redirect-uri
				"&client_id="     ctx/client-id
				either code-verifier [
					; PKCE version
					join "&code_verifier=" code-verifier
				][	; original version
					join "&client_secret=" ctx/client-secret
				]
			])
		]
		either result/1 == 200 [
			ctx/token: load-json result/3
			ctx/token/expires_in: time + (to time! ctx/token/expires_in)
		][
			result: load-json result/3
			sys/log/error 'SPOTIFY "Failed to receive access token!"
			if result/error [
				sys/log/error 'SPOTIFY [result/error_description "-" result/error]
			]
			return none
		]
	][
		sys/log/error 'SPOTIFY "Failed to receive access token!"
		sys/log/error 'SPOTIFY system/state/last-error
		return none
	]

	; return Spotify context 
	sys/log/debug 'SPOTIFY ["access_token: " copy/part ctx/token/access_token 15 "..."]
	store-config ctx
]

refresh: function[
	ctx [map!]
][
	sys/log/info 'SPOTIFY "Refreshing Spotify API token."
	if any [
		none? ctx/token/refresh_token
		none? ctx/client-id
	][
		sys/log/error 'SPOTIFY "Not sufficient info to refresh a token!"
		drop-token
		return authorize ctx
	]
	result: write/all https://accounts.spotify.com/api/token compose [
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
	data: attempt [load-json result/3]
	if result/1 >= 400 [
		sys/log/error 'SPOTIFY "Failed to refresh access token!"
		if data/error_description [	sys/log/error 'SPOTIFY data/error_description ]
		return none
	]
	try/with [
		data/expires_in: now + (to time! data/expires_in)
		ctx/token: data
		store-config ctx
	] :print
]

request: func [
	method [word!]
	what   [any-string!]
	data   [any-type!]
	/local ctx header result
][
	header: clear #()

	try/with [
		ctx: config
		unless ctx/token [ctx: authorize ctx] ;; resolve the token
		unless ctx/token [return none  ] ;; exit if still not present
		if now >= ctx/token/expires_in [ refresh ctx ]
		header/Authorization: join "Bearer " ctx/token/access_token
		header/Accept: "application/json"
		if map? data [
			data: to-json data
			header/Content-Type: "application/json"
		]
		sys/log/info 'SPOTIFY [method as-green what]

		result: write/all join https://api.spotify.com/v1/ what reduce [
			method header any [data ""]
		]

		data: attempt [load-json result/3]
		either result/1 >= 400 [
			sys/log/error 'SPOTIFY [method as-red what]
			if all [map? data data/error] [
				if data/error_description [sys/log/error 'SPOTIFY data/error_description]
				sys/log/error 'SPOTIFY [data/error]
			]
			none
		][	data ]
	][
		sys/log/error 'SPOTIFY system/state/last-error
		none
	]
]

GET:  func [what [any-string!]           ][request 'GET    what none]
PUT:  func [what [any-string!] /with data][request 'PUT    what data]
DEL:  func [what [any-string!]           ][request 'DELETE what none]
POST: func [what [any-string!]       data][request 'POST   what data]

DELETE: :DEL