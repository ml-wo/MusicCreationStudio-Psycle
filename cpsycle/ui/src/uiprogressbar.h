// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UIPROGRESSBAR_H)
#define UIPROGRESSBAR_H

#include "uicomponent.h"

typedef struct {
   ui_component component;   
   char* text;   
   float progress;
} ui_progressbar;

void ui_progressbar_init(ui_progressbar*, ui_component* parent);
void ui_progressbar_settext(ui_progressbar*, const char* text);
void ui_progressbar_setprogress(ui_progressbar*, float progress);
void ui_progressbar_tick(ui_progressbar*);

#endif