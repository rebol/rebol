REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Standard source code header"
	Rights: {
		Copyright 2012 REBOL Technologies
		Copyright 2012-2025 Rebol Open Source Contributors
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Author: "Carl Sassenrath"
]

form-header: func [title [string!] file [file!] /gen by] [
	print-info ["== Header:" as-green title]
	by: either gen [
		rejoin [{**  AUTO-GENERATED FILE - Do not modify. (From: } by {)^/**^/}]
	][""]

	rejoin [
{/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2025 Rebol Open Source Contributors
**  REBOL is a trademark of REBOL Technologies
**  Licensed under the Apache License, Version 2.0
**  This is a code-generated file.
**
************************************************************************
**
**  Title: } title {
**  Build: } ver3 {
**  Date:  } now/date {
**  File:  } file {
**
} by 
{***********************************************************************/

}
]]
