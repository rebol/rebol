REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Error objects"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0.
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Purpose: {
		Specifies error categories and default error messages.
	}
]

Throw: [
	code: 0
	type: "throw error"
	break:              {no loop to break}
	return:             {return or exit not in function}
	throw:              [{no catch for throw:} :arg1]
	continue:           {no loop to continue}
	halt:               [{halted by user or script}]
	quit:               [{user script quit}]
]

Note: [
	code: 100
	type: "note"
	no-load:            [{cannot load: } :arg1]
	exited:             [{exit occurred}]
	deprecated:         {deprecated function not allowed}
]

Syntax: [
	code: 200
	type: "syntax error"
	invalid:            [{invalid} :arg1 {--} :arg2]
	missing:            [{missing} :arg2 {at} :arg1]
	no-header:          [{script is missing a REBOL header:} :arg1]
	bad-header:         [{script header is not valid:} :arg1]
	bad-checksum:       [{script checksum failed:} :arg1]
	malconstruct:       [{invalid construction spec:} :arg1]
	bad-char:           [{invalid character in:} :arg1]
	needs:              [{this script needs} :arg1 :arg2 {or better to run correctly}]
]

Script: [
	code: 300
	type: "script error"
	no-value:           [:arg1 {has no value}]
	need-value:         [:arg1 {needs a value}]
	not-defined:        [:arg1 {word is not bound to a context}]
	not-in-context:     [:arg1 {is not in the specified context}]

	no-arg:             [:arg1 {is missing its} :arg2 {argument}]
	expect-arg:         [:arg1 {does not allow} :arg3 {for its} :arg2 {argument}]
	expect-val:         [{expected} :arg1 {not} :arg2]
	expect-type:        [:arg1 :arg2 {field must be of type} :arg3]
	cannot-use:         [{cannot use} :arg1 {on} :arg2 {value}]

	invalid-arg:        [{invalid argument:} :arg1]
	invalid-type:       [:arg1 {type is not allowed here}]
	invalid-op:         [{invalid operator:} :arg1]
	no-op-arg:          [:arg1 {operator is missing an argument}]
	invalid-data:       [{data not in correct format:} :arg1]
	not-same-type:      {values must be of the same type}
	not-related:        [{incompatible argument for} :arg1 {of} :arg2]
	bad-func-def:       [{invalid function definition:} :arg1]
	bad-func-arg:       [{function argument} :arg1 {is not valid}] ; can be a number

	no-refine:          [:arg1 {has no refinement called} :arg2]
	bad-refines:        {incompatible or invalid refinements}
	bad-refine:         [{incompatible refinement:} :arg1]
	invalid-path:       [{cannot access} :arg2 {in path} :arg1]
	bad-path-type:      [{path} :arg1 {is not valid for} :arg2 {type}]
	bad-path-set:       [{cannot set} :arg2 {in path} :arg1]
	bad-field-set:      [{cannot set} :arg1 {field to} :arg2 {datatype}]
	dup-vars:           [{duplicate variable specified:} :arg1]

	past-end:           {out of range or past end}
	missing-arg:        {missing a required argument or refinement}
	out-of-range:       [{value out of range:} :arg1]
	too-short:          {content too short (or just whitespace)}
	too-long:           {content too long}
	invalid-chars:      {contains invalid characters}
	invalid-compare:    [{cannot compare} :arg1 {with} :arg2]
	assert-failed:      [{assertion failed for:} :arg1]
	wrong-type:         [{datatype assertion failed for:} :arg1]

	invalid-part:       [{invalid /part count:} :arg1]
	type-limit:         [:arg1 {overflow/underflow}]
	size-limit:         [{maximum limit reached:} :arg1]

	no-return:          {block did not return a value}
	block-lines:        {expected block of lines}
	throw-usage:        {invalid use of a thrown error value}

	locked-word:        [{protected variable - cannot modify:} :arg1]
	protected:          {protected value or series - cannot modify}
	hidden:             {not allowed - would expose or modify hidden values}
	self-protected:     {cannot set/unset self - it is protected}
	bad-bad:            [:arg1 {error:} :arg2]

	bad-make-arg:       [{cannot MAKE/TO} :arg1 {from:} :arg2]
	bad-decode:         {missing or unsupported encoding marker}
;	no-decode:          [{cannot decode} :arg1 {encoding}]
	already-used:       [{alias word is already in use:} :arg1]
	wrong-denom:        [:arg1 {not same denomination as} :arg2]
;   bad-convert:        [{invalid conversion value:} :arg1]
	bad-press:          [{invalid compressed data - problem:} :arg1]
	dialect:            [{incorrect} :arg1 {dialect usage at:} :arg2]
	bad-command:        {invalid command format (extension function)}

	parse-rule:         [{PARSE - invalid rule or usage of rule:} :arg1]
	parse-end:          [{PARSE - unexpected end of rule after:} :arg1]
	parse-variable:     [{PARSE - expected a variable, not:} :arg1]
	parse-command:      [{PARSE - command cannot be used as variable:} :arg1]
	parse-series:       [{PARSE - input must be a series:} :arg1]

;   bad-prompt:         [{Error executing prompt block}]
;   bad-port-action:    [{Cannot use} :arg1 {on this type port}]
;   face-error:         [{Invalid graphics face object}]
;   face-reused:        [{Face object reused (in more than one pane):} :arg1]
]

Math: [
	code: 400
	type: "math error"
	zero-divide:        {attempt to divide by zero}
	overflow:           {math or number overflow}
	positive:           {positive number required}
]

Access: [
	code: 500
	type: "access error"

	cannot-open:        [{cannot open:} :arg1 {reason:} :arg2]
	not-open:           [{port is not open:} :arg1]
	already-open:       [{port is already open:} :arg1]
;   already-closed:     [{port} :arg1 {already closed}]
	no-connect:         [{cannot connect:} :arg1 {reason:} :arg2]
	not-connected:      [{port is not connected:} :arg1]
;   socket-open:        [{error opening socket:} :arg1]
	no-script:          [{script not found:} :arg1]

	no-scheme-name:     [{new scheme must have a name:} :arg1]
	no-scheme:          [{missing port scheme:} :arg1]

	invalid-spec:       [{invalid spec or options:} :arg1]
	invalid-port:       [{invalid port object (invalid field values)}]
	invalid-actor:      [{invalid port actor (must be native or object)}]
	invalid-port-arg:   [{invalid port argument:} arg1]
	no-port-action:     [{this port does not support:} :arg1]
	protocol:           [{protocol error:} :arg1]
	invalid-check:      [{invalid checksum (tampered file):} :arg1] 

	write-error:        [{write failed:} :arg1 {reason:} :arg2]
	read-error:         [{read failed:} :arg1 {reason:} :arg2]
	read-only:          [{read-only - write not allowed:} :arg1]
	no-buffer:          [{port has no data buffer:} :arg1]
	timeout:            [{port action timed out:} :arg1]

	no-create:          [{cannot create:} :arg1]
	no-delete:          [{cannot delete:} :arg1]
	no-rename:          [{cannot rename:} :arg1]
	bad-file-path:      [{bad file path:} :arg1]
	bad-file-mode:      [{bad file mode:} :arg1]
;   protocol:           [{protocol error} :arg1]

    security:           [{security violation:} :arg1 { (refer to SECURE function)}]
    security-level:     [{attempt to lower security to} :arg1]
    security-error:     [{invalid} :arg1 {security policy:} :arg2]

	no-codec:           [{cannot decode or encode (no codec):} :arg1]
	bad-media:          [{bad media data (corrupt image, sound, video)}]
;   would-block:        [{operation on port} :arg1 {would block}]
;   no-action:          [{this type of port does not support the} :arg1 {action}]
;   serial-timeout:     {serial port timeout}
	no-extension:       [{cannot open extension:} :arg1]
	bad-extension:      [{invalid extension format:} :arg1]
	extension-init:     [{extension cannot be initialized (check version):} :arg1]

	call-fail:          [{external process failed:} :arg1]

]

Command: [
	code: 600
	type: "command error"
;   fmt-too-short:      {Format string is too short}
;   fmt-no-struct-size: [{Missing size spec for struct at arg#} :arg1]
;   fmt-no-struct-align: [{Missing align spec for struct at arg#} :arg1]
;   fmt-bad-word:       [{Bad word in format string at arg#} :arg1]
;   fmt-type-mismatch:  [{Type mismatch in format string at arg#} :arg1]
;   fmt-size-mismatch:  [{Size mismatch in format string at arg#} :arg1]
;   dll-arg-count:      {Number of arguments exceeds 25}
;   empty-command:      {Command is empty}
;   db-not-open:        {Database is not open}
;   db-too-many:        {Too many open databases}
;   cant-free:          [{Cannot free} :arg1]
;   nothing-to-free:    {Nothing to free}
;   ssl-error:          [{SSL Error: } :arg1]
]

resv700: [
	code: 700
	type: "reserved"
]

User: [
	code: 800
	type: "user error"
	message: [:arg1]
]

Internal: [
	code: 900
	type: "internal error"
	bad-path:           [{bad path:} arg1]
	not-here:           [arg1 {not supported on your system}]
	no-memory:          {not enough memory}
	stack-overflow:     {stack overflow}
	globals-full:       {no more global variable space}
	max-natives:        {too many natives}
	bad-series:         {invalid series}
	limit-hit:          [{internal limit reached:} :arg1]
	bad-sys-func:       [{invalid or missing system function:} :arg1]
	feature-na:         {feature not available}
	not-done:           {reserved for future use (or not yet implemented)}
	invalid-error:      {error object or fields were not valid}
]
