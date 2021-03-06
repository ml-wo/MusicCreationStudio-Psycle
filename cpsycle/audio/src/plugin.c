// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "plugin.h"
#include "pattern.h"
#include "plugin_interface.h"
#include "songio.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../../detail/portable.h"

typedef CMachineInfo * (*GETINFO)(void);
typedef CMachineInterface * (*CREATEMACHINE)(void);

static psy_audio_Machine* clone(psy_audio_Plugin*);
static int hostevent(psy_audio_Plugin*, int const eventNr, int val1, float val2);
static void generateaudio(psy_audio_Plugin*, psy_audio_BufferContext*);
static void seqtick(psy_audio_Plugin*, uintptr_t channel,
	const psy_audio_PatternEvent*);
static void stop(psy_audio_Plugin*);
static void sequencerlinetick(psy_audio_Plugin*);
static psy_audio_MachineInfo* info(psy_audio_Plugin*);
static int parametertype(psy_audio_Plugin* self, uintptr_t param);
static unsigned int numparametercols(psy_audio_Plugin*);
static uintptr_t numparameters(psy_audio_Plugin*);
static void parameterrange(psy_audio_Plugin*, uintptr_t param, int* minval,
	int* maxval);
static int parameterlabel(psy_audio_Plugin*, char* txt, uintptr_t param);
static int parametername(psy_audio_Plugin*, char* txt, uintptr_t param);
static void parametertweak(psy_audio_Plugin*, uintptr_t param, float val);
static int describevalue(psy_audio_Plugin*, char* txt, uintptr_t param,
	int value);
static float parametervalue(psy_audio_Plugin*, uintptr_t param);
static void dispose(psy_audio_Plugin*);
static uintptr_t numinputs(psy_audio_Plugin*);
static uintptr_t numoutputs(psy_audio_Plugin*);
static void loadspecific(psy_audio_Plugin*, psy_audio_SongFile*,
	uintptr_t slot);
static void savespecific(psy_audio_Plugin*, psy_audio_SongFile*,
	uintptr_t slot);
static void setcallback(psy_audio_Plugin*, MachineCallback);

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_audio_Plugin* self)
{
	if (!vtable_initialized) {
		vtable = *(psy_audio_plugin_base(self)->vtable);
		vtable.clone = (fp_machine_clone) clone;
		vtable.hostevent = (fp_machine_hostevent) hostevent;
		vtable.seqtick = (fp_machine_seqtick) seqtick;
		vtable.stop = (fp_machine_stop) stop;
		vtable.sequencerlinetick = (fp_machine_sequencerlinetick)
			sequencerlinetick;
		vtable.info = (fp_machine_info) info;
		vtable.numparametercols = (fp_machine_numparametercols)
			numparametercols;
		vtable.numparameters = (fp_machine_numparameters) numparameters;
		vtable.parameterrange = (fp_machine_parameterrange) parameterrange;
		vtable.parametertype = (fp_machine_parametertype) parametertype;
		vtable.parameterlabel = (fp_machine_parameterlabel) parameterlabel;
		vtable.parametername = (fp_machine_parametername) parametername;		
		vtable.parametertweak = (fp_machine_parametertweak) parametertweak;				
		vtable.describevalue = (fp_machine_describevalue) describevalue;
		vtable.parametervalue = (fp_machine_parametervalue) parametervalue;
		vtable.dispose = (fp_machine_dispose) dispose;
		vtable.generateaudio = (fp_machine_generateaudio) generateaudio;
		vtable.numinputs = (fp_machine_numinputs) numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) numoutputs;
		vtable.loadspecific = (fp_machine_loadspecific) loadspecific;
		vtable.savespecific = (fp_machine_savespecific) savespecific;
		vtable.setcallback = (fp_machine_setcallback) setcallback;	
		vtable_initialized = 1;
	}
}

void psy_audio_plugin_init(psy_audio_Plugin* self, MachineCallback callback,
	const char* path)
{
	GETINFO GetInfo;
			
	custommachine_init(&self->custommachine, callback);	
	vtable_init(self);
	psy_audio_plugin_base(self)->vtable = &vtable;
	psy_library_init(&self->library);
	psy_library_load(&self->library, path);			
	self->mi = 0;
	self->plugininfo = 0;
	self->preventsequencerlinetick = 0;
	
	GetInfo = (GETINFO)psy_library_functionpointer(&self->library, "GetInfo");
	if (!GetInfo) {
		psy_library_dispose(&self->library);		
	} else {		
		self->mi = mi_create(self->library.module);		
		if (!self->mi) {
			psy_library_dispose(&self->library);			
		} else {						
			CMachineInfo* pInfo = GetInfo();
			if (pInfo) {
				int gbp;	
				mi_resetcallback(self->mi);
				mi_setcallback(self->mi, &callback);
				mi_init(self->mi);
				for (gbp = 0; gbp < pInfo->numParameters; gbp++) {
					mi_parametertweak(self->mi, gbp, 
						pInfo->Parameters[gbp]->DefValue);
				}
				self->plugininfo = machineinfo_allocinit();				
				machineinfo_setnativeinfo(self->plugininfo, pInfo, MACH_PLUGIN,
					self->library.path, 0);				
				psy_audio_machine_seteditname(psy_audio_plugin_base(self),
					pInfo->ShortName);
				if (strcmp(self->plugininfo->ShortName, "BexPhase!") == 0) {
					self->preventsequencerlinetick = 1;
				}
			}
		}
	}
	if (!psy_audio_machine_editname(psy_audio_plugin_base(self))) {
		psy_audio_machine_seteditname(psy_audio_plugin_base(self), "Plugin");
	}
}

void dispose(psy_audio_Plugin* self)
{		
	if (self->library.module != 0 && self->mi) {
		mi_dispose(self->mi);
		psy_library_dispose(&self->library);
		self->mi = 0;		
	}			
	if (self->plugininfo) {
		machineinfo_dispose(self->plugininfo);
		free(self->plugininfo);
		self->plugininfo = 0;
	}	
	custommachine_dispose(&self->custommachine);
}

psy_audio_Machine* clone(psy_audio_Plugin* self)
{
	psy_audio_Plugin* rv;

	rv = malloc(sizeof(psy_audio_Plugin));
	if (rv) {
		psy_audio_plugin_init(rv, self->custommachine.machine.callback,
			self->library.path);
	}
	return rv ? &rv->custommachine.machine : 0;
}

int psy_audio_plugin_psycle_test(const char* path, psy_audio_MachineInfo* info)
{	
	int rv = 0;

	if (path && strcmp(path, "") != 0) {
		GETINFO GetInfo;
		psy_Library library;

		psy_library_init(&library);
		psy_library_load(&library, path);					
		GetInfo =(GETINFO)psy_library_functionpointer(&library, "GetInfo");
		if (GetInfo != NULL) {	
			CMachineInfo* nativeinfo = GetInfo();
			if (nativeinfo) {				
				machineinfo_setnativeinfo(info, nativeinfo, MACH_PLUGIN,
					library.path, 0);
				rv = 1;
			}
		}
		psy_library_dispose(&library);
	}
	return rv;	
}

void seqtick(psy_audio_Plugin* self, uintptr_t channel,
	const psy_audio_PatternEvent* ev)
{	
	if (patternevent_has_volume(ev)) {
		mi_seqtick(self->mi, channel, ev->note, ev->inst & 0xFF, 0x0C, ev->vol);
		if (ev->parameter != 0 || ev->cmd != 0) {
			mi_seqtick(self->mi, channel, NOTECOMMANDS_EMPTY,
				NOTECOMMANDS_EMPTY, ev->cmd, ev->parameter);
		}
	} else
	if (ev->note == NOTECOMMANDS_MIDICC && ev->inst >= 0x80 && ev->inst < 0xFF) {		
		mi_midievent(self->mi, ev->inst & 0x0F, ev->inst & 0xF0,
			(ev->cmd << 8) + ev->parameter);			
	} else {
		mi_seqtick(self->mi, channel, ev->note, ev->inst & 0xFF, ev->cmd,
			ev->parameter);
	}
}

void stop(psy_audio_Plugin* self)
{
	mi_stop(self->mi);
}

void generateaudio(psy_audio_Plugin* self, psy_audio_BufferContext* bc)
{	
	mi_work(self->mi,
		psy_audio_buffer_at(bc->output, 0),
		psy_audio_buffer_at(bc->output, 1),
		bc->numsamples, bc->numtracks);
}

int hostevent(psy_audio_Plugin* self, int const eventNr, int val1, float val2)
{
	return mi_hostevent(self->mi, eventNr, val1, val2);
}

void sequencerlinetick(psy_audio_Plugin* self)
{	
	// prevent bexphase from crash
	if (!self->preventsequencerlinetick) {
		mi_sequencertick(self->mi);	
	}	
}

psy_audio_MachineInfo* info(psy_audio_Plugin* self)
{		
	return self->plugininfo;
}

uintptr_t numinputs(psy_audio_Plugin* self)
{
	return info(self) ? (self->plugininfo->mode == MACHMODE_FX ? 2 : 0) : 0;
}

uintptr_t numoutputs(psy_audio_Plugin* self)
{
	return info(self) ? 2 : 0;
}

void setcallback(psy_audio_Plugin* self, MachineCallback callback)
{
	if (self->mi) {				
		mi_setcallback(self->mi, &callback);
	}
}

void loadspecific(psy_audio_Plugin* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{	
	uint32_t size;

	// size of whole structure
	psyfile_read(songfile->file, &size, sizeof(size));
	if(size) {
		uint32_t numparams;
		uint32_t i;

		// size of vars
		psyfile_read(songfile->file, &numparams, sizeof(numparams));
		for (i = 0; i < numparams; ++i) {
			int temp;
			
			psyfile_read(songfile->file, &temp, sizeof(temp));			
			psy_audio_machine_parametertweak(psy_audio_plugin_base(self), i,
				machine_parametervalue_normed(psy_audio_plugin_base(self), i, temp));			
		}
		size -= sizeof(numparams) + sizeof(int) * numparams;
		if(size) {
			unsigned int size2 = 0;
			unsigned char* pData;
			
			size2 = mi_getdatasize(self->mi);
			// This way we guarantee that the plugin will have enough bytes,
			// even if it does not fit what it reads.
			pData = (unsigned char*)malloc(max(size,size2));
			psyfile_read(songfile->file, pData, size); // Read internal data.			
			mi_putdata(self->mi, pData); // Internal load
			free(pData);
		}						
	}	
}			

void savespecific(psy_audio_Plugin* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t count = psy_audio_machine_numparameters(psy_audio_plugin_base(self));
	uint32_t size2 = 0;
	uint32_t size;
	uint32_t i;

	size2 = mi_getdatasize(self->mi);
	size = size2 + sizeof(count) + sizeof(int) * count;
	psyfile_write(songfile->file, &size, sizeof(size));
	psyfile_write(songfile->file, &count, sizeof(count));
	for (i = 0; i < count; ++i) {
		int32_t temp;
		
		temp = machine_parametervalue_scaled(psy_audio_plugin_base(self), 
			i, psy_audio_machine_parametervalue(psy_audio_plugin_base(self), i));
		psyfile_write(songfile->file, &temp, sizeof temp);
	}
	if (size2) {
		unsigned char* pData;

		pData = malloc(size2);
		mi_getdata(self->mi, pData);
		psyfile_write(songfile->file, pData, size2); //data chunk
		free(pData);
		pData = 0;
	}
}

void parametertweak(psy_audio_Plugin* self, uintptr_t param, float val)
{
	if (param < numparameters(self)) {
		mi_parametertweak(self->mi, (int)param,
			machine_parametervalue_scaled(psy_audio_plugin_base(self), param,
				val));
	}
}

int describevalue(psy_audio_Plugin* self, char* txt, uintptr_t param,
	int value)
{
	return mi_describevalue(self->mi, txt, (int)param, value);
}

float parametervalue(psy_audio_Plugin* self, uintptr_t param)
{
	return machine_parametervalue_normed(psy_audio_plugin_base(self), param,
		mi_val(self->mi, (int)param));
}

int parametertype(psy_audio_Plugin* self, uintptr_t param)
{
	int rv = MPF_STATE;
	GETINFO GetInfo;

	GetInfo =(GETINFO)psy_library_functionpointer(&self->library, "GetInfo");
	if (GetInfo != NULL) {	
		CMachineInfo* cinfo = GetInfo();
		if (cinfo) {	
			if (param < (uintptr_t) cinfo->numParameters) {
				rv = cinfo->Parameters[param]->Flags;
			}
		}
	}
	return rv;
}

void parameterrange(psy_audio_Plugin* self, uintptr_t param, int* minval,
	int* maxval)
{	
	GETINFO GetInfo;

	*minval = 0;
	*maxval = 0;
	GetInfo =(GETINFO)psy_library_functionpointer(&self->library, "GetInfo");
	if (GetInfo != NULL) {	
		CMachineInfo* cinfo = GetInfo();
		if (cinfo) {	
			if (param < (uintptr_t) cinfo->numParameters) {
				*minval = cinfo->Parameters[param]->MinValue;
				*maxval = cinfo->Parameters[param]->MaxValue;				
			}
		}
	}	
}

int parameterlabel(psy_audio_Plugin* self, char* txt, uintptr_t param)
{
	int rv = 0;
	GETINFO GetInfo;

	GetInfo =(GETINFO) psy_library_functionpointer(&self->library, "GetInfo");
	if (GetInfo != NULL) {	
		CMachineInfo* cinfo = GetInfo();
		if (cinfo) {	
			if (param < (uintptr_t) cinfo->numParameters) {
				psy_snprintf(txt, 128, "%s",
					cinfo->Parameters[param]->Description);
				rv = 1;
			}
		}
	}
	return rv;
}

int parametername(psy_audio_Plugin* self, char* txt, uintptr_t param)
{
	int rv = 0;	
	GETINFO GetInfo;

	GetInfo =(GETINFO) psy_library_functionpointer(&self->library, "GetInfo");
	if (GetInfo != NULL) {	
		CMachineInfo* info = GetInfo();
		if (info) {	
			if (param < (uintptr_t) info->numParameters) {
				psy_snprintf(txt, 128, "%s", info->Parameters[param]->Name);
				rv = 1;
			}
		}
	}
	return rv;
}

unsigned int numparametercols(psy_audio_Plugin* self)
{
	int rv = 0;
	GETINFO GetInfo;

	GetInfo =(GETINFO)psy_library_functionpointer(&self->library, "GetInfo");
	if (GetInfo != NULL) {	
		CMachineInfo* info = GetInfo();
		if (info) {	
			rv = info->numparametercols;				
		}
	}
	return rv;
}

uintptr_t numparameters(psy_audio_Plugin* self)
{
	uintptr_t rv = 0;
	GETINFO GetInfo;

	GetInfo =(GETINFO)psy_library_functionpointer(&self->library, "GetInfo");
	if (info != NULL) {	
		CMachineInfo* info = GetInfo();
		if (info) {	
			rv = info->numParameters > 0 ? (uintptr_t) info->numParameters : 0;
		}
	}
	return rv;
}
