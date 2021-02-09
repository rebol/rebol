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
**  Title: Host Access Library
**  Build: A0
**  Date:  10-Feb-2021
**  File:  host-lib.h
**
**  AUTO-GENERATED FILE - Do not modify. (From: make-os-ext.reb)
**
***********************************************************************/


#define HOST_LIB_VER 0
#define HOST_LIB_SUM 40647
#define HOST_LIB_SIZE 44

extern REBDEV *Devices[];

typedef struct REBOL_Host_Lib {
	int size;
	unsigned int ver_sum;
	REBDEV **devices;
	REBREQ *std_io;
	REBINT (*os_get_pid)(void);
	REBINT (*os_get_uid)(void);
	REBINT (*os_set_uid)(REBINT uid);
	REBINT (*os_get_gid)(void);
	REBINT (*os_set_gid)(REBINT gid);
	REBINT (*os_get_euid)(void);
	REBINT (*os_set_euid)(REBINT uid);
	REBINT (*os_get_egid)(void);
	REBINT (*os_set_egid)(REBINT gid);
	REBINT (*os_send_signal)(REBINT pid, REBINT signal);
	REBINT (*os_kill)(REBINT pid);
	REBINT (*os_config)(int id, REBYTE *result);
	void *(*os_make)(size_t size);
	void (*os_free)(void *mem);
	void (*os_exit)(int code);
	void (*os_crash)(const REBYTE *title, const REBYTE *content);
	REBCHR *(*os_form_error)(int errnum, REBCHR *str, int len);
	REBOOL (*os_get_boot_path)(REBCHR *name);
	REBCHR *(*os_get_locale)(int what);
	REBINT (*os_get_env)(REBCHR *envname, REBCHR* envval, REBINT valsize);
	REBOOL (*os_set_env)(REBCHR *envname, REBCHR *envval);
	REBCHR *(*os_list_env)(void);
	void (*os_get_time)(REBOL_DAT *dat);
	i64 (*os_delta_time)(i64 base, int flags);
	int (*os_get_current_dir)(REBCHR **path);
	REBOOL (*os_set_current_dir)(REBCHR *path);
	void (*os_file_time)(REBREQ *file, REBOL_DAT *dat);
	void *(*os_open_library)(REBCHR *path, REBCNT *error);
	void (*os_close_library)(void *dll);
	void *(*os_find_function)(void *dll, const char *funcname);
	REBINT (*os_create_thread)(CFUNC init, void *arg, REBCNT stack_size);
	void (*os_delete_thread)(void);
	void (*os_task_ready)(REBINT tid);
	int (*os_create_process)(REBCHR *call, int argc, REBCHR* argv[], u32 flags, u64 *pid, int *exit_code, u32 input_type, void *input, u32 input_len, u32 output_type, void **output, u32 *output_len, u32 err_type, void **err, u32 *err_len);
	int (*os_reap_process)(int pid, int *status, int flags);
	int (*os_browse)(REBCHR *url, int reserved);
	REBOOL (*os_request_file)(REBRFR *fr);
	int (*os_call_device)(REBINT device, REBCNT command);
	int (*os_do_device)(REBREQ *req, REBCNT command);
	REBREQ *(*os_make_devreq)(int device);
	int (*os_abort_device)(REBREQ *req);
	int (*os_poll_devices)(void);
	int (*os_quit_devices)(int flags);
	REBINT (*os_wait)(REBCNT millisec, REBCNT res);
} REBOL_HOST_LIB;

//** Included by HOST *********************************************

#ifndef REB_DEF

extern REBINT OS_Get_PID(void);    // host-lib.c
extern REBINT OS_Get_UID(void);    // host-lib.c
extern REBINT OS_Set_UID(REBINT uid);    // host-lib.c
extern REBINT OS_Get_GID(void);    // host-lib.c
extern REBINT OS_Set_GID(REBINT gid);    // host-lib.c
extern REBINT OS_Get_EUID(void);    // host-lib.c
extern REBINT OS_Set_EUID(REBINT uid);    // host-lib.c
extern REBINT OS_Get_EGID(void);    // host-lib.c
extern REBINT OS_Set_EGID(REBINT gid);    // host-lib.c
extern REBINT OS_Send_Signal(REBINT pid, REBINT signal);    // host-lib.c
extern REBINT OS_Kill(REBINT pid);    // host-lib.c
extern REBINT OS_Config(int id, REBYTE *result);    // host-lib.c
extern void *OS_Make(size_t size);    // host-lib.c
extern void OS_Free(void *mem);    // host-lib.c
extern void OS_Exit(int code);    // host-lib.c
extern void OS_Crash(const REBYTE *title, const REBYTE *content);    // host-lib.c
extern REBCHR *OS_Form_Error(int errnum, REBCHR *str, int len);    // host-lib.c
extern REBOOL OS_Get_Boot_Path(REBCHR *name);    // host-lib.c
extern REBCHR *OS_Get_Locale(int what);    // host-lib.c
extern REBINT OS_Get_Env(REBCHR *envname, REBCHR* envval, REBINT valsize);    // host-lib.c
extern REBOOL OS_Set_Env(REBCHR *envname, REBCHR *envval);    // host-lib.c
extern REBCHR *OS_List_Env(void);    // host-lib.c
extern void OS_Get_Time(REBOL_DAT *dat);    // host-lib.c
extern i64 OS_Delta_Time(i64 base, int flags);    // host-lib.c
extern int OS_Get_Current_Dir(REBCHR **path);    // host-lib.c
extern REBOOL OS_Set_Current_Dir(REBCHR *path);    // host-lib.c
extern void OS_File_Time(REBREQ *file, REBOL_DAT *dat);    // host-lib.c
extern void *OS_Open_Library(REBCHR *path, REBCNT *error);    // host-lib.c
extern void OS_Close_Library(void *dll);    // host-lib.c
extern void *OS_Find_Function(void *dll, const char *funcname);    // host-lib.c
extern REBINT OS_Create_Thread(CFUNC init, void *arg, REBCNT stack_size);    // host-lib.c
extern void OS_Delete_Thread(void);    // host-lib.c
extern void OS_Task_Ready(REBINT tid);    // host-lib.c
extern int OS_Create_Process(REBCHR *call, int argc, REBCHR* argv[], u32 flags, u64 *pid, int *exit_code, u32 input_type, void *input, u32 input_len, u32 output_type, void **output, u32 *output_len, u32 err_type, void **err, u32 *err_len);    // host-lib.c
extern int OS_Reap_Process(int pid, int *status, int flags);    // host-lib.c
extern int OS_Browse(REBCHR *url, int reserved);    // host-lib.c
extern REBOOL OS_Request_File(REBRFR *fr);    // host-lib.c
extern int OS_Call_Device(REBINT device, REBCNT command);    // ../host-device.c
extern int OS_Do_Device(REBREQ *req, REBCNT command);    // ../host-device.c
extern REBREQ *OS_Make_Devreq(int device);    // ../host-device.c
extern int OS_Abort_Device(REBREQ *req);    // ../host-device.c
extern int OS_Poll_Devices(void);    // ../host-device.c
extern int OS_Quit_Devices(int flags);    // ../host-device.c
extern REBINT OS_Wait(REBCNT millisec, REBCNT res);    // ../host-device.c

#ifdef OS_LIB_TABLE

REBOL_HOST_LIB *Host_Lib;

REBOL_HOST_LIB Host_Lib_Init = {  // Host library function vector table.
	HOST_LIB_SIZE,
	(HOST_LIB_VER << 16) + HOST_LIB_SUM,
	(REBDEV**)&Devices,
	NULL, // Std_IO_Req is set on start-up
	OS_Get_PID,
	OS_Get_UID,
	OS_Set_UID,
	OS_Get_GID,
	OS_Set_GID,
	OS_Get_EUID,
	OS_Set_EUID,
	OS_Get_EGID,
	OS_Set_EGID,
	OS_Send_Signal,
	OS_Kill,
	OS_Config,
	OS_Make,
	OS_Free,
	OS_Exit,
	OS_Crash,
	OS_Form_Error,
	OS_Get_Boot_Path,
	OS_Get_Locale,
	OS_Get_Env,
	OS_Set_Env,
	OS_List_Env,
	OS_Get_Time,
	OS_Delta_Time,
	OS_Get_Current_Dir,
	OS_Set_Current_Dir,
	OS_File_Time,
	OS_Open_Library,
	OS_Close_Library,
	OS_Find_Function,
	OS_Create_Thread,
	OS_Delete_Thread,
	OS_Task_Ready,
	OS_Create_Process,
	OS_Reap_Process,
	OS_Browse,
	OS_Request_File,
	OS_Call_Device,
	OS_Do_Device,
	OS_Make_Devreq,
	OS_Abort_Device,
	OS_Poll_Devices,
	OS_Quit_Devices,
	OS_Wait,
};

#endif //OS_LIB_TABLE 

#else //REB_DEF

//** Included by REBOL ********************************************

extern	REBOL_HOST_LIB *Host_Lib;

#define OS_GET_PID(void)            Host_Lib->os_get_pid(void)
#define OS_GET_UID(void)            Host_Lib->os_get_uid(void)
#define OS_SET_UID(a)               Host_Lib->os_set_uid(a)
#define OS_GET_GID(void)            Host_Lib->os_get_gid(void)
#define OS_SET_GID(a)               Host_Lib->os_set_gid(a)
#define OS_GET_EUID(void)           Host_Lib->os_get_euid(void)
#define OS_SET_EUID(a)              Host_Lib->os_set_euid(a)
#define OS_GET_EGID(void)           Host_Lib->os_get_egid(void)
#define OS_SET_EGID(a)              Host_Lib->os_set_egid(a)
#define OS_SEND_SIGNAL(a,b)         Host_Lib->os_send_signal(a,b)
#define OS_KILL(a)                  Host_Lib->os_kill(a)
#define OS_CONFIG(a,b)              Host_Lib->os_config(a,b)
#define OS_MAKE(a)                  Host_Lib->os_make(a)
#define OS_FREE(a)                  Host_Lib->os_free(a)
#define OS_EXIT(a)                  Host_Lib->os_exit(a)
#define OS_CRASH(a,b)               Host_Lib->os_crash(a,b)
#define OS_FORM_ERROR(a,b,c)        Host_Lib->os_form_error(a,b,c)
#define OS_GET_BOOT_PATH(a)         Host_Lib->os_get_boot_path(a)
#define OS_GET_LOCALE(a)            Host_Lib->os_get_locale(a)
#define OS_GET_ENV(a,b,c)           Host_Lib->os_get_env(a,b,c)
#define OS_SET_ENV(a,b)             Host_Lib->os_set_env(a,b)
#define OS_LIST_ENV(void)           Host_Lib->os_list_env(void)
#define OS_GET_TIME(a)              Host_Lib->os_get_time(a)
#define OS_DELTA_TIME(a,b)          Host_Lib->os_delta_time(a,b)
#define OS_GET_CURRENT_DIR(a)       Host_Lib->os_get_current_dir(a)
#define OS_SET_CURRENT_DIR(a)       Host_Lib->os_set_current_dir(a)
#define OS_FILE_TIME(a,b)           Host_Lib->os_file_time(a,b)
#define OS_OPEN_LIBRARY(a,b)        Host_Lib->os_open_library(a,b)
#define OS_CLOSE_LIBRARY(a)         Host_Lib->os_close_library(a)
#define OS_FIND_FUNCTION(a,b)       Host_Lib->os_find_function(a,b)
#define OS_CREATE_THREAD(a,b,c)     Host_Lib->os_create_thread(a,b,c)
#define OS_DELETE_THREAD(void)      Host_Lib->os_delete_thread(void)
#define OS_TASK_READY(a)            Host_Lib->os_task_ready(a)
#define OS_CREATE_PROCESS(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) Host_Lib->os_create_process(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o)
#define OS_REAP_PROCESS(a,b,c)      Host_Lib->os_reap_process(a,b,c)
#define OS_BROWSE(a,b)              Host_Lib->os_browse(a,b)
#define OS_REQUEST_FILE(a)          Host_Lib->os_request_file(a)
#define OS_CALL_DEVICE(a,b)         Host_Lib->os_call_device(a,b)
#define OS_DO_DEVICE(a,b)           Host_Lib->os_do_device(a,b)
#define OS_MAKE_DEVREQ(a)           Host_Lib->os_make_devreq(a)
#define OS_ABORT_DEVICE(a)          Host_Lib->os_abort_device(a)
#define OS_POLL_DEVICES(void)       Host_Lib->os_poll_devices(void)
#define OS_QUIT_DEVICES(a)          Host_Lib->os_quit_devices(a)
#define OS_WAIT(a,b)                Host_Lib->os_wait(a,b)

#endif //REB_DEF
