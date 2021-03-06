// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instnotemapview.h"
#include <math.h>
#include <string.h>
#include "../../detail/portable.h"

static void instrumententryview_init(InstrumentEntryView*, psy_ui_Component* parent,
	InstrumentParameterView*);
static void instrumententryview_setinstrument(InstrumentEntryView*, psy_audio_Instrument*);
static void instrumentnotemapview_setmetrics(InstrumentNoteMapView*,
	InstrumentNoteMapMetrics);
static void instrumentparameterview_setinstrument(InstrumentParameterView*,
	psy_audio_Instrument*);
static void instrumententryview_ondraw(InstrumentEntryView*, psy_ui_Component* sender,
	psy_ui_Graphics*);
static void instrumententryview_onsize(InstrumentEntryView*, psy_ui_Component* sender, psy_ui_Size* size);
static void instrumententryview_onscroll(InstrumentEntryView*, psy_ui_Component* sender,
	int stepx, int stepy);
static void instrumententryview_adjustscroll(InstrumentEntryView*);
static void instrumententryview_onmousedown(InstrumentEntryView*,
	psy_ui_Component* sender, psy_ui_MouseEvent*);
static void instrumententryview_onmousemove(InstrumentEntryView*,
	psy_ui_Component* sender, psy_ui_MouseEvent*);
static void instrumententryview_onmouseup(InstrumentEntryView*,
	psy_ui_Component* sender, psy_ui_MouseEvent*);
static int isblack(int key);
static int whitekeytokey(int whitekey);
static int screentokey(int x, int keysize);
// instrument parameter view
static void instrumentparameterview_init(InstrumentParameterView*,
	psy_ui_Component* parent);
static void instrumentparameterview_ondraw(InstrumentParameterView*,
	psy_ui_Component* sender, psy_ui_Graphics*);
static void instrumentnotemapbuttons_init(InstrumentNoteMapButtons*,
	psy_ui_Component* parent);
// keyboardview
static void instrumentkeyboardview_ondraw(InstrumentKeyboardView*,
	psy_ui_Component* sender, psy_ui_Graphics*);

void instrumentnotemapview_init(InstrumentNoteMapView* self,
	psy_ui_Component* parent)
{
	psy_ui_Margin margin;

	psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makeeh(1.5),
		psy_ui_value_makepx(0));
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	self->metrics.keysize = 8;
	self->metrics.lineheight = 15;
	psy_ui_label_init(&self->label, &self->component);
	psy_ui_label_settext(&self->label, "Notemap");
	psy_ui_component_setalign(&self->label.component, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->label.component, &margin);
	instrumentnotemapbuttons_init(&self->buttons, &self->component);
	psy_ui_component_setalign(&self->buttons.component, psy_ui_ALIGN_TOP);
	psy_ui_component_setmargin(&self->buttons.component, &margin);
	instrumentparameterview_init(&self->parameterview, &self->component);
	psy_ui_component_setalign(&self->parameterview.component, psy_ui_ALIGN_LEFT);	
	instrumententryview_init(&self->entryview,
		&self->component, &self->parameterview);	
	psy_ui_component_setalign(&self->entryview.component, psy_ui_ALIGN_CLIENT);
	instrumentkeyboardview_init(&self->keyboard, &self->component);
	psy_ui_component_setalign(&self->keyboard.component, psy_ui_ALIGN_BOTTOM);
	instrumentnotemapview_setmetrics(self, self->metrics);
}

void instrumentnotemapview_setinstrument(InstrumentNoteMapView* self,
	psy_audio_Instrument* instrument)
{
	self->instrument = instrument;
	instrumententryview_setinstrument(&self->entryview, instrument);
	instrumentparameterview_setinstrument(&self->parameterview, instrument);
}

void instrumentnotemapview_setmetrics(InstrumentNoteMapView* self,
	InstrumentNoteMapMetrics metrics)
{
	self->metrics = metrics;
	self->entryview.metrics = metrics;
	self->parameterview.metrics = metrics;
	self->keyboard.metrics = metrics;
}

void instrumentnotemapview_update(InstrumentNoteMapView* self)
{
	instrumententryview_adjustscroll(&self->entryview);
	psy_ui_component_invalidate(&self->component);
}

void instrumentkeyboardview_init(InstrumentKeyboardView* self,
	psy_ui_Component* parent)
{
	self->dy = 0;
	self->metrics.keysize = 6;
	self->metrics.lineheight = 15;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_resize(&self->component, 0, 40);
	psy_signal_connect(&self->component.signal_draw, self,
		instrumentkeyboardview_ondraw);
}

int isblack(int key)
{
	int offset = key % 12;
	// 0 1 2 3 4 5 6 7 8 9 10 11
	// c   d   e f   g   a    h 
	return (offset == 1 || offset == 3 || offset == 6 || offset == 8 
		|| offset == 10);	
}

void instrumentkeyboardview_ondraw(InstrumentKeyboardView* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{		
	int keymin = 0;
	int keymax = NOTECOMMANDS_RELEASE;
	int key;	
	int keyboardsize;
	int keysize = self->metrics.keysize;
	int cp = 0;
	float top = 0.50;
	float bottom = 1 - top;
	psy_ui_TextMetric tm;
	psy_ui_Size size;

	tm = psy_ui_component_textmetric(&self->component);
	keyboardsize = (keymax - keymin) * keysize;
	size = psy_ui_component_size(&self->component);
	psy_ui_setcolor(g, 0x00333333);

	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolor(g, 0x00333333);
	for (key = keymin; key < keymax; ++key) {					
		psy_ui_drawline(g, cp, 0, cp, size.height);
		if (!isblack(key)) {
			psy_ui_Rectangle r;

			psy_ui_setrectangle(&r, cp, 0, keysize, size.height);
			psy_ui_drawsolidrectangle(g, r, 0x00CACACA);
			psy_ui_drawline(g, cp, 0, cp, size.height);			
			cp += keysize;			
			psy_ui_drawline(g, cp, 0, cp, size.height);			
		}
	}
	psy_ui_settextcolor(g, 0x00CACACA);
	for (cp = 0, key = keymin; key < keymax; ++key) {							
		if (!isblack(key)) {			
			cp += keysize;			
		} else {
			psy_ui_Rectangle r;
			int x;
			int width;

			x = cp - (int)(keysize * 0.8 / 2);
			width = (int)(keysize * 0.8);
			psy_ui_setrectangle(&r, x, 0, width, (int)(size.height * top));
			psy_ui_drawsolidrectangle(g, r, 0x00444444);
		}
	}
}

// entry view
void instrumententryview_init(InstrumentEntryView* self,
	psy_ui_Component* parent, InstrumentParameterView* parameterview)
{
	self->parameterview = parameterview;
	self->instrument = 0;
	self->dy = 0;
	self->metrics.keysize = 8;
	self->metrics.lineheight = 15;
	self->dragmode = 0;
	self->selected = NOINSTRUMENT_INDEX;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_doublebuffer(&self->component);
	self->component.scrollstepy = 45;	
	psy_signal_connect(&self->component.signal_draw, self,
		instrumententryview_ondraw);
	psy_signal_connect(&self->component.signal_size, self,
		instrumententryview_onsize);
	psy_signal_connect(&self->component.signal_scroll, self,
		instrumententryview_onscroll);
	psy_signal_connect(&self->component.signal_mousedown, self,
		instrumententryview_onmousedown);
	psy_signal_connect(&self->component.signal_mousemove, self,
		instrumententryview_onmousemove);
	psy_signal_connect(&self->component.signal_mouseup, self,
		instrumententryview_onmouseup);	
}

void instrumententryview_setinstrument(InstrumentEntryView* self, psy_audio_Instrument* instrument)
{
	self->instrument = instrument;
	psy_ui_component_invalidate(&self->component);
}

int numwhitekey(int key)
{
	int octave = key / 12;
	int offset = key % 12;
	int c = 0;
	int i;

	for (i = 1; i <= offset; ++i) {
		if (!isblack(i)) ++c;
	}
	return octave * 7 + c;
}

int whitekeytokey(int whitekey)
{
	int octave = whitekey / 7;
	int offset = whitekey % 7;
	// 0 1 2 3 4 5 6 7 8 9 10 11
	// c   d   e f   g   a    h 
	static int numkey[] = { 0, 2, 4, 5, 7, 9, 11 };	
	return octave * 12 + numkey[offset];	
}

void instrumententryview_ondraw(InstrumentEntryView* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{	
	if (self->instrument) {
		int cpy = 0;
		psy_List* p;
		psy_ui_Size size;
		int keysize = self->metrics.keysize;
		uintptr_t c = 0;
		int width = keysize * numwhitekey(NOTECOMMANDS_RELEASE);		

		size = psy_ui_component_size(&self->component);		
		for (p = self->instrument->entries; p != 0; p = p->next, ++c) {
			psy_audio_InstrumentEntry* entry;
			int startx;
			int endx;			

			entry = (psy_audio_InstrumentEntry*) p->entry;			
			startx = (int)(
				(float) numwhitekey(entry->keyrange.low) / 
					numwhitekey(NOTECOMMANDS_RELEASE) * width) +
					(int) (isblack(entry->keyrange.low)
							 ? self->metrics.keysize / 2 : 0);
			endx = (int)(
				(float)numwhitekey(entry->keyrange.high + 1) / 
					numwhitekey(NOTECOMMANDS_RELEASE) * width) +
					(int) (isblack(entry->keyrange.high + 1)
							 ? self->metrics.keysize / 2 : 0);
			if (c == self->selected) {
				psy_ui_setcolor(g, 0x00EAEAEA);
			} else {
				psy_ui_setcolor(g, 0x00CACACA);
			}
			psy_ui_drawline(g, startx, cpy + 5 + self->dy, endx, cpy + 5 + self->dy);
			psy_ui_drawline(g, startx, cpy + self->dy, startx, cpy + self->dy + 10);
			psy_ui_drawline(g, endx, cpy + self->dy, endx, cpy + self->dy + 10);
			cpy += self->metrics.lineheight * 3;
		}
	}
}

void instrumententryview_onsize(InstrumentEntryView* self,
	psy_ui_Component* sender, psy_ui_Size* size)
{
	instrumententryview_adjustscroll(self);
}

void instrumententryview_onscroll(InstrumentEntryView* self, psy_ui_Component* sender,
	int stepx, int stepy)
{
	self->dy += (stepy * self->metrics.lineheight * 3);
	self->parameterview->dy = self->dy;
	psy_ui_component_invalidate(&self->parameterview->component);
}

void instrumententryview_adjustscroll(InstrumentEntryView* self)
{
	psy_ui_Size size;
	int vscrollmax;
	int numentries;
	int visientries;

	size = psy_ui_component_size(&self->component);
	numentries = self->instrument
		? psy_list_size(instrument_entries(self->instrument))
		: 0;	
	visientries = size.height / (self->metrics.lineheight * 3);
	vscrollmax = numentries < visientries ? 0 : numentries - visientries;
	psy_ui_component_setverticalscrollrange(&self->component, 0, vscrollmax);
}

void instrumententryview_onmousedown(InstrumentEntryView* self,
	psy_ui_Component* sender, psy_ui_MouseEvent* ev)
{
	if (self->instrument) {
		psy_audio_InstrumentEntry* entry;
		if (self->instrument) {
			uintptr_t numentry;	

			numentry = (ev->y - self->dy) / (self->metrics.lineheight * 3);
			if (numentry < psy_list_size(instrument_entries(self->instrument))) {
				self->selected = numentry;
			} else {
				self->selected = NOINSTRUMENT_INDEX;
			}
		} else {
			self->selected = NOINSTRUMENT_INDEX;
		}
		self->dragmode = 1;
			
		entry = instrument_entryat(self->instrument, self->selected);
		if (entry) {
			if (abs(entry->keyrange.low  - screentokey(ev->x,
					self->metrics.keysize)) < 
				abs(entry->keyrange.high  - screentokey(ev->x,
					self->metrics.keysize))) {
				self->dragmode = INSTVIEW_DRAG_LEFT;
			} else {
				self->dragmode = INSTVIEW_DRAG_RIGHT;
			}
			psy_ui_component_capture(&self->component);
		}
		psy_ui_component_invalidate(&self->component);
	}
}

void instrumententryview_onmousemove(InstrumentEntryView* self,
	psy_ui_Component* sender, psy_ui_MouseEvent* ev)
{	
	if (self->dragmode != INSTVIEW_DRAG_NONE && self->instrument) {
		psy_audio_InstrumentEntry* entry;
		
		entry = instrument_entryat(self->instrument, self->selected);
		if (entry) {
			if (self->dragmode == INSTVIEW_DRAG_LEFT) {
				entry->keyrange.low = screentokey(ev->x,
					self->metrics.keysize);
				if (entry->keyrange.low > entry->keyrange.high) {
					entry->keyrange.low = entry->keyrange.high;
				}
			} else {
				entry->keyrange.high = screentokey(ev->x,
					self->metrics.keysize);
				if (entry->keyrange.high < entry->keyrange.low) {
					entry->keyrange.high = entry->keyrange.low;
				}
			}
			psy_ui_component_invalidate(&self->component);
			psy_ui_component_invalidate(&self->parameterview->component);
		}
	}
}

void instrumententryview_onmouseup(InstrumentEntryView* self,
	psy_ui_Component* sender, psy_ui_MouseEvent* ev)
{
	if (self->instrument) {
		self->dragmode = 0;
		psy_ui_component_releasecapture(&self->component);
	}
}

int screentokey(int x, int keysize)
{
	int rv;	
	int numwhitekey;

	numwhitekey = x / keysize;
	rv = whitekeytokey(numwhitekey);	
	// 0 1 2 3 4 5 6 7 8 9 10 11
	// c   d   e f   g   a    h 
	if ((rv % 12) != 4 && (rv % 12) != 11) {		
		int delta;

		delta = x - numwhitekey * keysize;
		if (delta > 0.5 * keysize) {
			++rv;
		}
	}
	return rv;
}


void instrumentparameterview_init(InstrumentParameterView* self,
	psy_ui_Component* parent)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_doublebuffer(&self->component);
	self->instrument = 0;
	self->dy = 0;		
	psy_signal_connect(&self->component.signal_draw, self,
		instrumentparameterview_ondraw);
	psy_ui_component_resize(&self->component, 100, 200);
}

void instrumentparameterview_setinstrument(InstrumentParameterView* self,
		psy_audio_Instrument* instrument)
{
	self->instrument = instrument;
	psy_ui_component_invalidate(&self->component);
}

void instrumentparameterview_ondraw(InstrumentParameterView* self,
	psy_ui_Component* sender, psy_ui_Graphics* g)
{	
	if (self->instrument) {
		int cpy = 0;
		psy_List* p;
		psy_ui_Size size;
		int keysize = self->metrics.keysize;
		int width = keysize * numwhitekey(NOTECOMMANDS_RELEASE);		

		size = psy_ui_component_size(&self->component);
		psy_ui_setcolor(g, 0x00CACACA);
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		psy_ui_settextcolor(g, 0x00CACACA);
		for (p = self->instrument->entries; p != 0; p = p->next) {
			psy_audio_InstrumentEntry* entry;
			char text[40];

			entry = (psy_audio_InstrumentEntry*) p->entry;
			psy_snprintf(text, 40, "Sample %0X:%0X", entry->sampleindex.slot,
				entry->sampleindex.subslot);
			psy_ui_textout(g, 0, cpy + self->dy, text, strlen(text));
			psy_snprintf(text, 40, "Notes %.3d - %.3d", entry->keyrange.low,
				entry->keyrange.high);
			psy_ui_textout(g, 0, cpy + self->dy + self->metrics.lineheight, text,
				strlen(text));
			psy_snprintf(text, 40, "Volume %.2X - %.2X", entry->velocityrange.low,
				entry->velocityrange.high);
			psy_ui_textout(g, 0, cpy + self->dy + self->metrics.lineheight * 2,
				text, strlen(text));
			cpy += (self->metrics.lineheight * 3);
		}
	}
}

void instrumentnotemapbuttons_init(InstrumentNoteMapButtons* self,
	psy_ui_Component* parent)
{
	psy_ui_Margin margin;

	psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(1.5), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_button_init(&self->add, &self->component);
	psy_ui_button_settext(&self->add, "Add");
	psy_ui_button_init(&self->remove, &self->component);
	psy_ui_button_settext(&self->remove, "Remove");
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, 0),
		psy_ui_ALIGN_LEFT,
			&margin));
}
