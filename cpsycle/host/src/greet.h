// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(GREET_H)
#define GREET_H

#include <uilabel.h>
#include <uibutton.h>
#include <uilistbox.h>

typedef struct {
	psy_ui_Component component;	
	psy_ui_Label header;
	psy_ui_Label thanks;
	psy_ui_ListBox greetz;
	psy_ui_Button original;
	int current;
} Greet;

void greet_init(Greet* greet, psy_ui_Component* parent);

#endif
