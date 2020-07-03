Rebol [
	Title: "Test Spotify module"
	Date: 02-Jul-2020
	Author: "Oldes"
	File: %test-spotify.r3
	Version: 0.1.1
	Require: 3.1.2
	Note: {
		Spotify API is still in Beta, and some bugs still happens...
		like this one: https://github.com/spotify/web-api/issues/1594
	}
]

do %spotify.reb

system/options/log/http: 1 ; for verbose output
spotify: system/modules/spotify/authorize [
	client-id:     {8a01eb3b22ee4ba3b310e5e4fe10fd68}
	; optional client-secret may be used for classic "Authorization Code Flow"
	; if not used, the authentication will use PKCE extension, which is recommended
	;client-secret: {00000000000000000000000000000000} 
	scope: [
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

unless spotify [
	print "*** Access to Spotify API failed!"
	halt
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
	track: load-json track
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

print {^/Get Current User's Profile:}
print spotify/get %me

print {^/Get Information About The User's Current Playback:}
print spotify/get %me/player

print {^/Get a User's Available Devices:}
print spotify/get %me/player/devices

print {^/Get the User's Currently Playing Track:}
print spotify/get %me/player/currently-playing
;@@ https://developer.spotify.com/documentation/web-api/reference/player/get-the-users-currently-playing-track/

print {^/Get a List of Current User's Playlists:}
print spotify/get %me/playlists
; optionaly for example with %me/playlists?limit=1&offset=2 to get just second playlist
;@@ https://developer.spotify.com/documentation/web-api/reference/playlists/get-a-list-of-current-users-playlists/

print {^/Get a List of a User's Playlists:}
print spotify/get %users/01vv226r3fsej509cp2yuozb8/playlists
;@@ https://developer.spotify.com/documentation/web-api/reference/playlists/get-list-users-playlists/

print {^/Pause/Play/Seek a User's Playback:}
print spotify/put %me/player/pause
print spotify/put %me/player/play
print spotify/put %me/player/seek?position_ms=0
;@@ https://github.com/spotify/web-api/issues/1594

print {^/Search for an Item:}
print spotify/get %search?type=artist&q=ixieindamix
;@@ https://developer.spotify.com/documentation/web-api/reference/search/search/

print {^/Check if Current User Follows Artists or Users:}
print spotify/get %me/following/contains?type=artist&ids=2IpvlHMrM6rBvDY7dAAg7D,74ASZWbe4lXaubB36ztrGX
print {^/Get User's Followed Artists:}
print spotify/get %me/following?type=artist&limit=2
print {^/Start following Artists:}
print spotify/put %me/following?type=artist&ids=74ASZWbe4lXaubB36ztrGX
print {^/Stop following Artists:}
print spotify/del %me/following?type=artist&ids=74ASZWbe4lXaubB36ztrGX
;@@ https://developer.spotify.com/documentation/web-api/reference/follow/


print {^/Get Audio Features for a Track:}
print spotify/get %audio-features/0uq0ibSGT4AwiVLLZGs9Qm
;@@ https://developer.spotify.com/documentation/web-api/reference/tracks/get-audio-features/

print {^/Save Tracks for Current User:}
print spotify/put %me/tracks?ids=0uq0ibSGT4AwiVLLZGs9Qm
;@@ https://developer.spotify.com/documentation/web-api/reference/library/save-tracks-user/

halt
