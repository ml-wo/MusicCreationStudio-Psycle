// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uieditor.h"
#include "uiapp.h"
#include "uiwincompdetail.h"
#include "scintilla/include/scintilla.h"

#include <stdio.h>

#define SCI_ENABLED 1

static HMODULE scimodule = 0;

#define MAXTEXTLENGTH 65535

static int loadscilexer(void);
static void onappdestroy(void*, psy_ui_App* sender);
static void psy_ui_editor_styleclearall(psy_ui_Editor*);
static void psy_ui_editor_setcaretcolor(psy_ui_Editor*, uint32_t color);
static intptr_t sci(psy_ui_Editor*, uintptr_t msg, uintptr_t wparam,
	uintptr_t lparam);

void psy_ui_editor_init(psy_ui_Editor* self, psy_ui_Component* parent)
{  		
#ifdef SCI_ENABLED
	int err;

	if ((err = loadscilexer()) == 0) {	
		psy_ui_win32_component_init(&self->component, parent, TEXT("Scintilla"), 
			0, 0, 100, 20, WS_CHILD | WS_VISIBLE, 0);
		if (psy_ui_win_component_details(&self->component)->hwnd) {
			extern psy_ui_App app;

			psy_ui_editor_setcolor(self, psy_ui_defaults_color(&app.defaults));
			psy_ui_editor_setcaretcolor(self, psy_ui_defaults_color(&app.defaults));
			psy_ui_editor_setbackgroundcolor(self, 
				psy_ui_defaults_backgroundcolor(&app.defaults));
			psy_ui_editor_styleclearall(self);
		}
	} else
#endif	
	{
		psy_ui_win32_component_init(&self->component, parent, TEXT("STATIC"),
			0, 0, 100, 20,
			WS_CHILD | WS_VISIBLE | SS_CENTER, 0);
#ifdef SCI_ENABLED
		SetWindowText((HWND)psy_ui_win_component_details(&self->component)->hwnd, 
			"Editor can't be used.\n"
			"LoadLibrary SciLexer.dll failure\n"
			"Check if 'SciLexer.dll' is in the psycle bin directory or\n"
			"if you have the right version for your system.");
		psy_ui_error("LoadLibrary SciLexer.dll failure ...",
			"Error - Psycle Ui - Editor");
#else
	SetWindowText((HWND)psy_ui_win_component_details(&self->component)->hwnd, 
			"Editor can't be used. Scintilla disabled in build\n");
#endif
	}
}

int loadscilexer(void)
{
#ifdef SCI_ENABLED
	if (scimodule == 0) {
		scimodule = LoadLibrary ("SciLexer.dll");
		if (scimodule != NULL) {		
			psy_signal_connect(&app.signal_dispose, 0, onappdestroy);
		}		
	}
	return scimodule == NULL;
#endif
}

void onappdestroy(void* context, psy_ui_App* sender)
{	
	if (scimodule != NULL) {
		FreeLibrary(scimodule);
		scimodule = 0;	
	}
}

intptr_t sci(psy_ui_Editor* self, uintptr_t msg, uintptr_t wparam,
	uintptr_t lparam)
{
	return SendMessage((HWND) psy_ui_win_component_details(&self->component)->hwnd,
		msg, (WPARAM) wparam, (LPARAM) lparam);
}

void psy_ui_editor_load(psy_ui_Editor* self, const char* path)
{	
	FILE* fp;

	fp = fopen(path, "rb");
	if (fp) {
		char c;
		int pos = 0;
		char text[MAXTEXTLENGTH];

		memset(text, 0, MAXTEXTLENGTH);
		while ((c = fgetc(fp)) != EOF && pos < MAXTEXTLENGTH) {
			text[pos] = c;
			++pos;
		}		
		fclose(fp);		
		psy_ui_editor_addtext(self, text);		
	}
}

void psy_ui_editor_settext(psy_ui_Editor* self, const char* text)
{
	sci(self, SCI_SETTEXT, strlen(text), (uintptr_t) text);		
}

void psy_ui_editor_addtext(psy_ui_Editor* self, const char* text)
{
	sci(self, SCI_ADDTEXT, strlen(text), (uintptr_t) text);
}

void psy_ui_editor_clear(psy_ui_Editor* self)
{
	sci(self, SCI_CLEARALL, 0, 0);
}

void psy_ui_editor_setcolor(psy_ui_Editor* self, uint32_t color)
{
	sci(self, SCI_STYLESETFORE, STYLE_DEFAULT, color);  
}

void psy_ui_editor_setbackgroundcolor(psy_ui_Editor* self, uint32_t color)
{
	sci(self, SCI_STYLESETBACK, STYLE_DEFAULT, color);  
}

void psy_ui_editor_styleclearall(psy_ui_Editor* self)
{
	sci(self, SCI_STYLECLEARALL, 0, 0);
}

void psy_ui_editor_setcaretcolor(psy_ui_Editor* self, uint32_t color)
{
	sci(self, SCI_SETCARETFORE, color, 0);	
}

void psy_ui_editor_preventedit(psy_ui_Editor* self)
{	
	sci(self, SCI_SETREADONLY, 1, 0);
}

void psy_ui_editor_enableedit(psy_ui_Editor* self)
{
	sci(self, SCI_SETREADONLY, 0, 0);	
}
