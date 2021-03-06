// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(SAMPLESVIEW_H)
#define SAMPLESVIEW_H

#include <uinotebook.h>
#include <uibutton.h>
#include <uicombobox.h>
#include <uiedit.h>
#include <uilabel.h>
#include "samplesbox.h"
#include "sampleeditor.h"
#include <uislider.h>
#include "wavebox.h"
#include "tabbar.h"
#include "notestab.h"
#include "workspace.h"

typedef struct {
	psy_ui_Component component;
	struct SamplesView* view;
	psy_audio_Sample* sample;
	psy_audio_Instruments* instruments;	
	psy_ui_Label namelabel;
	psy_ui_Edit nameedit;
	psy_ui_Button prevbutton;
	psy_ui_Button nextbutton;
	psy_ui_Label srlabel;
	psy_ui_Edit sredit;
	psy_ui_Label numsamplesheaderlabel;
	psy_ui_Label numsampleslabel;
	psy_ui_Label channellabel;
} SamplesHeaderView;

typedef struct {
	psy_ui_Component component;
	psy_audio_Sample* sample;
	psy_dsp_NotesTabMode notestabmode;
	psy_ui_Slider defaultvolume;
	psy_ui_Slider globalvolume;
	psy_ui_Slider panposition;
	psy_ui_Slider samplednote; 
	psy_ui_Slider pitchfinetune;
} SamplesGeneralView;

typedef struct {
	psy_ui_Component component;
	psy_audio_Sample* sample;	
	psy_ui_Component header;
	psy_ui_Label waveformheaderlabel;
	psy_ui_ComboBox waveformbox;
	psy_ui_Slider attack;
	psy_ui_Slider speed;
	psy_ui_Slider depth;
	psy_audio_Player* player;
} SamplesVibratoView;

typedef struct {
	psy_ui_Component component;
	struct SamplesView* view;
	psy_audio_Sample* sample;	
	psy_ui_Component cont;
	psy_ui_Label loopheaderlabel;
	psy_ui_ComboBox loopdir;
	psy_ui_Label loopstartlabel;
	psy_ui_Edit loopstartedit;
	psy_ui_Label loopendlabel;
	psy_ui_Edit loopendedit;
	psy_ui_Component sustain;
	psy_ui_Label sustainloopheaderlabel;
	psy_ui_ComboBox sustainloopdir;
	psy_ui_Label sustainloopstartlabel;
	psy_ui_Edit sustainloopstartedit;
	psy_ui_Label sustainloopendlabel;
	psy_ui_Edit sustainloopendedit;		
} SamplesLoopView;

typedef struct {
	psy_ui_Component component;
	psy_ui_Button load;
	psy_ui_Button save;
	psy_ui_Button duplicate;
	psy_ui_Button del;
} SamplesViewButtons;

void samplesviewbuttons_init(SamplesViewButtons*, psy_ui_Component* parent);

typedef struct {
	psy_ui_Component component;
	psy_ui_Component header;
	psy_ui_Label label;
	psy_ui_Label songname;
	psy_ui_Button browse;
	SamplesBox samplesbox;
	psy_ui_Component bar;
	psy_ui_Button add;
	WaveBox samplebox;
	psy_audio_Song* source;
	struct SamplesView* view;
	Workspace* workspace;
} SamplesSongImportView;

typedef struct SamplesView {
	psy_ui_Component component;	
	TabBar clienttabbar;
	psy_ui_Notebook clientnotebook;
	psy_ui_Component mainview;
	psy_ui_Component importview;
	psy_ui_Notebook notebook;
	SamplesBox samplesbox;
	psy_ui_Component left;
	SamplesViewButtons buttons;	
	psy_ui_Component client;
	SamplesSongImportView songimport;
	SampleEditor sampleeditor;
	SamplesHeaderView header;
	TabBar tabbar;
	SamplesGeneralView general;
	SamplesVibratoView vibrato;
	psy_ui_Component loop;
	SamplesLoopView waveloop;	
	WaveBox wavebox;	
	Workspace* workspace;
} SamplesView;

void samplesview_init(SamplesView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace);

#endif
