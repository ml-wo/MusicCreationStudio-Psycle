// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "player.h"
#include "exclusivelock.h"
#include "math.h"
#include "master.h"
#include "plugin.h"
#include "vstplugin.h"
#include "silentdriver.h"
#include <operations.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE 1
#endif

static float bufferdriver[65535];
static void* mainframe;

static float* Work(Player*, int* numsamples);
static void player_advance(Player*, unsigned int amount);
static void player_workpath(Player*, unsigned int amount);
static List* player_timedevents(Player*, unsigned int slot, unsigned int amount);
static Buffer* player_mix(Player*, unsigned int slot, unsigned int amount);
static void player_filldriver(Player*, float* buffer, unsigned int amount);
static void player_signal_wait_host(Player*);
static void Interleave(float* dst, float* left, float* right, int num);
static beat_t Offset(Player*, int numsamples);
static int onenumsequencertick(Player*, int slot, Machine*);

void player_init(Player* self, Song* song, void* handle)
{		
	mainframe = handle;
	self->song = song;		
	self->playing = FALSE;
	self->lpb = 4;
	self->seqtickcount = 1.f / self->lpb;
	self->numsongtracks = 16;	
	sequencer_init(&self->sequencer, &song->sequence);	
	library_init(&self->drivermodule);
	self->silentdriver = create_silent_driver();
	self->silentdriver->init(self->silentdriver);	
	self->driver = self->silentdriver;		
	signal_init(&self->signal_numsongtrackschanged);
	signal_init(&self->signal_lpbchanged);
	lock_init();	
}

void player_loaddriver(Player* self, const char* path)
{
	Driver* driver = 0;
	if (path) {
		library_load(&self->drivermodule, path);	
		if (self->drivermodule.module) {
			pfndriver_create fpdrivercreate;
			fpdrivercreate = (pfndriver_create)
				library_functionpointer(&self->drivermodule, "driver_create");
			if (fpdrivercreate) {
				driver = fpdrivercreate();		
				driver->init(driver);
				driver->connect(driver, self, Work, mainframe);									
				self->t = 125 / (driver->samplerate(driver) * 60.0f);
				self->sequencer.samplerate = driver->samplerate(driver);
			} else {
				self->t = 125 / (44100 * 60.0f);
				self->sequencer.samplerate = 44100;
			}
			lock_enable();
		}
		if (!driver) {
			driver = self->silentdriver;
			lock_disable();
		}
		self->driver = driver;	
	} else {
		self->driver = self->silentdriver;
		lock_disable();
	}
}

void player_unloaddriver(Player* self)
{
	if (self->driver && self->driver != self->silentdriver &&
			self->drivermodule.module) {
		self->driver->dispose(self->driver);
		self->driver->free(self->driver);
		library_dispose(&self->drivermodule);
		library_init(&self->drivermodule);
	}
}

void player_reloaddriver(Player* self, const char* path)
{	
	self->driver->close(self->driver);
	Sleep(400);	
	player_unloaddriver(self);
	player_loaddriver(self, path);	
	self->driver->open(self->driver);
}

void player_restartdriver(Player* self)
{
	if (self->driver != self->silentdriver) {			
		self->driver->close(self->driver);
		Sleep(400);
		self->driver->updateconfiguration(self->driver);
		self->driver->open(self->driver);		
	}
}

void player_dispose(Player* self)
{		
	if (self->driver != self->silentdriver) {			
		self->driver->close(self->driver);
		Sleep(400);
		player_unloaddriver(self);
		lock_dispose();
	}	
	self->silentdriver->dispose(self->silentdriver);
	self->silentdriver->free(self->silentdriver);
	self->silentdriver = 0;
	signal_dispose(&self->signal_lpbchanged);
	sequencer_dispose(&self->sequencer);
}

void player_setsong(Player* self, Song* song)
{
	self->song = song;
	sequencer_reset(&self->sequencer, &song->sequence);
}

void player_start(Player* self)
{	
	sequencer_setposition(&self->sequencer, 0.0f);
	self->playing = TRUE;
	self->seqtickcount = 1.f / self->lpb;
}

void player_stop(Player* self)
{
	self->playing = FALSE;
}

beat_t player_position(Player* self)
{
	return self->sequencer.position;
}

void player_setbpm(Player* self, beat_t bpm)
{
	if (bpm < 32) {
		self->sequencer.bpm = 32;
	} else
	if (bpm > 999) {
		self->sequencer.bpm = 999;
	} else {
		self->sequencer.bpm = bpm;
	}
	self->t = self->sequencer.bpm / (44100 * 60.0f);
}

beat_t player_bpm(Player* self)
{
	return self->sequencer.bpm;
}

void player_setlpb(Player* self, unsigned int lpb)
{
	self->lpb = lpb;
	signal_emit(&self->signal_lpbchanged, self, 1, lpb);
}

unsigned int player_lpb(Player* self)
{
	return self->lpb;
}

beat_t Offset(Player* self, int numsamples)
{
	return numsamples * self->t;
}

unsigned int Frames(Player* self, beat_t offset)
{
	return (unsigned int)(offset / self->t);
}

real* Work(Player* self, int* numsamples)
{		
	unsigned int amount;
	unsigned int numsamplex;
	float* psamples;
	
	psamples = bufferdriver;
	numsamplex = *numsamples;	
	amount = numsamplex > MAX_STREAM_SIZE ? MAX_STREAM_SIZE : numsamplex;
	do {				
		player_advance(self, amount);		
		player_workpath(self, amount);
		player_filldriver(self, psamples, amount);
		numsamplex -= amount;		
		psamples  += (2*amount);
	}  while (numsamplex > 0);
	signalwaithost();	
	return bufferdriver;
}

void player_advance(Player* self, unsigned int amount)
{
	beat_t offset;

	offset = Offset(self, amount);
	if (self->playing) {		
		sequencer_tick(&self->sequencer, offset);
	}
	self->seqtickcount -= offset;
	if (self->seqtickcount <= 0) {		
		machines_enumerate(&self->song->machines, self, onenumsequencertick);		
		self->seqtickcount = 0;
	}
}

int onenumsequencertick(Player* machines, int slot, Machine* machine)
{
	machine->sequencertick(machine);	
	return 1;
}

void player_workpath(Player* self, unsigned int amount)
{
	MachinePath* path;
	path = machines_path(&self->song->machines);
	if (path) {
		for ( ; path != 0; path = path->next) {
			unsigned int slot;						
			Buffer* output;

			slot = (int)path->entry;				
			output = player_mix(self, slot, amount);								
			if (slot != MASTER_INDEX) {				
				Machine* machine;
				BufferContext bc;

				machine = machines_at(&self->song->machines, slot);
				if (machine && output) {
					List* events;

					events = player_timedevents(self, slot, amount);												
					buffercontext_init(&bc, events, output, output, amount,
						self->numsongtracks);
					machine->work(machine, &bc);						
					signal_emit(&machine->signal_worked, machine, 1, &bc);
					list_free(events);
				}									
			}			
		}							
	}		
}


List* player_timedevents(Player* self, unsigned int slot, unsigned int amount)
{
	List* events = 0;

	if (self->playing) {												
		List* node = self->sequencer.events;
		while (node) {
			PatternEntry* entry = (PatternEntry*)node->entry;
			if (entry->event.mach == slot) {
				unsigned int deltaframes;

				if (!events) {
					events = list_create(entry);
				} else {
					list_append(events, entry);
				}
				deltaframes = Frames(self, entry->delta);
				if (deltaframes >= amount) {
					deltaframes = amount - 1;
				}
				entry->delta = (beat_t) deltaframes;
			}
			node = node->next;
		}						
	}
	return events;
}

Buffer* player_mix(Player* self, unsigned int slot, unsigned int amount)
{
	MachineConnections* connections;				
	Buffer* output;
	
	connections = machines_connections(&self->song->machines, slot);
	output = machines_outputs(&self->song->machines, slot);
	if (output) {
		buffer_clearsamples(output, amount);
		if (connections) {
			MachineConnection* connection;
			
			for (connection = connections->inputs; connection != 0;
					connection = connection->next) {
				MachineConnectionEntry* source = 
					(MachineConnectionEntry*)connection->entry;
				if (source->slot != -1) {							
					buffer_addsamples(
						output, 
						machines_outputs(&self->song->machines,
							source->slot),
						amount,
						1.0f);
				}						
			}								
		}
	}
	return output;
}

void player_filldriver(Player* self, float* buffer, unsigned int amount)
{
	Buffer* masteroutput;
	masteroutput = machines_outputs(&self->song->machines, MASTER_INDEX);
	if (masteroutput) {		
		Machine* master;

		master = machines_master(&self->song->machines);
		if (master) {
			BufferContext bc;
					
			buffercontext_init(&bc, 0, masteroutput, masteroutput, amount,
				self->numsongtracks);	
			buffer_mulsamples(masteroutput, amount,
				machines_volume(&self->song->machines));
			signal_emit(&master->signal_worked, master, 1, &bc);
		}
		dsp_interleave(buffer, masteroutput->samples[0],
			masteroutput->samples[1], amount);
	}
}

void player_setnumsongtracks(Player* self, unsigned int numsongtracks)
{
	if (numsongtracks >= 1 && numsongtracks <= 64) {
		self->numsongtracks = numsongtracks;	
		signal_emit(&self->signal_numsongtrackschanged, self, 1,
			self->numsongtracks);
	}
}

unsigned int player_numsongtracks(Player* self)
{
	return self->numsongtracks;
}




