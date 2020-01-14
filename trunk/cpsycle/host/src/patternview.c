// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternview.h"
#include <portable.h>

static void patternview_ontabbarchange(PatternView*, psy_ui_Component* sender,
	int tabindex);
static void patternview_onshow(PatternView*, psy_ui_Component* sender);
static void patternview_onhide(PatternView*, psy_ui_Component* sender);
static void patternview_onsongchanged(PatternView*, Workspace* sender);
static void patternview_onsequenceselectionchanged(PatternView*,
	Workspace* sender);
static void patternview_onpropertiesapply(PatternView*,
	psy_ui_Component* sender);
static void patternview_onkeydown(PatternView*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void patternview_onkeyup(PatternView*, psy_ui_Component* sender,
	psy_ui_KeyEvent*);
static void patternview_onfocus(PatternView*, psy_ui_Component* sender);
static void patternviewstatus_ondraw(PatternViewStatus*, psy_ui_Graphics*);
static void patternviewstatus_onpreferredsize(PatternViewStatus* self,
	psy_ui_Size* limit, psy_ui_Size* rv);
static void patternviewstatus_onpatterneditpositionchanged(PatternViewStatus*,
	Workspace* sender);
static void patternviewstatus_onsequenceselectionchanged(PatternViewStatus*,
	Workspace* sender);
static void patternviewbar_ondefaultline(PatternViewBar*,
	psy_ui_CheckBox* sender);
void patternviewbar_initalign(PatternViewBar*);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(PatternViewStatus* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			patternviewstatus_onpreferredsize;
		vtable.ondraw = (psy_ui_fp_ondraw) patternviewstatus_ondraw;
	}
}

void patternviewstatus_init(PatternViewStatus* self, psy_ui_Component* parent,
	Workspace* workspace)
{		
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	ui_component_doublebuffer(&self->component);	
	psy_signal_connect(&self->component.signal_draw, self,
		patternviewstatus_ondraw);
	psy_signal_connect(&workspace->signal_patterneditpositionchanged, self,
		patternviewstatus_onpatterneditpositionchanged);	
	psy_signal_connect(&workspace->signal_sequenceselectionchanged,
		self, patternviewstatus_onsequenceselectionchanged);
}

void patternviewstatus_onsequenceselectionchanged(PatternViewStatus* self,
	Workspace* sender)
{
	ui_component_invalidate(&self->component);
}

void patternviewstatus_onpatterneditpositionchanged(PatternViewStatus* self,
	Workspace* sender)
{
	ui_component_invalidate(&self->component);
}

void patternviewstatus_ondraw(PatternViewStatus* self, psy_ui_Graphics* g)
{
	char text[256];
	PatternEditPosition editposition;
	SequencePosition sequenceposition;
	SequenceEntry* sequenceentry;
	int pattern;
	psy_ui_Size size;
	psy_ui_TextMetric tm;

	size = ui_component_size(&self->component);
	tm = ui_component_textmetric(&self->component);
	editposition = workspace_patterneditposition(self->workspace);
	sequenceposition = workspace_sequenceselection(self->workspace).editposition;		
	sequenceentry = sequenceposition_entry(&sequenceposition);	
	if (sequenceentry) {
		pattern = sequenceentry->pattern;
	} else {
		pattern = -1;
	}	
	ui_settextcolor(g, 0x00D1C5B6);
	ui_setbackgroundmode(g, TRANSPARENT);
	psy_snprintf(text, 256, "Pat %d  Ln %d  Trk %d  Col %d:%d Edit",
		pattern,
		editposition.line,
		editposition.track,
		editposition.column,
		editposition.digit);
	ui_textout(g, 0, (size.height - tm.tmHeight) / 2, text, strlen(text));
}

void patternviewstatus_onpreferredsize(PatternViewStatus* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{				
	if (rv) {
		psy_ui_TextMetric tm;
	
		tm = ui_component_textmetric(&self->component);
		rv->width = tm.tmAveCharWidth * 40;
		rv->height = (int)(tm.tmHeight * 1.5);
	}
}

void patternviewbar_init(PatternViewBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	self->workspace = workspace;
	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);	
	stepbox_init(&self->step, &self->component, workspace);
	psy_ui_checkbox_init(&self->movecursorwhenpaste, &self->component);
	psy_ui_checkbox_settext(&self->movecursorwhenpaste, "Move Cursor When Paste");
	psy_ui_checkbox_init(&self->defaultentries, &self->component);
	psy_ui_checkbox_settext(&self->defaultentries, "Default Line");
	if (workspace_showgriddefaults(self->workspace)) {
		psy_ui_checkbox_check(&self->defaultentries);
	}
	psy_signal_connect(&self->defaultentries.signal_clicked, self,
		patternviewbar_ondefaultline);
	patternviewstatus_init(&self->status, &self->component, workspace);
	patternviewbar_initalign(self);	
}

void patternviewbar_ondefaultline(PatternViewBar* self, psy_ui_CheckBox* sender)
{
	psy_Properties* pv;

	pv = psy_properties_findsection(self->workspace->config, "visual.patternview");
	if (pv) {
		psy_Properties* p;
		
		p = psy_properties_read(pv, "griddefaults");
		if (p) {			
			psy_properties_write_bool(pv, "griddefaults", !psy_properties_value(p));
			psy_signal_emit(&self->workspace->signal_configchanged, self->workspace, 1, p);
		}
	}
}

void patternviewbar_initalign(PatternViewBar* self)
{		
	psy_ui_Margin margin;

	psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2.0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));			
	psy_list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		psy_ui_ALIGN_LEFT,
		&margin));		
}

void patternview_init(PatternView* self, 
		psy_ui_Component* parent,
		psy_ui_Component* tabbarparent,		
		Workspace* workspace)
{
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_NONE);
	psy_signal_connect(&self->component.signal_keydown, self,
		patternview_onkeydown);
	psy_signal_connect(&self->component.signal_keyup, self,
		patternview_onkeyup);
	psy_signal_connect(&self->component.signal_focus, self, patternview_onfocus);
	psy_ui_notebook_init(&self->notebook, &self->component);
	ui_component_setalign(psy_ui_notebook_base(&self->notebook), psy_ui_ALIGN_CLIENT);
	ui_component_setbackgroundmode(psy_ui_notebook_base(&self->notebook), BACKGROUND_NONE);
	psy_ui_notebook_init(&self->editnotebook, psy_ui_notebook_base(&self->notebook));
	ui_component_setbackgroundmode(&self->editnotebook.component, BACKGROUND_NONE);
	psy_ui_notebook_setpageindex(&self->editnotebook, 0);
	trackerview_init(&self->trackerview, &self->editnotebook.component, workspace);	
	pianoroll_init(&self->pianoroll, &self->editnotebook.component, workspace);
	patternproperties_init(&self->properties, psy_ui_notebook_base(&self->notebook), 0);
	patternview_setpattern(self, patterns_at(&workspace->song->patterns, 0));	
	psy_signal_connect(&self->properties.applybutton.signal_clicked, self, patternview_onpropertiesapply);	
	// Tabbar
	tabbar_init(&self->tabbar, tabbarparent);
	ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);	
	ui_component_hide(tabbar_base(&self->tabbar));	
	tabbar_append(&self->tabbar, "Tracker");
	tabbar_append(&self->tabbar, "Pianoroll");	
	tabbar_append(&self->tabbar, "Split");
	tabbar_append(&self->tabbar, "Properties");	
	psy_signal_connect(&self->tabbar.signal_change, self,
		patternview_ontabbarchange);	
	tabbar_select(&self->tabbar, 0);	
	psy_signal_connect(&self->component.signal_show, self, patternview_onshow);
	psy_signal_connect(&self->component.signal_hide, self, patternview_onhide);
	psy_signal_connect(&workspace->signal_songchanged, self,
		patternview_onsongchanged);	
	psy_signal_connect(&workspace->signal_sequenceselectionchanged,
		self, patternview_onsequenceselectionchanged);	
}

void patternview_ontabbarchange(PatternView* self, psy_ui_Component* sender,
	int tabindex)
{
	if (tabindex < 2) {
		if (self->editnotebook.splitbar.hwnd) { //platform) {
			psy_ui_notebook_full(&self->editnotebook);						
		}
		psy_ui_notebook_setpageindex(&self->notebook, 0);
		psy_ui_notebook_setpageindex(&self->editnotebook, tabindex);
	} else 
	if (tabindex == 2) {
		psy_ui_notebook_setpageindex(&self->notebook, 0);
		if (!self->editnotebook.splitbar.hwnd) { // platform) {
			psy_ui_notebook_split(&self->editnotebook);			
		}
	} else {
		psy_ui_notebook_setpageindex(&self->notebook, 1);
	}
}

void patternview_setpattern(PatternView* self, psy_audio_Pattern* pattern)
{	
	trackerview_setpattern(&self->trackerview, pattern);
	pianoroll_setpattern(&self->pianoroll, pattern);
	patternproperties_setpattern(&self->properties, pattern);
}

void patternview_onshow(PatternView* self, psy_ui_Component* sender)
{			
	tabbar_base(&self->tabbar)->visible = 1;	
	ui_component_align(ui_component_parent(tabbar_base(&self->tabbar)));
	ui_component_show(tabbar_base(&self->tabbar));	
}

void patternview_onhide(PatternView* self, psy_ui_Component* sender)
{	
	ui_component_hide(tabbar_base(&self->tabbar));				
}

void patternview_onsongchanged(PatternView* self, Workspace* workspace)
{
	psy_audio_Pattern* pattern;
	SequenceSelection selection;	
	
	selection = workspace_sequenceselection(workspace);	
	if (selection.editposition.trackposition.tracknode) {
		SequenceEntry* entry;

		entry = (SequenceEntry*)
			selection.editposition.trackposition.tracknode->entry;
		pattern = patterns_at(&workspace->song->patterns, entry->pattern);	
	} else {
		pattern = 0;
	}
	patternview_setpattern(self, pattern);
	self->trackerview.sequenceentryoffset = 0.f;
	self->pianoroll.sequenceentryoffset = 0.f;
	self->pianoroll.pattern = pattern;
	ui_component_invalidate(&self->component);	
}

void patternview_onsequenceselectionchanged(PatternView* self,
	Workspace* workspace)
{	
	SequenceSelection selection;
	SequenceEntry* entry;

	selection = workspace_sequenceselection(workspace);
	entry = sequenceposition_entry(&selection.editposition);
	if (entry) {
		psy_audio_Pattern* pattern;

		pattern = patterns_at(&workspace->song->patterns, 
			entry->pattern);
		patternview_setpattern(self, pattern);
		self->trackerview.sequenceentryoffset = entry->offset;
		self->pianoroll.sequenceentryoffset = entry->offset;
	} else {
		patternview_setpattern(self, 0);		
		self->trackerview.sequenceentryoffset = 0.f;
		self->pianoroll.sequenceentryoffset = 0.f;
	}
	ui_component_invalidate(&self->component);		
}

void patternview_onpropertiesapply(PatternView* self, psy_ui_Component* sender)
{
	patternview_setpattern(self, self->properties.pattern);
}

void patternview_onkeydown(PatternView* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* keyevent)
{
	ui_component_propagateevent(sender);
}

void patternview_onkeyup(PatternView* self, psy_ui_Component* sender,
	psy_ui_KeyEvent* keyevent)
{
	ui_component_propagateevent(sender);
}

void patternview_onfocus(PatternView* self, psy_ui_Component* sender)
{
	ui_component_setfocus(&self->trackerview.grid.component);
}

