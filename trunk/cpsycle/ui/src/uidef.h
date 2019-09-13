// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UIDEF)
#define UIDEF

#include <windows.h>

typedef struct { 
	int x;
	int y;
} ui_point;

typedef struct {
	int left;
	int top;
	int right;
	int bottom;
} ui_rectangle;

typedef struct {
	int width;
	int height;
} ui_size;

typedef struct {
	int top;
	int right;
	int bottom;
	int left;
} ui_margin;

typedef struct {
	LOGFONT lf; 
} ui_fontinfo;

void ui_fontinfo_init(ui_fontinfo*, const char* family, int height);

typedef struct {
	HFONT hfont;
	int stock;
} ui_font;

void ui_setrectangle(ui_rectangle*, int left, int top, int width, int height);
void ui_setmargin(ui_margin*, int top, int right, int bottom, int left);
void ui_error(const char* err, const char* shorterr);

void ui_font_init(ui_font*, const ui_fontinfo* info);
void ui_font_dispose(ui_font*);

#endif