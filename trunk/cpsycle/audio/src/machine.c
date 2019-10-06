// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "machine.h"
#include "pattern.h"
#include <string.h>

static CMachineInfo const macinfo = {
	MI_VERSION,
	0x0250,
	EFFECT | 32 | 64,
	0,
	0,
	"Machine"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Machine",
	"Psycledelics",
	"help",
	3
};

static Machine* clone(Machine* self) { return 0; }
static void work(Machine* self, BufferContext*);
static void generateaudio(Machine* self, BufferContext* bc) { }
static int hostevent(Machine* self, int const eventNr, int const val1, float const val2) { return 0; }
static void seqtick(Machine* self, int channel, const PatternEvent* event) { }
static void sequencertick(Machine* self) { }
static List* sequencerinsert(Machine* self, List* events) { return 0; }
static void sequencerlinetick(Machine* self) { }
static const CMachineInfo* info(Machine* self) { return &macinfo; }
static void parametertweak(Machine* self, int par, int val) { }
static int describevalue(Machine* self, char* txt, int const param, int const value) { return 0; }
static int value(Machine* self, int const param) { return 0; }
static void setvalue(Machine* self, int const param, int const value) { }
static void dispose(Machine* self);
static int mode(Machine* self) { return MACHMODE_FX; }
static unsigned int numinputs(Machine* self) { return 0; }
static unsigned int numoutputs(Machine* self) { return 0; }	
static float pan(Machine* self) { return 0; } 
static void setpan(Machine* self, float val) { };
static void setcallback(Machine* self, MachineCallback callback) { self->callback = callback; }
static void updatesamplerate(Machine* self, unsigned int samplerate) { }

static int dummymachine_mode(DummyMachine* self) { return self->mode; }
static void dummymachine_dispose(DummyMachine* self);
static unsigned int dummymachine_numinputs(DummyMachine* self) { return 2; }
static unsigned int dummymachine_numoutputs(DummyMachine* self) { return 2; }

static unsigned int samplerate(Machine* self) { return self->callback.samplerate(self->callback.context); }
static unsigned int bpm(Machine* self) { return self->callback.bpm(self->callback.context); }
static struct Samples* samples(Machine* self) { return self->callback.samples(self->callback.context); }
static struct Machines* machines(Machine* self) { return self->callback.machines(self->callback.context); }
static struct Instruments* instruments(Machine* self) { return self->callback.instruments(self->callback.context); }

void machine_init(Machine* self, MachineCallback callback)
{		
	memset(self, 0, sizeof(Machine));
	self->clone = clone;
	self->dispose = machine_dispose;
	self->work = work;
	self->mode = mode;
	self->hostevent = hostevent;
	self->seqtick = seqtick;
	self->sequencertick = sequencertick;
	self->sequencerlinetick = sequencerlinetick;
	self->sequencerinsert = sequencerinsert;
	self->info = info;
	self->parametertweak = parametertweak;
	self->describevalue = describevalue;
	self->setvalue = setvalue;
	self->value = value;
	self->generateaudio = generateaudio;
	self->numinputs = numinputs;
	self->numoutputs = numoutputs;
	self->pan = pan;
	self->setpan = setpan;
	self->setcallback = setcallback;
	self->updatesamplerate = updatesamplerate;
	self->bpm = bpm;
	self->samplerate = samplerate;
	self->instruments = instruments;
	self->samples = samples;
	self->machines = machines;
	self->callback = callback;
	signal_init(&self->signal_worked);
}

void machine_dispose(Machine* self)
{
	signal_dispose(&self->signal_worked);
}

void work(Machine* self, BufferContext* bc)
{			
	List* p;
	unsigned int amount = bc->numsamples;
	unsigned int pos = 0;

	for (p = bc->events; p != 0; p = p->next) {					
		int numworksamples;

		PatternEntry* entry = (PatternEntry*)p->entry;		
		numworksamples = (unsigned int)entry->delta - pos;		
		if (numworksamples > 0) {				
			int restorenumsamples = bc->numsamples;
			
			buffer_setoffset(bc->input, pos);
			buffer_setoffset(bc->output, pos);			
			bc->numsamples = numworksamples;
			self->generateaudio(self, bc);
			amount -= numworksamples;
			bc->numsamples = restorenumsamples;
		}
		self->seqtick(self, entry->track, &entry->event);		
		pos = (unsigned int)entry->delta;	
	}
	if (amount > 0 && self->generateaudio) {
		int restorenumsamples = bc->numsamples;
		buffer_setoffset(bc->input, pos);
		buffer_setoffset(bc->output, pos);			
		bc->numsamples = amount;
		self->generateaudio(self, bc);
		bc->numsamples = restorenumsamples;
	}
	buffer_setoffset(bc->input, 0);
	buffer_setoffset(bc->output, 0);			
}

int machine_supports(Machine* self, int option)
{
	if (self->info(self)) {
		return (self->info(self)->Flags & option) == option;
	}
	return 0;
}

void dummymachine_init(DummyMachine* self, MachineCallback callback)
{
	memset(self, 0, sizeof(DummyMachine));
	machine_init(&self->machine, callback);	
	self->machine.mode = dummymachine_mode;
	self->machine.dispose = dummymachine_dispose;
	self->machine.numinputs = dummymachine_numinputs;
	self->machine.numoutputs = dummymachine_numoutputs;

	self->mode = MACHMODE_FX;
}

void dummymachine_dispose(DummyMachine* self)
{	
	machine_dispose(&self->machine);
}

