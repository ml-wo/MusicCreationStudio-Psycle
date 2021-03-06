// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "instrumentsbox.h"
#include <stdio.h>
#include "../../detail/portable.h"

static void BuildInstrumentList(InstrumentsBox* self);
static void AddString(InstrumentsBox* self, const char* text);
static void OnInstrumentSlotChanged(InstrumentsBox* self, psy_audio_Instrument* sender, int slot);
static void OnInstrumentInsert(InstrumentsBox* self, psy_ui_Component* sender, int slot);
static void OnInstrumentRemoved(InstrumentsBox* self, psy_ui_Component* sender, int slot);
static void OnInstrumentListChanged(InstrumentsBox* self, psy_ui_Component* sender,
	int slot);

void instrumentsbox_init(InstrumentsBox* self, psy_ui_Component* parent,
	psy_audio_Instruments* instruments)
{	
	psy_ui_listbox_init(&self->instrumentlist, parent);	
	instrumentsbox_setinstruments(self, instruments);	
	psy_signal_connect(&self->instrumentlist.signal_selchanged, self,
		OnInstrumentListChanged);
}

void BuildInstrumentList(InstrumentsBox* self)
{
	psy_audio_Instrument* instrument;
	int slot = 0;
	char buffer[20];

	psy_ui_listbox_clear(&self->instrumentlist);
	for ( ; slot < 256; ++slot) {		
		if (instrument = instruments_at(self->instruments, slot)) {
			psy_snprintf(buffer, 20, "%02X:%s", slot, instrument_name(instrument));
		} else {
			psy_snprintf(buffer, 20, "%02X:%s", slot, "");
		}
		AddString(self, buffer);
	}
}

void AddString(InstrumentsBox* self, const char* text)
{
	psy_ui_listbox_addtext(&self->instrumentlist, text);
}

void OnInstrumentListChanged(InstrumentsBox* self, psy_ui_Component* sender, int slot)
{
	instruments_changeslot(self->instruments, slot);
}

void OnInstrumentInsert(InstrumentsBox* self, psy_ui_Component* sender, int slot)
{
	BuildInstrumentList(self);
	psy_ui_listbox_setcursel(&self->instrumentlist, slot);		
}

void OnInstrumentRemoved(InstrumentsBox* self, psy_ui_Component* sender, int slot)
{
	BuildInstrumentList(self);
	psy_ui_listbox_setcursel(&self->instrumentlist, slot);		
}

void OnInstrumentSlotChanged(InstrumentsBox* self, psy_audio_Instrument* sender, int slot)
{
	psy_ui_listbox_setcursel(&self->instrumentlist, slot);	
}

void instrumentsbox_setinstruments(InstrumentsBox* self, psy_audio_Instruments* instruments)
{
	self->instruments = instruments;	
	BuildInstrumentList(self);
	psy_signal_connect(&instruments->signal_insert, self,
		OnInstrumentInsert);
	psy_signal_connect(&instruments->signal_removed, self,
		OnInstrumentRemoved);
	psy_signal_connect(&instruments->signal_slotchange, self,
		OnInstrumentSlotChanged);
}

int instrumentsbox_selected(InstrumentsBox* self)
{
	return psy_ui_listbox_cursel(&self->instrumentlist);	
}
