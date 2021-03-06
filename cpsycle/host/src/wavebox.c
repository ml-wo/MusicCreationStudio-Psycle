// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "wavebox.h"

#include <string.h>

static void wavebox_ondraw(WaveBox*, psy_ui_Graphics*);
static void wavebox_ondestroy(WaveBox*, psy_ui_Component* sender);
static void wavebox_onmousedown(WaveBox*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void wavebox_onmousemove(WaveBox*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void wavebox_onmouseup(WaveBox*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static int wavebox_hittest(WaveBox*, uintptr_t frame, int x, int epsilon);
psy_ui_Rectangle wavebox_framerangetoscreen(WaveBox*, uintptr_t framebegin,
	uintptr_t frameend);
int wavebox_hittest_range(WaveBox*, uintptr_t framemin, uintptr_t framemax, 
	int x);
uintptr_t wavebox_screentoframe(WaveBox*, int x);
int wavebox_frametoscreen(WaveBox*, uintptr_t frame);
static void wavebox_swapselection(WaveBox*);

enum {
	SAMPLEBOX_DRAG_NONE,
	SAMPLEBOX_DRAG_LEFT,
	SAMPLEBOX_DRAG_RIGHT,
	SAMPLEBOX_DRAG_MOVE
};

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(WaveBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);		
		vtable.ondraw = (psy_ui_fp_ondraw) wavebox_ondraw;
		vtable_initialized = 1;
	}
}

void wavebox_init(WaveBox* self, psy_ui_Component* parent)
{			
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_doublebuffer(&self->component);
	self->sample = 0;
	self->hasselection = 0;
	self->selectionstart = 0;
	self->selectionend = 0;
	self->zoomleft = 0.f;
	self->zoomright = 1.f;
	self->offsetstep = 0;	
	psy_signal_connect(&self->component.signal_destroy, self, wavebox_ondestroy);
	psy_signal_connect(&self->component.signal_mousedown, self,
		wavebox_onmousedown);
	psy_signal_connect(&self->component.signal_mousemove, self,
		wavebox_onmousemove);
	psy_signal_connect(&self->component.signal_mouseup, self,
		wavebox_onmouseup);
}

void wavebox_ondestroy(WaveBox* self, psy_ui_Component* sender)
{		
}

void wavebox_setsample(WaveBox* self, psy_audio_Sample* sample)
{
	psy_ui_Size size;

	size = psy_ui_component_size(&self->component);
	self->sample = sample;
	self->offsetstep = sample ? (float)self->sample->numframes / size.width *
		(self->zoomright - self->zoomleft) : 0.f;
	psy_ui_component_invalidate(&self->component);
}

void wavebox_ondraw(WaveBox* self, psy_ui_Graphics* g)
{	
	psy_ui_Rectangle r;
	psy_ui_Size size = psy_ui_component_size(&self->component);	
	psy_ui_setrectangle(&r, 0, 0, size.width, size.height);
	psy_ui_setcolor(g, 0x00B1C8B0);
	if (!self->sample) {
		psy_ui_TextMetric tm;
		static const char* txt = "No wave loaded";

		tm = psy_ui_component_textmetric(&self->component);
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		psy_ui_settextcolor(g, 0x00D1C5B6);
		psy_ui_textout(g, (size.width - tm.tmAveCharWidth * strlen(txt)) / 2,
			(size.height - tm.tmHeight) / 2, txt, strlen(txt));
	} else {
		psy_dsp_amp_t scaley;		
		int x;
		int centery = size.height / 2;
		psy_ui_Rectangle cont_loop_rc;
		psy_ui_Rectangle sustain_loop_rc;

		if (self->sample->looptype != LOOP_DO_NOT) {
			cont_loop_rc = wavebox_framerangetoscreen(self,
				self->sample->loopstart, self->sample->loopend);
		}
		if (self->sample->sustainlooptype != LOOP_DO_NOT) {
			sustain_loop_rc = wavebox_framerangetoscreen(self,
				self->sample->sustainloopstart,
				self->sample->sustainloopend);
		}
		scaley = (size.height / 2) / (psy_dsp_amp_t)32768;	

		if (self->sample && self->sample->looptype != LOOP_DO_NOT) {			
			psy_ui_drawsolidrectangle(g, cont_loop_rc, 0x00292929);
		}		
		if (self->hasselection) {			
			psy_ui_drawsolidrectangle(g, wavebox_framerangetoscreen(self,
				self->selectionend, self->selectionend), 0x00B1C8B0);			
		}		
		for (x = 0; x < size.width; ++x) {			
			uintptr_t frame = (int)(self->offsetstep * x + 
				(int)(self->sample->numframes * self->zoomleft));
			float framevalue;
			
			if (frame >= self->sample->numframes) {
				break;
			}
			framevalue = self->sample->channels.samples[0][frame];
			if (self->hasselection &&
				frame >= self->selectionstart &&
				frame < self->selectionend) {				
				psy_ui_setcolor(g, 0x00232323);
			} else
			if (self->sample->looptype != LOOP_DO_NOT &&
					psy_ui_rectangle_intersect(&cont_loop_rc, x, 0)) {
				psy_ui_setcolor(g, 0x00D1C5B6);
			} else			
			{
				psy_ui_setcolor(g, 0x00B1C8B0);
			}
			psy_ui_drawline(g, x, centery, x, centery + (int)(framevalue * scaley));
		}
		if (self->sample && self->sample->looptype != LOOP_DO_NOT) {
			psy_ui_setcolor(g, 0x00D1C5B6);
			psy_ui_drawline(g, cont_loop_rc.left, 0, cont_loop_rc.left + 1,
				size.height);
			psy_ui_drawline(g, cont_loop_rc.right, 0, cont_loop_rc.right + 1,
				size.height);
		}
		if (self->sample && self->sample->sustainlooptype != LOOP_DO_NOT) {
			psy_ui_setcolor(g, 0x00B6C5D1);
			psy_ui_drawline(g, sustain_loop_rc.left, 0, sustain_loop_rc.left + 1,
				size.height);
			psy_ui_drawline(g, sustain_loop_rc.right, 0, sustain_loop_rc.right + 1,
				size.height);
		}
	}
}

psy_ui_Rectangle wavebox_framerangetoscreen(WaveBox* self, uintptr_t framebegin,
	uintptr_t frameend)
{
	psy_ui_Rectangle rv;
	int startx;
	int endx;
	psy_ui_Size size;
	
	size = psy_ui_component_size(&self->component);
	startx = wavebox_frametoscreen(self, framebegin);
	if (startx < 0) {
		startx = 0;
	}
	endx = wavebox_frametoscreen(self, frameend);
	if (endx < 0) {
		endx = 0;
	}
	psy_ui_setrectangle(&rv, startx, 0, endx - startx, size.height);
	return rv;
}

void wavebox_onmousedown(WaveBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	if (self->sample && self->sample->numframes > 0) {
		if (self->hasselection) {
			if (wavebox_hittest(self, self->selectionstart, ev->x, 5)) {
				self->dragmode = SAMPLEBOX_DRAG_LEFT;
				psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
			} else 
			if (wavebox_hittest(self, self->selectionend, ev->x, 5)) {
				self->dragmode = SAMPLEBOX_DRAG_RIGHT;
				psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
			} else
			if (wavebox_hittest_range(self, self->selectionstart,
					self->selectionend, ev->x)) {
				self->dragmode = SAMPLEBOX_DRAG_MOVE;
				self->dragoffset = wavebox_screentoframe(self, ev->x)
					- self->selectionstart;
				psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_MOVE);
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
		psy_ui_component_capture(&self->component);
	}
}

void wavebox_onmousemove(WaveBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{	
	if (self->sample && self->sample->numframes > 0) {
		if (self->dragmode == SAMPLEBOX_DRAG_LEFT) {		
			self->selectionstart = wavebox_screentoframe(self, ev->x);
			if (self->selectionstart > self->selectionend) {
				wavebox_swapselection(self);
				self->dragmode = SAMPLEBOX_DRAG_RIGHT;
			}
			psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
			psy_ui_component_invalidate(&self->component);
		} else
		if (self->dragmode == SAMPLEBOX_DRAG_RIGHT) {
			self->selectionend = wavebox_screentoframe(self, ev->x);
			if (self->selectionend < self->selectionstart) {
				wavebox_swapselection(self);
				self->dragmode = SAMPLEBOX_DRAG_LEFT;
			}
			psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
			psy_ui_component_invalidate(&self->component);
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
			psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_MOVE);
			psy_ui_component_invalidate(&self->component);
		} else {
			if (self->hasselection && 
				(wavebox_hittest(self, self->selectionstart, ev->x, 5) ||
				 wavebox_hittest(self, self->selectionend, ev->x, 5))) {
				psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_COL_RESIZE);
			} else
			if (wavebox_hittest_range(self, self->selectionstart,
					self->selectionend, ev->x)) {
				psy_ui_component_setcursor(&self->component, psy_ui_CURSORSTYLE_MOVE);
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
		psy_ui_Size size;		
		
		size = psy_ui_component_size(&self->component);			
		frame = (int)(self->offsetstep * x) + 
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
		psy_ui_Size size;		
		
		size = psy_ui_component_size(&self->component);		
		rv = (int)((frame - (intptr_t)(self->sample->numframes *
				self->zoomleft)) / self->offsetstep);
	}
	return rv;
}

void wavebox_onmouseup(WaveBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	if (self->sample && self->sample->numframes > 0) {
		psy_ui_component_releasecapture(&self->component);
		self->dragmode = SAMPLEBOX_DRAG_NONE;
		if (self->selectionstart == self->selectionend) {
			self->hasselection = 0;
			psy_ui_component_invalidate(&self->component);
		}	
	}
}

void wavebox_setzoom(WaveBox* self, float zoomleft, float zoomright)
{
	self->zoomleft = zoomleft;
	self->zoomright = zoomright;
	if (self->sample) {
		psy_ui_Size size;		

		size = psy_ui_component_size(&self->component);
		self->offsetstep = (float)self->sample->numframes / size.width *
			(self->zoomright - self->zoomleft);
		psy_ui_component_invalidate(&self->component);
		psy_ui_component_update(&self->component);
	}
}
