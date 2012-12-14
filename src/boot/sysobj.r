REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "System object"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0.
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Purpose: {
		Defines the system object. This is a special block that is evaluted
		such that its words do not get put into the current context. 
	}
	Note: "Remove older/unused fields before beta release"
]

product:  'core

; Next three fields are updated during build:
version:  0.0.0
build:    1
platform: none

license: {                                 Apache License
                           Version 2.0, January 2004
                        http://www.apache.org/licenses/

   TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

   1. Definitions.

      "License" shall mean the terms and conditions for use, reproduction,
      and distribution as defined by Sections 1 through 9 of this document.

      "Licensor" shall mean the copyright owner or entity authorized by
      the copyright owner that is granting the License.

      "Legal Entity" shall mean the union of the acting entity and all
      other entities that control, are controlled by, or are under common
      control with that entity. For the purposes of this definition,
      "control" means (i) the power, direct or indirect, to cause the
      direction or management of such entity, whether by contract or
      otherwise, or (ii) ownership of fifty percent (50%) or more of the
      outstanding shares, or (iii) beneficial ownership of such entity.

      "You" (or "Your") shall mean an individual or Legal Entity
      exercising permissions granted by this License.

      "Source" form shall mean the preferred form for making modifications,
      including but not limited to software source code, documentation
      source, and configuration files.

      "Object" form shall mean any form resulting from mechanical
      transformation or translation of a Source form, including but
      not limited to compiled object code, generated documentation,
      and conversions to other media types.

      "Work" shall mean the work of authorship, whether in Source or
      Object form, made available under the License, as indicated by a
      copyright notice that is included in or attached to the work
      (an example is provided in the Appendix below).

      "Derivative Works" shall mean any work, whether in Source or Object
      form, that is based on (or derived from) the Work and for which the
      editorial revisions, annotations, elaborations, or other modifications
      represent, as a whole, an original work of authorship. For the purposes
      of this License, Derivative Works shall not include works that remain
      separable from, or merely link (or bind by name) to the interfaces of,
      the Work and Derivative Works thereof.

      "Contribution" shall mean any work of authorship, including
      the original version of the Work and any modifications or additions
      to that Work or Derivative Works thereof, that is intentionally
      submitted to Licensor for inclusion in the Work by the copyright owner
      or by an individual or Legal Entity authorized to submit on behalf of
      the copyright owner. For the purposes of this definition, "submitted"
      means any form of electronic, verbal, or written communication sent
      to the Licensor or its representatives, including but not limited to
      communication on electronic mailing lists, source code control systems,
      and issue tracking systems that are managed by, or on behalf of, the
      Licensor for the purpose of discussing and improving the Work, but
      excluding communication that is conspicuously marked or otherwise
      designated in writing by the copyright owner as "Not a Contribution."

      "Contributor" shall mean Licensor and any individual or Legal Entity
      on behalf of whom a Contribution has been received by Licensor and
      subsequently incorporated within the Work.

   2. Grant of Copyright License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      copyright license to reproduce, prepare Derivative Works of,
      publicly display, publicly perform, sublicense, and distribute the
      Work and such Derivative Works in Source or Object form.

   3. Grant of Patent License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      (except as stated in this section) patent license to make, have made,
      use, offer to sell, sell, import, and otherwise transfer the Work,
      where such license applies only to those patent claims licensable
      by such Contributor that are necessarily infringed by their
      Contribution(s) alone or by combination of their Contribution(s)
      with the Work to which such Contribution(s) was submitted. If You
      institute patent litigation against any entity (including a
      cross-claim or counterclaim in a lawsuit) alleging that the Work
      or a Contribution incorporated within the Work constitutes direct
      or contributory patent infringement, then any patent licenses
      granted to You under this License for that Work shall terminate
      as of the date such litigation is filed.

   4. Redistribution. You may reproduce and distribute copies of the
      Work or Derivative Works thereof in any medium, with or without
      modifications, and in Source or Object form, provided that You
      meet the following conditions:

      (a) You must give any other recipients of the Work or
          Derivative Works a copy of this License; and

      (b) You must cause any modified files to carry prominent notices
          stating that You changed the files; and

      (c) You must retain, in the Source form of any Derivative Works
          that You distribute, all copyright, patent, trademark, and
          attribution notices from the Source form of the Work,
          excluding those notices that do not pertain to any part of
          the Derivative Works; and

      (d) If the Work includes a "NOTICE" text file as part of its
          distribution, then any Derivative Works that You distribute must
          include a readable copy of the attribution notices contained
          within such NOTICE file, excluding those notices that do not
          pertain to any part of the Derivative Works, in at least one
          of the following places: within a NOTICE text file distributed
          as part of the Derivative Works; within the Source form or
          documentation, if provided along with the Derivative Works; or,
          within a display generated by the Derivative Works, if and
          wherever such third-party notices normally appear. The contents
          of the NOTICE file are for informational purposes only and
          do not modify the License. You may add Your own attribution
          notices within Derivative Works that You distribute, alongside
          or as an addendum to the NOTICE text from the Work, provided
          that such additional attribution notices cannot be construed
          as modifying the License.

      You may add Your own copyright statement to Your modifications and
      may provide additional or different license terms and conditions
      for use, reproduction, or distribution of Your modifications, or
      for any such Derivative Works as a whole, provided Your use,
      reproduction, and distribution of the Work otherwise complies with
      the conditions stated in this License.

   5. Submission of Contributions. Unless You explicitly state otherwise,
      any Contribution intentionally submitted for inclusion in the Work
      by You to the Licensor shall be under the terms and conditions of
      this License, without any additional terms or conditions.
      Notwithstanding the above, nothing herein shall supersede or modify
      the terms of any separate license agreement you may have executed
      with Licensor regarding such Contributions.

   6. Trademarks. This License does not grant permission to use the trade
      names, trademarks, service marks, or product names of the Licensor,
      except as required for reasonable and customary use in describing the
      origin of the Work and reproducing the content of the NOTICE file.

   7. Disclaimer of Warranty. Unless required by applicable law or
      agreed to in writing, Licensor provides the Work (and each
      Contributor provides its Contributions) on an "AS IS" BASIS,
      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
      implied, including, without limitation, any warranties or conditions
      of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
      PARTICULAR PURPOSE. You are solely responsible for determining the
      appropriateness of using or redistributing the Work and assume any
      risks associated with Your exercise of permissions under this License.

   8. Limitation of Liability. In no event and under no legal theory,
      whether in tort (including negligence), contract, or otherwise,
      unless required by applicable law (such as deliberate and grossly
      negligent acts) or agreed to in writing, shall any Contributor be
      liable to You for damages, including any direct, indirect, special,
      incidental, or consequential damages of any character arising as a
      result of this License or out of the use or inability to use the
      Work (including but not limited to damages for loss of goodwill,
      work stoppage, computer failure or malfunction, or any and all
      other commercial damages or losses), even if such Contributor
      has been advised of the possibility of such damages.

   9. Accepting Warranty or Additional Liability. While redistributing
      the Work or Derivative Works thereof, You may choose to offer,
      and charge a fee for, acceptance of support, warranty, indemnity,
      or other liability obligations and/or rights consistent with this
      License. However, in accepting such obligations, You may act only
      on Your own behalf and on Your sole responsibility, not on behalf
      of any other Contributor, and only if You agree to indemnify,
      defend, and hold each Contributor harmless for any liability
      incurred by, or claims asserted against, such Contributor by reason
      of your accepting any such warranty or additional liability.

   END OF TERMS AND CONDITIONS

   APPENDIX: How to apply the Apache License to your work.

      To apply the Apache License to your work, attach the following
      boilerplate notice, with the fields enclosed by brackets "[]"
      replaced with your own identifying information. (Don't include
      the brackets!)  The text should be enclosed in the appropriate
      comment syntax for the file format. We also recommend that a
      file or class name and description of purpose be included on the
      same "printed page" as the copyright notice for easier
      identification within third-party archives.

   Copyright [yyyy] [name of copyright owner]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

}

catalog: context [
	; Static (non-changing) values, blocks, objects
	datatypes: []
	actions: none
	natives: none
	errors: none
	reflectors: [spec body words values types title]
	; Official list of system/options/flags that can appear.
	; Must match host reb-args.h enum!
	boot-flags: [
		script args do import version debug secure
		help vers quiet verbose no-boot
		secure-min secure-max trace halt cgi boot-level
	]
]

contexts: context [
	root:
	sys:
	lib:
	user:
		none
]

state: context [
	; Mutable system state variables
	note: "contains protected hidden fields"
	policies: context [ ; Security policies
		file:    ; file access
		net:     ; network access
		eval:    ; evaluation limit
		memory:  ; memory limit
		secure:  ; secure changes
		protect: ; protect function
		debug:   ; debugging features
		envr:    ; read/write
		call:    ; execute only
		browse:  ; execute only
			0.0.0
		extension: 2.2.2 ; execute only
	]
	last-error: none ; used by WHY?
]

modules: []

codecs: context []

dialects: context [
	secure:
	draw:
	effect:
	text:
	rebcode:
		none
]

schemes: context []

ports: context [
	wait-list: []	; List of ports to add to 'wait
	input:          ; Port for user input.
	output:         ; Port for user output
	echo:           ; Port for echoing output
	system:         ; Port for system events
	callback: none	; Port for callback events
;	serial: none	; serial device name block
]

locale: context [
	language: 	; Human language locale
	language*:
	locale:
	locale*: none
	months: [
		"January" "February" "March" "April" "May" "June"
		"July" "August" "September" "October" "November" "December"
	]
	days: [
		"Monday" "Tuesday" "Wednesday" "Thursday" "Friday" "Saturday" "Sunday"
	]
]

options: context [  ; Options supplied to REBOL during startup
	boot:           ; The path to the executable
	home:           ; Path of home directory
	path:           ; Where script was started or the startup dir
		none

	flags:          ; Boot flag bits (see system/catalog/boot-flags)
	script:         ; Filename of script to evaluate
	args:           ; Command line arguments passed to script
	do-arg:         ; Set to a block if --do was specified
	import:         ; imported modules
	debug:          ; debug flags
	secure:         ; security policy
	version:        ; script version needed
	boot-level:     ; how far to boot up
		none

	quiet: false    ; do not show startup info (compatibility)

	binary-base: 16    ; Default base for FORMed binary values (64, 16, 2)
	decimal-digits: 15 ; Max number of decimal digits to print.
	module-paths: [%./]
	default-suffix: %.reb ; Used by IMPORT if no suffix is provided
	file-types: []
	result-types: none
]

script: context [
	title:          ; Title string of script
	header:         ; Script header as evaluated
	parent:         ; Script that loaded the current one
	path:           ; Location of the script being evaluated
	args:           ; args passed to script
		none
]

standard: context [

	error: context [ ; Template used for all errors:
		code: 0
		type: 'user
		id:   'message
		arg1:
		arg2:
		arg3:
		near:
		where:
			none
	]
 
	script: context [
		title:
		header:
		parent: 
		path:
		args:
			none
	]

	header: context [
		title: {Untitled}
		name:
		type:
		version:
		date:
		file:
		author:
		needs:
		options:
		checksum:
;		compress:
;		exports:
;		content:
			none
	]

	scheme: context [
		name:		; word of http, ftp, sound, etc.
		title:		; user-friendly title for the scheme
		spec:		; custom spec for scheme (if needed)
		info:		; prototype info object returned from query
;		kind:		; network, file, driver
;		type:		; bytes, integers, objects, values, block
		actor:		; standard action handler for scheme port functions
		awake:		; standard awake handler for this scheme's ports
			none
	]

	port: context [ ; Port specification object
		spec:		; published specification of the port
		scheme:		; scheme object used for this port
		actor:		; port action handler (script driven)
		awake:		; port awake function (event driven)
		state:		; internal state values (private)
		data:		; data buffer (usually binary or block)
		locals:		; user-defined storage of local data
;		stats:		; stats on operation (optional)
			none
	]

	port-spec-head: context [
		title:		; user-friendly title for port
		scheme:		; reference to scheme that defines this port
		ref:		; reference path or url (for errors)
		path:		; used for files
		   none 	; (extended here)
	]

	port-spec-net: make port-spec-head [
		host: none
		port-id: 80
			none
	]
	
	file-info: context [
		name:
		size:
		date:
		type:
			none
	]

	net-info: context [
		local-ip:
		local-port:
		remote-ip:
		remote-port:
			none
	]

	extension: context [
		lib-base:	; handle to DLL
		lib-file:	; file name loaded
		lib-boot:	; module header and body
		command:	; command function
		cmd-index:	; command index counter
		words:		; symbol references
			none
	]

	stats: context [ ; port stats
		timer:		; timer (nanos)
		evals:		; evaluations
		eval-natives:
		eval-functions:
		series-made:
		series-freed:
		series-expanded:
		series-bytes:
		series-recycled:
		made-blocks:
		made-objects:
		recycles:
			none
	]

	type-spec: context [
		title:
		type:
			none
	]

	utype: none
	font: none	; mezz-graphics.h
	para: none	; mezz-graphics.h
]

view: context [
	screen-gob: none
	handler: none
	event-port: none
	metrics: context [
		screen-size: 0x0
		border-size: 0x0
		border-fixed: 0x0
		title-size: 0x0
		work-origin: 0x0
		work-size: 0x0
	]
	event-types: [
		; Event types. Order dependent for C and REBOL.
		; Due to fixed C constants, this list cannot be reordered after release!
		ignore			; ignore event (0)
		interrupt		; user interrupt
		device			; misc device request
		callback		; callback event
		custom			; custom events
		error
		init

		open
		close
		connect
		accept
		read
		write
		wrote
		lookup

		ready
		done
		time

		show
		hide
		offset
		resize
		active
		inactive 
		minimize
		maximize
		restore

		move
		down
		up
		alt-down 
		alt-up 
		aux-down 
		aux-up 
		key
		key-up ; Move above when version changes!!!

		scroll-line
		scroll-page

		drop-file
	]
	event-keys: [
		; Event types. Order dependent for C and REBOL.
		; Due to fixed C constants, this list cannot be reordered after release!
		page-up
		page-down
		end
		home
		left
		up
		right
		down
		insert
		delete
		f1
		f2
		f3
		f4
		f5
		f6
		f7
		f8
		f9
		f10
		f11
		f12
	]
]

;;stats: none

;user-license: context [
;	name:
;	email:
;	id:
;	message:
;		none
;]



; (returns value)

;		model:		; Network, File, Driver
;		type:		; bytes, integers, values
;		user:		; User data

;		host:
;		port-id:
;		user:
;		pass:
;		target:
;		path:
;		proxy:
;		access:
;		allow:
;		buffer-size:
;		limit:
;		handler:
;		status:
;		size:
;		date:
;		sub-port:
;		locals:
;		state:
;		timeout:
;		local-ip:
;		local-service:
;		remote-service:
;		last-remote-service:
;		direction:
;		key:
;		strength:
;		algorithm:
;		block-chaining:
;		init-vector:
;		padding:
;		async-modes:
;		remote-ip:
;		local-port:
;		remote-port:
;		backlog:
;		device:
;		speed:
;		data-bits:
;		parity:
;		stop-bits:
;			none
;		rts-cts: true
;		user-data:
;		awake:

;	port-flags: context [
;		direct:
;		pass-thru:
;		open-append:
;		open-new:
;			none
;	]

;	email: context [ ; Email header object
;		To:
;		CC:
;		BCC:
;		From:
;		Reply-To:
;		Date:
;		Subject:
;		Return-Path:
;		Organization:
;		Message-Id:
;		Comment:
;		X-REBOL:
;		MIME-Version:
;		Content-Type:
;		Content:
;			none
;	]

;user: context [
;	name:           ; User's name
;	email:          ; User's default email address
;	home:			; The HOME environment variable
;	words: none
;]

;network: context [
;	host: ""        ; Host name of the user's computer
;	host-address: 0.0.0.0 ; Host computer's TCP-IP address
;	trace: none
;]

;console: context [
;	hide-types: none ; types not to print
;	history:         ; Log of user inputs
;	keys: none       ; Keymap for special key
;	prompt:  {>> }   ; Specifies the prompt
;	result:  {== }   ; Specifies result
;	escape:  {(escape)} ; Indicates an escape
;	busy:    {|/-\}  ; Spinner for network progress
;	tab-size: 4      ; default tab size
;	break: true		 ; whether escape breaks or not
;]

;			decimal: #"."	; The character used as the decimal point in decimal and money vals
;			sig-digits: none	; Significant digits to use for decimals ; none for normal printing
;			date-sep: #"-"	; The character used as the date separator
;			date-month-num: false	; True if months are displayed as numbers; False for names
;			time-sep: #":"	; The character used as the time separator
;	cgi: context [ ; CGI environment variables
;		server-software:
;		server-name:
;		gateway-interface:
;		server-protocol:
;		server-port:
;		request-method:
;		path-info:
;		path-translated:
;		script-name:
;		query-string:
;		remote-host:
;		remote-addr:
;		auth-type:
;		remote-user:
;		remote-ident:
;		Content-Type:			; cap'd for email header
;		content-length: none
;		other-headers: []
;	]
;	browser-type: 0

;	trace:			; True if the --trace flag was specified
;	help: none      ; True if the --help flags was specified
;	halt: none		; halt after script

