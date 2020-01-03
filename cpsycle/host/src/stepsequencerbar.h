// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(STEPSEQUENCERBAR_H)
#define STEPSEQUENCERBAR_H

#include <uicomponent.h>
#include "workspace.h"

#include <pattern.h>

typedef struct {
	psy_ui_Component component;
	Workspace* workspace;
	psy_audio_Pattern* pattern;
	psy_dsp_beat_t lastplayposition;
	psy_dsp_beat_t sequenceentryoffset;
} StepsequencerBar;

void stepsequencerbar_init(StepsequencerBar*, psy_ui_Component* parent, Workspace*);

#endif
