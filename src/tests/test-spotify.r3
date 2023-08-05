Rebol [
	Title:  "Test Spotify module"
	Version: 0.2.1
	Author:  @Oldes
	Date:    20-Jul-2023
	File:    %test-spotify.r3
	Needs:   3.11.0
]

system/options/quiet: false   ;; allow Spotify traces
system/options/log/spotify: 4 ;; verbose
system/options/log/http: 0    ;; no HTTP traces

spotify: import spotify

test-all?: false ;; I don't want CI tests to stop my real playback

errors: 0 ;; error counter (process return value)

do-test: func[title code][
	print-horizontal-line
	print as-yellow title
	probe code
	try/with code [
		print system/state/last-error
		++ errors
	]
]


[	;@@ By default, the authorize is called when needed, but it can be forced using this code
	remove/key system/user/data 'spotify-api
	spotify/authorize [
		client-id:      {8a01eb3b22ee4ba3b310e5e4fe10fd68}
		; optional client-secret may be used for classic "Authorization Code Flow"
		; if not used, the authentication will use PKCE extension.
		; But!
		; A refresh token that has been obtained through PKCE can be exchanged for
		; an access token only once, after which it becomes invalid!
		;client-secret: {00000000000000000000000000000000}
		; Both values can be found for an app in: https://developer.spotify.com/dashboard
		scopes: [
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
	]
]

spot?: function [
	"Prints information what is currently playing on user's Spotify"
][
	log: system/options/log/http
	system/options/log/http: 0 
	track: spotify/get %me/player/currently-playing
	unless track [
		print "No device!"
		exit
	]
	print ["Song:  " track/item/name]
	print ["Album: " track/item/album/name]
	foreach artist track/item/artists [
		print ["Artist:" artist/name ]
	]
	print ["ID:    " track/item/id] 
	print ["Pop:   " track/item/popularity]
	system/options/log/http: log
	()
]

do-test {Get Current User's Profile} [
	probe spotify/get %me
]

do-test {Get Information About The User's Current Playback} [
	probe spotify/get %me/player
]

do-test {Get a User's Available Devices} [
	probe spotify/get %me/player/devices
]

;@@ https://developer.spotify.com/documentation/web-api/reference/player/get-the-users-currently-playing-track/
do-test {Get the User's Currently Playing Track} [
	probe spotify/get %me/player/currently-playing
]

;@@ https://developer.spotify.com/documentation/web-api/reference/playlists/get-a-list-of-current-users-playlists/
do-test {Get a List of Current User's Playlists} [
	probe spotify/get %me/playlists
	; optionaly for example with %me/playlists?limit=1&offset=2 to get just second playlist
]

;@@ https://developer.spotify.com/documentation/web-api/reference/playlists/get-list-users-playlists/
do-test {Get a List of a User's Playlists} [
	probe spotify/get %users/01vv226r3fsej509cp2yuozb8/playlists
]

;@@ https://developer.spotify.com/documentation/web-api/reference/search/search/
do-test {Search for an Item} [
	probe spotify/get %search?type=artist&q=ixieindamix
]

;@@ https://developer.spotify.com/documentation/web-api/reference/follow/
do-test {Get User's Followed Artists} [
	probe spotify/get %me/following?type=artist&limit=2
]
do-test {Check if Current User Follows Artists or Users} [
	probe spotify/get %me/following/contains?type=artist&ids=2IpvlHMrM6rBvDY7dAAg7D,74ASZWbe4lXaubB36ztrGX
]
do-test {Start following Artists} [
	probe spotify/put %me/following?type=artist&ids=74ASZWbe4lXaubB36ztrGX
	probe spotify/get %me/following/contains?type=artist&ids=74ASZWbe4lXaubB36ztrGX
]
do-test {Stop following Artists} [
	probe spotify/del %me/following?type=artist&ids=74ASZWbe4lXaubB36ztrGX
	probe spotify/get %me/following/contains?type=artist&ids=74ASZWbe4lXaubB36ztrGX
]

;@@ https://developer.spotify.com/documentation/web-api/reference/tracks/get-audio-features/
do-test {Get Audio Features for a Track} [
	probe spotify/get %audio-features/0uq0ibSGT4AwiVLLZGs9Qm
]

do-test {Tracks} [
	;@@ https://developer.spotify.com/documentation/web-api/reference/get-track
	print "Get the track"
	probe spotify/get %me/tracks/0uq0ibSGT4AwiVLLZGs9Qm

	;@@ https://developer.spotify.com/documentation/web-api/reference/get-audio-features
	print "Get Track's Audio Features"
	probe spotify/get %audio-features/0uq0ibSGT4AwiVLLZGs9Qm
	;probe spotify/get %audio-features?ids=3U7TesefC7piVH8SUqQz9L,0uq0ibSGT4AwiVLLZGs9Qm

	;@@ https://developer.spotify.com/documentation/web-api/reference/get-audio-analysis
	;; Get a low-level audio analysis for a track in the Spotify catalog. The audio analysis
	;; describes the track’s structure and musical content, including rhythm, pitch, and timbre.
	print "Get Track's Audio Analysis"
	probe spotify/get %audio-analysis/0uq0ibSGT4AwiVLLZGs9Qm

	;@@ https://developer.spotify.com/documentation/web-api/reference/check-users-saved-tracks
	print "Check User's Saved Tracks"
	probe spotify/get %me/tracks/contains?ids=0uq0ibSGT4AwiVLLZGs9Qm
	
	;@@ https://developer.spotify.com/documentation/web-api/reference/remove-tracks-user
	print "Remove one or more tracks to the current user's 'Your Music' library. Maximum: 50 IDs."
	probe spotify/del %me/tracks?ids=0uq0ibSGT4AwiVLLZGs9Qm

	;@@ https://developer.spotify.com/documentation/web-api/reference/save-tracks-user
	print "Save one or more tracks to the current user's 'Your Music' library. Maximum: 50 IDs."
	probe spotify/put %me/tracks?ids=0uq0ibSGT4AwiVLLZGs9Qm
]

if test-all? [
	do-test {Pause/Play/Seek a User's Playback} [
		;@@ https://github.com/spotify/web-api/issues/1594
		probe spotify/put %me/player/pause
		probe spotify/put %me/player/play
		probe spotify/put %me/player/seek?position_ms=0
	]
]

quit/return errors
