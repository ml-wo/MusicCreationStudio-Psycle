// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_DIR_H
#define psy_DIR_H

#ifdef __cplusplus
extern "C" {
#endif

struct FileSearch;

typedef int (*psy_fp_findfile)(void* context, const char* path, int flag);

void psy_dir_enumerate(void* self, const char* path, const char* wildcard, int flag,
	psy_fp_findfile enumproc);
void psy_dir_enumerate_recursive(void* self, const char* path, const char* wildcard, int flag,
	psy_fp_findfile enumproc);
void psy_dir_findfile(const char* searchpath, const char* wildcard, char* filepath);
const char* psy_dir_config(void);
char* workdir(char* buffer);
const char* pathenv(void);
void setpathenv(const char* path);
void insertpathenv(const char* path);
void psy_dir_extract_path(const char* path, char* prefix, char* name, char* ext);

#ifdef __cplusplus
}
#endif

#endif /* psy_DIR_H */

