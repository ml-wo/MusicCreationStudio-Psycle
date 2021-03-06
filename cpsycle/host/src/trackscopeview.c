// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "trackscopeview.h"
#include <math.h>
#include <string.h>
#include <uiapp.h>

#include "../../detail/trace.h"
#include "../../detail/portable.h"

#define TIMERID_TRACKSCOPEVIEW 6000

static void trackscopeview_ondraw(TrackScopeView*, psy_ui_Graphics*);
static void trackscopeview_onmousedown(TrackScopeView*, psy_ui_MouseEvent*);
static void trackscopeview_drawtrack(TrackScopeView*, psy_ui_Graphics*,
	int x, int y, int width, int height, int track);
void trackscopeview_drawtrackindex(TrackScopeView*, psy_ui_Graphics*,
	int x, int y, int width, int height, int track);
void trackscopeview_drawtrackmuted(TrackScopeView*, psy_ui_Graphics*, int x,
	int y, int width, int height, int track);
static void trackscopeview_ontimer(TrackScopeView*, psy_ui_Component* sender,
	int timerid);
static void trackscopeview_onalign(TrackScopeView*);
static void trackscopeview_onpreferredsize(TrackScopeView*, psy_ui_Size* limit,
	psy_ui_Size* rv);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(TrackScopeView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onalign = (psy_ui_fp_onalign) trackscopeview_onalign;
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize)
			trackscopeview_onpreferredsize;
		vtable.ondraw = (psy_ui_fp_ondraw) trackscopeview_ondraw;
		vtable.onmousedown = (psy_ui_fp_onmousedown)
			trackscopeview_onmousedown;
	}
}

void trackscopeview_init(TrackScopeView* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_doublebuffer(&self->component);
	psy_ui_component_enablealign(&self->component);
	self->workspace = workspace;
	self->trackheight = 30;
	self->textheight = 12;
	psy_signal_connect(&self->component.signal_timer, self,
		trackscopeview_ontimer);
	psy_ui_component_starttimer(&self->component, TIMERID_TRACKSCOPEVIEW, 50);
}

void trackscopeview_ondraw(TrackScopeView* self, psy_ui_Graphics* g)
{
	if (self->workspace->song) {
		psy_ui_Size size;
		int numtracks = player_numsongtracks(&self->workspace->player);
		int c;
		int maxcolumns = 16;		
		int rows = 1;
		int track = 0;
		int line = 0;		
		int cpx = 0;
		int cpy = 0;
		int width;		
		
		size = psy_ui_component_size(&self->component);
		width = size.width / maxcolumns;
		psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
		psy_ui_settextcolor(g, 0x00444444);
		for (c = 0; c < numtracks; ++c) {
			trackscopeview_drawtrackindex(self, g, cpx, cpy, width,
					self->trackheight, c);
			if (!patternstrackstate_istrackmuted(
					&self->workspace->song->patterns.trackstate, c)) {
				trackscopeview_drawtrack(self, g, cpx, cpy, width,
					self->trackheight, c);
			} else {
				trackscopeview_drawtrackmuted(self, g, cpx, cpy, width,
					self->trackheight, c);
			}
			if (track < maxcolumns - 1) {
				++track;
				cpx += width;
			} else {
				++line;
				track = 0;
				cpx = 0;
				cpy += self->trackheight;			
			}
		}
	}
}

void trackscopeview_drawtrackindex(TrackScopeView* self, psy_ui_Graphics* g, int x, int y,
	int width, int height, int track)
{
	char text[40];
	extern psy_ui_App app;

	psy_ui_setcolor(g, app.defaults.defaultcolor);
	psy_snprintf(text, 40, "%X", track);
	psy_ui_textout(g, x + 3, y, text, strlen(text));
}


void trackscopeview_drawtrack(TrackScopeView* self, psy_ui_Graphics* g, int x, int y,
	int width, int height, int track)
{
	uintptr_t lastmachine;	

	if (psy_table_exists(&self->workspace->player.sequencer.lastmachine, track)) {
		lastmachine = (uintptr_t)
			psy_table_at(&self->workspace->player.sequencer.lastmachine, track);	
	} else {
		lastmachine = NOMACHINE_INDEX;
	}	
	if (lastmachine != NOMACHINE_INDEX) {
		char text[40];

		psy_snprintf(text, 40, "%X", lastmachine);
		psy_ui_textout(g, x + width - 10, y + height - self->textheight, text,
			strlen(text));
	}
	psy_ui_setcolor(g, 0x00888888);
	if (self->workspace->song) {
		psy_audio_Machine* machine;
		int centery;

		centery = height / 2 + y;
		machine = machines_at(&self->workspace->song->machines, lastmachine);
		if (machine) {
			psy_audio_Buffer* memory;
			
			memory = psy_audio_machine_buffermemory(machine);
			if (memory) {
				uintptr_t numsamples;
				uintptr_t frame;
				float px;
				float py;
				float cpx = 0;
				int x1, y1, x2, y2;
				static float epsilon = 0.01f;

				numsamples = psy_audio_machine_buffermemorysize(machine);
				if (numsamples > 0) {
					int zero = 1;

					for (frame = 0; frame < numsamples; ++frame) {
						if (fabs(memory->samples[0][frame]) > epsilon) {
							zero = 0;
							break;
						}
					}
					if (!zero) {
						px = width / (float) numsamples;
						py = height / 32768.f / 3;
						x1 = 0;
						y1 = (int) (memory->samples[0][0] * py);						
						x2 = 0;
						y2 = y1;						
						for (frame = 0; frame < numsamples; ++frame, cpx += px) {
							x1 = x2;
							x2 = (int) (frame * px);
							if (frame == 0 || x1 != x2) {
								y1 = y2;							
								y2 = (int) (memory->samples[0][frame] * py);
								if (y2 > height / 2) {
									y2 = height / 2;
								} else
								if (y2 < -height / 2) {
									y2 = -height / 2;
								}
								psy_ui_drawline(g, x + x1, centery + y1, x + x2,
									centery + y2);
							}
						}
					} else {
						psy_ui_drawline(g, x, centery, x + width, centery);
					}
				}
			} else {
				psy_ui_drawline(g, x, centery, x + width, centery);
			}
		} else {
			psy_ui_drawline(g, x, centery, x + width, centery);
		}
	}	
}

void trackscopeview_drawtrackmuted(TrackScopeView* self, psy_ui_Graphics* g, int x,
	int y, int width, int height, int track)
{	
	int ident = (int)(width * 0.25);
	psy_ui_setcolor(g, app.defaults.defaultcolor);
	psy_ui_moveto(g, psy_ui_point_make(x + ident, y + (int)(height * 0.2)));
	psy_ui_curveto(g,
		psy_ui_point_make(x + width - ident * 2, y + (int)(height * 0.3)),
		psy_ui_point_make(x + width - ident, y + (int)(height * 0.6)),
		psy_ui_point_make(x + width - (int)(ident * 0.5), y + (int)(height * 0.9)));
	psy_ui_moveto(g,
		psy_ui_point_make(x + ident + (int)(width * 0.1), y + (int)(height * 0.8)));
	psy_ui_curveto(g,
		psy_ui_point_make(x + ident + (int)(width * 0.3), y + (int)(height * 0.4)),
		psy_ui_point_make(x + width - ident * 2, y + (int)(height * 0.2)),
		psy_ui_point_make(x + width - (int)(ident * 0.5), (int)(height * 0.25)));
}

void trackscopeview_ontimer(TrackScopeView* self, psy_ui_Component* sender, int timerid)
{
	psy_ui_component_invalidate(&self->component);	
}

void trackscopeview_onalign(TrackScopeView* self)
{
	psy_ui_TextMetric tm;

	tm = psy_ui_component_textmetric(&self->component);
	self->trackheight = (int)(tm.tmHeight * 2.75f);
	self->textheight = tm.tmHeight;
}

void trackscopeview_onpreferredsize(TrackScopeView* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{
	psy_ui_TextMetric tm;	
	int maxcolumns = 16;	
	int numtracks = player_numsongtracks(&self->workspace->player);
	int rows = ((numtracks - 1) / maxcolumns) + 1;

	tm = psy_ui_component_textmetric(&self->component);	
	rv->width = 16 * 30;
	rv->height = (int)(rows * tm.tmHeight * 2.75f);
}

void trackscopeview_onmousedown(TrackScopeView* self, psy_ui_MouseEvent* ev)
{
	if (self->workspace->song) {
		int columns;
		psy_ui_Size size;
		int track;
		int trackwidth;
		int maxcolumns = 16;
		int numtracks = player_numsongtracks(&self->workspace->player);

		columns = numtracks < maxcolumns ? numtracks : maxcolumns;
		size = psy_ui_component_size(&self->component);
		trackwidth = size.width / columns;
		
		track = (ev->x / trackwidth) + (ev->y / self->trackheight) * columns;
		if (ev->button == 1) {
			if (!patternstrackstate_istrackmuted(
					&self->workspace->song->patterns.trackstate, track)) {
				patternstrackstate_mutetrack(
					&self->workspace->song->patterns.trackstate, track);
			} else {
				patternstrackstate_unmutetrack(
					&self->workspace->song->patterns.trackstate, track);
			}
		} else
		if (ev->button == 2) {				
			if (patterns_istracksoloed(&self->workspace->song->patterns,
						track)) {
				patterns_deactivatesolotrack(
					&self->workspace->song->patterns);
			} else {
				patterns_activatesolotrack(
					&self->workspace->song->patterns, track);
			}
			psy_ui_component_invalidate(&self->component);
		}
	}
}

void trackscopeview_start(TrackScopeView* self)
{
	psy_ui_component_starttimer(&self->component, TIMERID_TRACKSCOPEVIEW, 50);
}

void trackscopeview_stop(TrackScopeView* self)
{
	psy_ui_component_stoptimer(&self->component, TIMERID_TRACKSCOPEVIEW);
}
