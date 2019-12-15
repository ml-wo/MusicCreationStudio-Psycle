// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(DSP_NOTESTAB_H)
#define DSP_NOTESTAB_H

typedef unsigned char note_t;

typedef enum {
	psy_dsp_NOTESTAB_A440,
	psy_dsp_NOTESTAB_A220	
} psy_dsp_NotesTabMode;

#define psy_dsp_NOTESTAB_DEFAULT psy_dsp_NOTESTAB_A220

const char* psy_dsp_notetostr(note_t note, psy_dsp_NotesTabMode mode);
const char* const * psy_dsp_notetab(psy_dsp_NotesTabMode mode);

#endif