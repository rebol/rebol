/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**  Copyright 2012 REBOL Technologies
**  REBOL is a trademark of REBOL Technologies
**  Licensed under the Apache License, Version 2.0
**  This is a code-generated file.
**
************************************************************************
**
**  Title: Port Modes
**  Build: A0
**  Date:  10-Feb-2021
**  File:  port-modes.h
**
**  AUTO-GENERATED FILE - Do not modify. (From: make-boot.reb)
**
***********************************************************************/


enum reb_port_modes {
	MODE_PORT_OWNER_READ,
	MODE_PORT_OWNER_WRITE,
	MODE_PORT_OWNER_EXECUTE,
	MODE_PORT_GROUP_READ,
	MODE_PORT_GROUP_WRITE,
	MODE_PORT_GROUP_EXECUTE,
	MODE_PORT_WORLD_READ,
	MODE_PORT_WORLD_WRITE,
	MODE_PORT_WORLD_EXECUTE 
};

enum reb_console_modes {
	MODE_CONSOLE_ECHO,
	MODE_CONSOLE_LINE 
};
