/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Title: Device: File access for Posix
**  Author: Carl Sassenrath
**  Purpose: File open, close, read, write, and other actions.
**
**  Compile note: -D_FILE_OFFSET_BITS=64 to support large files
**
************************************************************************
**
**  NOTE to PROGRAMMERS:
**
**    1. Keep code clear and simple.
**    2. Document unusual code, reasoning, or gotchas.
**    3. Use same style for code, vars, indent(4), comments, etc.
**    4. Keep in mind Linux, OS X, BSD, big/little endian CPUs.
**    5. Test everything, then test it again.
**
***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <glob.h>

#include "reb-host.h"
#include "host-lib.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

// The BSD legacy names S_IREAD/S_IWRITE are not defined on e.g. Android.
#ifndef S_IREAD
#define S_IREAD S_IRUSR
#endif
#ifndef S_IWRITE
#define S_IWRITE S_IWUSR
#endif

// NOTE: the code below assumes a file id will never by zero. This should
// be safe. In posix, zero is stdin, which is handled by dev-stdio.c.


/***********************************************************************
**
**	Local Functions
**
***********************************************************************/

#ifndef DT_DIR
// dirent.d_type is a BSD extension, actually not part of POSIX
// reformatted from: http://ports.haiku-files.org/wiki/CommonProblems
static int Is_Dir(const char *path, const char *name)
{
	int len1 = strlen(path);
	int len2 = strlen(name);
	struct stat st;

	char pathname[len1 + 1 + len2 + 1 + 13];
	strcpy(pathname, path);

	/* Avoid UNC-path "//name" on Cygwin.  */
	if (len1 > 0 && pathname[len1 - 1] != '/')
		strcat(pathname, "/");

	strcat(pathname, name);

	if (stat(pathname, &st))
		return 0;

	return S_ISDIR(st.st_mode);
}
#endif

static REBOOL Seek_File_64(REBREQ *file)
{
	// Performs seek and updates index value. TRUE on success.
	// On error, returns FALSE and sets file->error field.
	int h = file->id;
	i64 result;

	if (file->file.index == -1) {
		// Append:
		result = lseek(h, 0, SEEK_END);
	}
	else {
		result = lseek(h, file->file.index, SEEK_SET);
	}

	if (result < 0) {
		file->error = -RFE_NO_SEEK;
		return 0;
	}

	file->file.index = result;

	return 1;
}

static int Get_File_Info(REBREQ *file)
{
	struct stat info;

	if (stat(file->file.path, &info)) {
		file->error = errno;
		return DR_ERROR;
	}

	if (S_ISDIR(info.st_mode)) {
		SET_FLAG(file->modes, RFM_DIR);
		file->file.size = MIN_I64; // using MIN_I64 to notify, that size should be reported as NONE
	}
	else {
		CLR_FLAG(file->modes, RFM_DIR);
		file->file.size = info.st_size;
	}
	file->file.time.l = (long)(info.st_mtime);

	return DR_DONE;
}


/***********************************************************************
**
*/	static int Read_Directory(REBREQ *dir, REBREQ *file)
/*
**		This function will read a file directory, one file entry
**		at a time, then close when no more files are found.
**
**	Procedure:
**
**		This function is passed directory and file arguments.
**		The dir arg provides information about the directory to read.
**		The file arg is used to return specific file information.
**
**		To begin, this function is called with a dir->handle that
**		is set to zero and a dir->file.path string for the directory.
**
**		The directory is opened and a handle is stored in the dir
**		structure for use on subsequent calls. If an error occurred,
**		dir->error is set to the error code and -1 is returned.
**		The dir->size field can be set to the number of files in the
**		dir, if it is known. The dir->file.index field can be used by this
**		function to store information between calls.
**
**		If the open succeeded, then information about the first file
**		is stored in the file argument and the function returns 0.
**		On an error, the dir->error is set, the dir is closed,
**		dir->handle is nulled, and -1 is returned.
**
**		The caller loops until all files have been obtained. This
**		action should be uninterrupted. (The caller should not perform
**		additional OS or IO operations between calls.)
**
**		When no more files are found, the dir is closed, dir->handle
**		is nulled, and 1 is returned. No file info is returned.
**		(That is, this function is called one extra time. This helps
**		for OSes that may deallocate file strings on dir close.)
**
**		Note that the dir->file.path can contain wildcards * and ?. The
**		processing of these can be done in the OS (if supported) or
**		by a separate filter operation during the read.
**
**		Store file date info in file->file.index or other fields?
**		Store permissions? Ownership? Groups? Or, require that
**		to be part of a separate request?
**
***********************************************************************/
{
	struct dirent *d;
	char *cp;
	DIR *h;
	int len, n = 0;

	// If no dir handle, open the dir:
	if (!(h = dir->handle)) {
		// Remove * from tail, if present. (Allowed because the
		// path was copied into to-local-path first).
		len = strlen((cp = dir->file.path));
		if (len > 0 && cp[len-1] == '*') {
			// keep track that we removed *
			n = len-1;
			cp[n] = 0;
		}
		h = opendir(dir->file.path);
		if (!h) {
			// revert back the * char as it may be part of pattern matching
			if (n > 0) cp[n] = '*';
			dir->error = errno;
			return DR_ERROR;
		}
		dir->handle = h;
		CLR_FLAG(dir->flags, RRF_DONE);
	}

	// Get dir entry (skip over the . and .. dir cases):
	do {
		// Read next file entry or error:
		if (!(d = readdir(h))) {
			//dir->error = errno;
			closedir(h);
			dir->handle = 0;
			//if (dir->error) return DR_ERROR;
			SET_FLAG(dir->flags, RRF_DONE); // no more files
			return DR_DONE;
		}
		cp = d->d_name;
	} while (cp[0] == '.' && (cp[1] == 0 || (cp[1] == '.' && cp[2] == 0)));

	file->modes = 0;
	COPY_BYTES(file->file.path, cp, MAX_FILE_NAME);

#ifdef DT_DIR
	// NOTE: not all posix filesystems support this (mainly
	// the Linux and BSD support it.) If this fails to build, a
	// different mechanism must be used. However, this is the
	// most efficient, because it does not require a separate
	// file system call for determining directories.
	if (d->d_type == DT_DIR) SET_FLAG(file->modes, RFM_DIR);
	// NOTE: DT_DIR may be enabled using _BSD_SOURCE define
	// https://stackoverflow.com/a/9241608/494472
#else
	if (Is_Dir(dir->file.path, file->file.path)) SET_FLAG(file->modes, RFM_DIR);
#endif

	// Line below DOES NOT WORK -- because we need full path.
	//Get_File_Info(file); // updates modes, size, time

	return DR_DONE;
}


/***********************************************************************
**
*/	static int Read_Pattern(REBREQ *dir, REBREQ *file)
/*
**		This function will read a file with wildcards, one file entry
**		at a time, then close when no more files are found.
**
**		Although GLOB allows to pass patterns which match content
**		thru multiple directories, that is intentionally disabled,
**		because such a functionality would not be easy to implement
**		and because result would have to be full path, which also
**		may not be the best choice from user's view.
**
**		Actually the result is truncated so only files are returned and
**		not complete paths.
**
***********************************************************************/
{
	char *cp;
	glob_t *g;
	int n, p, end = 0;
	int wld = -1;

	if (!(g = dir->handle)) {
		//printf("init pattern: %s\n", dir->file.path);

		n = strlen((cp = dir->file.path));
		for (p = 0; p < n; p++) {
			if (cp[p] == '/') {
				// store position of the directory separator
				end = p;
				if (wld > 0) {
					// don't support wildcards thru multiple directories
					// like: %../?/?.png
					// as this is not available on Windows

					//puts("Not supported pattern!");
					dir->error = -GLOB_NOMATCH; // result will be []
					return DR_ERROR;
				}
			}
			else if (cp[p] == '*' || cp[p] == '?') wld = p;
		}
		// keep position of the last directory separator so it can be used
		// to limit result into just a file and not a full path
		dir->clen = end + 1;

		g = MAKE_NEW(glob_t); // deallocate once done!		
		n = glob(dir->file.path, GLOB_MARK, NULL, g);
		if (n) {
			//printf("glob: %s err: %i errno: %i\n", dir->file.path, n, errno);
			globfree(g);
			OS_Free(g);
			dir->error = -n; // using negative number as on Windows
			return DR_ERROR;
		}
		//printf("found patterns: %li\n", g->gl_pathc);
		// all patterns are already in the glob buffer,
		// but we will not report them all at once
		dir->handle = g;
		dir->actual = 0;
		dir->length = g->gl_pathc;
		dir->modes  = 1 << RFM_PATTERN; // changing mode from RFM_DIR to RFM_PATTERN
		CLR_FLAG(dir->flags, RRF_DONE);
	}
	if(dir->actual >= g->gl_pathc) {
		globfree(g);
		OS_Free(g);
		SET_FLAG(dir->flags, RRF_DONE); // no more files
		return DR_DONE;
	}
	//printf("path[%i]: %s\n", dir->actual, g->gl_pathv[dir->actual]);
	file->modes = 0;
	//TODO: assert if: 0 <= dir->clen <= MAX_FILE_NAME ???
	// only file part is returned...
	COPY_BYTES(file->file.path, g->gl_pathv[dir->actual++] + dir->clen, MAX_FILE_NAME - dir->clen);
	return DR_DONE;
}

/***********************************************************************
**
*/	DEVICE_CMD Open_File(REBREQ *file)
/*
**		Open the specified file with the given modes.
**
**		Notes:
**		1.	The file path is provided in REBOL format, and must be
**			converted to local format before it is used.
**		2.	REBOL performs the required access security check before
**			calling this function.
**		3.	REBOL clears necessary fields of file structure before
**			calling (e.g. error and size fields).
**
***********************************************************************/
{
	int modes;
	int access = 0;
	int h;
	char *path;
	struct stat info;

	// Posix file names should be compatible with REBOL file paths:
	if (!(path = file->file.path)) {
		file->error = -RFE_BAD_PATH;
		return DR_ERROR;
	}

	// Set the modes:
	modes = (O_BINARY | GET_FLAG(file->modes, RFM_READ)) ? O_RDONLY : O_RDWR;

	if (GET_FLAGS(file->modes, RFM_WRITE, RFM_APPEND)) {
		modes = O_BINARY | O_RDWR | O_CREAT;
		if (
			GET_FLAG(file->modes, RFM_NEW) ||
			!(
				GET_FLAG(file->modes, RFM_READ) ||
				GET_FLAG(file->modes, RFM_APPEND) ||
				GET_FLAG(file->modes, RFM_SEEK)
			)
		) modes |= O_TRUNC;
	}

	//modes |= GET_FLAG(file->modes, RFM_SEEK) ? O_RANDOM : O_SEQUENTIAL;

	if (GET_FLAG(file->modes, RFM_READONLY))
		access = S_IREAD;
	else
		access = S_IREAD | S_IWRITE | S_IRGRP | S_IWGRP | S_IROTH;

	// Open the file:
	// printf("Open: %s %d %d\n", path, modes, access);
	h = open(path, modes, access);
	if (h < 0) {
		file->error = -RFE_OPEN_FAIL;
		goto fail;
	}

	// Confirm that a seek-mode file is actually seekable:
	if (GET_FLAG(file->modes, RFM_SEEK)) {
		if (lseek(h, 0, SEEK_CUR) < 0) {
			close(h);
			file->error = -RFE_BAD_SEEK;
			goto fail;
		}
	}

	// Fetch file size (if fails, then size is assumed zero):
	if (fstat(h, &info) == 0) {
		file->file.size = info.st_size;
		file->file.time.l = (i32)(info.st_mtime);
	}

	file->id = h;

	return DR_DONE;

fail:
	return DR_ERROR;
}


/***********************************************************************
**
*/	DEVICE_CMD Close_File(REBREQ *file)
/*
**		Closes a previously opened file.
**
***********************************************************************/
{
	if (file->id) {
		close(file->id);
		file->id = 0;
	}
	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Read_File(REBREQ *file)
/*
***********************************************************************/
{
	ssize_t num_bytes;

	if (GET_FLAG(file->modes, RFM_DIR)) {
		int ret = Read_Directory(file, (REBREQ*)file->data);
		// If there is no id yet and reading failed, we will
		// try to use file as a pattern...
		if (ret == DR_ERROR && !file->id) goto init_pattern;
		return ret;
	}
	else if (GET_FLAG(file->modes, RFM_PATTERN)) {
init_pattern:
		return Read_Pattern(file, (REBREQ*)file->data);
	}

	if (!file->id) {
		file->error = -RFE_NO_HANDLE;
		return DR_ERROR;
	}

	if (file->modes & ((1 << RFM_SEEK) | (1 << RFM_RESEEK))) {
		CLR_FLAG(file->modes, RFM_RESEEK);
		if (!Seek_File_64(file)) return DR_ERROR;
	}

	// printf("read %d len %d\n", file->id, file->length);
	num_bytes = read(file->id, file->data, file->length);
	if (num_bytes < 0) {
		file->error = -RFE_BAD_READ;
		return DR_ERROR;
	} else {
		file->actual = num_bytes;
		file->file.index += file->actual;
	}

	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Write_File(REBREQ *file)
/*
**	Bug?: update file->size value after write !?
**
***********************************************************************/
{
	ssize_t num_bytes;
	struct stat info;
	file->actual = 0;
	
	if (!file->id) {
		file->error = -RFE_NO_HANDLE;
		return DR_ERROR;
	}

	if (GET_FLAG(file->modes, RFM_APPEND)) {
		CLR_FLAG(file->modes, RFM_APPEND);
		lseek(file->id, 0, SEEK_END);
	}

	if (file->modes & ((1 << RFM_SEEK) | (1 << RFM_RESEEK) | (1 << RFM_TRUNCATE))) {
		CLR_FLAG(file->modes, RFM_RESEEK);
		if (!Seek_File_64(file)) return DR_ERROR;
		if (GET_FLAG(file->modes, RFM_TRUNCATE))
			if (ftruncate(file->id, file->file.index)) return DR_ERROR;
	}

	if (file->length > 0) {
		num_bytes = write(file->id, file->data, file->length);
		if (num_bytes < 0) {
			if (errno == ENOSPC) file->error = -RFE_DISK_FULL;
			else file->error = -RFE_BAD_WRITE;
			return DR_ERROR;
		} else {
			file->actual = (u32)num_bytes;
		}
	}
	// update new file info
	if (fstat(file->id, &info) == 0) {
		file->file.size = info.st_size;
		file->file.time.l = (i32)(info.st_mtime);
	}

	return DR_DONE;
}


/***********************************************************************
**
*/	DEVICE_CMD Query_File(REBREQ *file)
/*
**		Obtain information about a file. Return TRUE on success.
**		On error, return FALSE and set file->error code.
**
**		Note: time is in local format and must be converted
**
***********************************************************************/
{
	return Get_File_Info(file);
}


/***********************************************************************
**
*/	DEVICE_CMD Create_File(REBREQ *file)
/*
***********************************************************************/
{
	if (GET_FLAG(file->modes, RFM_DIR)) {
		if (!mkdir(file->file.path, 0777)) return DR_DONE;
		file->error = errno;
		return DR_ERROR;
	} else
		return Open_File(file);
}


/***********************************************************************
**
*/	DEVICE_CMD Delete_File(REBREQ *file)
/*
**		Delete a file or directory. Return TRUE if it was done.
**		The file->file.path provides the directory path and name.
**		For errors, return FALSE and set file->error to error code.
**
**		Note: Dirs must be empty to succeed
**
***********************************************************************/
{
	if (GET_FLAG(file->modes, RFM_DIR)) {
		if (!rmdir(file->file.path)) return DR_DONE;
	} else
		if (!remove(file->file.path)) return DR_DONE;

	file->error = errno;
	return DR_ERROR;
}


/***********************************************************************
**
*/	DEVICE_CMD Rename_File(REBREQ *file)
/*
**		Rename a file or directory.
**		Note: cannot rename across file volumes.
**
***********************************************************************/
{
	if (!rename(file->file.path, file->data)) return DR_DONE;
	file->error = errno;
	return DR_ERROR;
}


/***********************************************************************
**
*/	DEVICE_CMD Poll_File(REBREQ *file)
/*
***********************************************************************/
{
	return DR_DONE;		// files are synchronous (currently)
}


/***********************************************************************
**
**	Command Dispatch Table (RDC_ enum order)
**
***********************************************************************/

static DEVICE_CMD_FUNC Dev_Cmds[RDC_MAX] = {
	0,
	0,
	Open_File,
	Close_File,
	Read_File,
	Write_File,
	Poll_File,
	0,	// connect
	Query_File,
	0,	// modify
	Create_File,
	Delete_File,
	Rename_File,
};

DEFINE_DEV(Dev_File, "File IO", 1, Dev_Cmds, RDC_MAX, sizeof(REBREQ));
