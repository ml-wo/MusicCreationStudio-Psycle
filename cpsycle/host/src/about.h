// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(ABOUT_H)
#define ABOUT_H

#include <bitmap.h>
#include <uibutton.h>
#include <uilabel.h>
#include <uiedit.h>
#include <uinotebook.h>
#include <uiimage.h>


typedef struct {	
	ui_component component;		
	ui_label asio;
	ui_edit	sourceforge;
	ui_edit	psycledelics;
	ui_label steincopyright;		
	ui_label headercontrib;
	ui_edit	contrib;	
} Contrib;

void InitContrib(Contrib*, ui_component* parent);

typedef struct {
	ui_component component;
	ui_label versioninfo;
} Version;

void InitVersion(Version*, ui_component* parent);

typedef struct {	
	ui_component component;
	ui_notebook notebook;
	ui_image image;
	Contrib contrib;
	Version version;
	ui_button contribbutton;
	ui_button versionbutton;
	ui_button okbutton;		
} About;

void InitAbout(About*, ui_component* parent);		

#endif
