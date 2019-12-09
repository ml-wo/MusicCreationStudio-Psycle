// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "wavebox.h"

static void wavebox_ondraw(WaveBox*, ui_component* sender, ui_graphics*);
static void wavebox_ondestroy(WaveBox*, ui_component* sender);
static void wavebox_onmousedown(WaveBox*, ui_component* sender, MouseEvent*);
static void wavebox_onmousemove(WaveBox*, ui_component* sender, MouseEvent*);
static void wavebox_onmouseup(WaveBox*, ui_component* sender, MouseEvent*);
static int wavebox_hittest(WaveBox*, uintptr_t frame, int x, int epsilon);
int wavebox_hittest_range(WaveBox*, uintptr_t framemin, uintptr_t framemax, 
	int x);
uintptr_t wavebox_screentoframe(WaveBox* self, int x);
int wavebox_frametoscreen(WaveBox*, uintptr_t frame);
static void wavebox_swapselection(WaveBox*);

enum {
	SAMPLEBOX_DRAG_NONE,
	SAMPLEBOX_DRAG_LEFT,
	SAMPLEBOX_DRAG_RIGHT,
	SAMPLEBOX_DRAG_MOVE
};

void wavebox_init(WaveBox* self, ui_component* parent)
{			
	self->sample = 0;
	self->hasselection = 0;
	self->selectionstart = 0;
	self->selectionend = 0;
	self->zoomleft = 0.f;
	self->zoomright = 1.f;
	ui_component_init(&self->component, parent);
	self->component.doublebuffered = 1;
	signal_connect(&self->component.signal_draw, self, wavebox_ondraw);
	signal_connect(&self->component.signal_destroy, self, wavebox_ondestroy);
	signal_connect(&self->component.signal_mousedown, self,
		wavebox_onmousedown);
	signal_connect(&self->component.signal_mousemove, self,
		wavebox_onmousemove);
	signal_connect(&self->component.signal_mouseup, self,
		wavebox_onmouseup);
}

void wavebox_ondestroy(WaveBox* self, ui_component* sender)
{		
}

void wavebox_setsample(WaveBox* self, Sample* sample)
{
	self->sample = sample;
	ui_component_invalidate(&self->component);
}

void wavebox_ondraw(WaveBox* self, ui_component* sender, ui_graphics* g)
{	
	ui_rectangle r;
	ui_size size = ui_component_size(&self->component);	
	ui_setrectangle(&r, 0, 0, size.width, size.height);
	ui_setcolor(g, 0x00B1C8B0);
	if (!self->sample) {
		ui_textmetric tm;
		static const char* txt = "No wave loaded";

		tm = ui_component_textmetric(&self->component);
		ui_setbackgroundmode(g, TRANSPARENT);
		ui_settextcolor(g, 0x00D1C5B6);
		ui_textout(g, (size.width - tm.tmAveCharWidth * strlen(txt)) / 2,
			(size.height - tm.tmHeight) / 2, txt, strlen(txt));
	} else {
		amp_t scaley;
		float offsetstep;
		int x;
		int centery = size.height / 2;
		
		scaley = (size.height / 2) / (amp_t)32768;		
		offsetstep = (float) self->sample->numframes / size.width *
			(self->zoomright - self->zoomleft);
		if (self->hasselection) {
			ui_rectangle r;
			int startx;
			int endx;

			startx = wavebox_frametoscreen(self, self->selectionstart);
			if (startx < 0) {
				startx = 0;
			}
			endx = wavebox_frametoscreen(self, self->selectionend);			
			if (endx < 0) {
				endx = 0;
			}
			ui_setrectangle(&r, startx, 0, endx - startx, size.height);
			ui_drawsolidrectangle(g, r, 0x00B1C8B0);
		} 
		for (x = 0; x < size.width; ++x) {			
			uintptr_t frame = (int)(offsetstep * x + 
				(int)(self->sample->numframes * self->zoomleft));
			float framevalue;
			
			if (frame >= self->sample->numframes) {
				break;
			}
			framevalue = self->sample->channels.samples[0][frame];
			if (self->hasselection &&
				frame >= self->selectionstart &&
				frame < self->selectionend) {				
				ui_setcolor(g, 0x00232323);
			} else {
				ui_setcolor(g, 0x00B1C8B0);
			}
			ui_drawline(g, x, centery, x, centery + (int)(framevalue * scaley));
		}
	}
}

void wavebox_onmousedown(WaveBox* self, ui_component* sender, MouseEvent* ev)
{	
	if (self->sample && self->sample->numframes > 0) {
		if (self->hasselection) {
			if (wavebox_hittest(self, self->selectionstart, ev->x, 5)) {
				self->dragmode = SAMPLEBOX_DRAG_LEFT;
				SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			} else 
			if (wavebox_hittest(self, self->selectionend, ev->x, 5)) {
				self->dragmode = SAMPLEBOX_DRAG_RIGHT;
				SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			} else
			if (wavebox_hittest_range(self, self->selectionstart,
					self->selectionend, ev->x)) {
				self->dragmode = SAMPLEBOX_DRAG_MOVE;
				self->dragoffset = wavebox_screentoframe(self, ev->x)
					- self->selectionstart;
				SetCursor(LoadCursor(NULL, IDC_SIZEALL));
			} else {
				self->hasselection = 0;
			}
		} 
		if (!self->hasselection) {
			self->hasselection = 1;
			self->selectionstart = wavebox_screentoframe(self, ev->x);
			self->selectionend = self->selectionstart;
			self->dragmode = SAMPLEBOX_DRAG_RIGHT;		
		}
		ui_component_capture(&self->component);
	}
}

void wavebox_onmousemove(WaveBox* self, ui_component* sender, MouseEvent* ev)
{	
	if (self->sample && self->sample->numframes > 0) {
		if (self->dragmode == SAMPLEBOX_DRAG_LEFT) {		
			self->selectionstart = wavebox_screentoframe(self, ev->x);
			if (self->selectionstart > self->selectionend) {
				wavebox_swapselection(self);
				self->dragmode = SAMPLEBOX_DRAG_RIGHT;
			}
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			ui_component_invalidate(&self->component);
		} else
		if (self->dragmode == SAMPLEBOX_DRAG_RIGHT) {
			self->selectionend = wavebox_screentoframe(self, ev->x);
			if (self->selectionend < self->selectionstart) {
				wavebox_swapselection(self);
				self->dragmode = SAMPLEBOX_DRAG_LEFT;
			}
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));		
			ui_component_invalidate(&self->component);
		} else 
		if (self->dragmode == SAMPLEBOX_DRAG_MOVE) {
			uintptr_t length;
			intptr_t start;

			length = self->selectionend - self->selectionstart;
			start = wavebox_screentoframe(self, ev->x) - self->dragoffset;		
			if (start < 0) {
				start = 0;
			}
			self->selectionstart = start;
			self->selectionend = self->selectionstart + length;
			if (self->selectionend >= self->sample->numframes) {			
				self->selectionend = self->sample->numframes;
				self->selectionstart = self->sample->numframes - length;			
			}
			SetCursor(LoadCursor(NULL, IDC_SIZEALL));		
			ui_component_invalidate(&self->component);
		} else {
			if (self->hasselection && 
				(wavebox_hittest(self, self->selectionstart, ev->x, 5) ||
				 wavebox_hittest(self, self->selectionend, ev->x, 5))) {
				SetCursor(LoadCursor(NULL, IDC_SIZEWE));
			} else
			if (wavebox_hittest_range(self, self->selectionstart,
					self->selectionend, ev->x)) {
				SetCursor(LoadCursor(NULL, IDC_SIZEALL));
			}
		}
	}
}

void wavebox_swapselection(WaveBox* self)
{
	uintptr_t tmp;

	tmp = self->selectionstart;
	self->selectionstart = self->selectionend;
	self->selectionend = self->selectionstart;
}

int wavebox_hittest(WaveBox* self, uintptr_t frame, int x, int epsilon)
{		
	return frame >= wavebox_screentoframe(self, x - epsilon) && 
		   frame <= wavebox_screentoframe(self, x + epsilon);
}

int wavebox_hittest_range(WaveBox* self, uintptr_t framemin,
	uintptr_t framemax, int x)
{	
	uintptr_t frame = wavebox_screentoframe(self, x);
	return frame >= framemin && frame <= framemax;
}

uintptr_t wavebox_screentoframe(WaveBox* self, int x)
{
	uintptr_t rv = 0;
	intptr_t frame;

	if (self->sample) {
		ui_size size = ui_component_size(&self->component);	
		float offsetstep = (float) self->sample->numframes / size.width *
			(self->zoomright - self->zoomleft);

		frame = (int)(offsetstep * x) + 
			(int)(self->sample->numframes * self->zoomleft);
		if (frame < 0) {
			frame = 0;
		} else
		if (frame > (intptr_t) self->sample->numframes) {
			frame = self->sample->numframes;			
		}
		rv = frame;
	}
	return rv;
}

int wavebox_frametoscreen(WaveBox* self, uintptr_t frame)
{
	int rv = 0;

	if (self->sample) {
		ui_size size = ui_component_size(&self->component);	
		float offsetstep = (float) self->sample->numframes / size.width *
			(self->zoomright - self->zoomleft);		
		rv = (int)((frame - (intptr_t)(self->sample->numframes *
				self->zoomleft)) / offsetstep);		
	}
	return rv;
}

void wavebox_onmouseup(WaveBox* self, ui_component* sender, MouseEvent* ev)
{
	if (self->sample && self->sample->numframes > 0) {
		ui_component_releasecapture(&self->component);
		self->dragmode = SAMPLEBOX_DRAG_NONE;
		if (self->selectionstart == self->selectionend) {
			self->hasselection = 0;
			ui_component_invalidate(&self->component);
		}	
	}
}

void wavebox_setzoom(WaveBox* self, float zoomleft, float zoomright)
{
	self->zoomleft = zoomleft;
	self->zoomright = zoomright;
	ui_component_invalidate(&self->component);
}
