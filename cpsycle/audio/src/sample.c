// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "sample.h"
#include "waveio.h"
#include <string.h>
#include <stdlib.h>
#include <operations.h>
#include <alignedalloc.h>

void double_setvalue(Double* self, double value)
{
	self->QuadPart = (uint64_t)(value * 4294967296.0f);
}

void sampleiterator_init(SampleIterator* self, psy_audio_Sample* sample)
{
	self->sample = sample;
	self->forward = 1;
	double_setvalue(&self->pos, 0.0);
	self->speed = (int64_t) 4294967296.0f;	
}

SampleIterator* sampleiterator_alloc(void)
{
	return malloc(sizeof(SampleIterator));
}

SampleIterator* sampleiterator_allocinit(psy_audio_Sample* sample)
{
	SampleIterator* rv;

	rv = sampleiterator_alloc();
	if (rv) {		
		sampleiterator_init(rv, sample);
	}
	return rv;
}

int sampleiterator_inc(SampleIterator* self)
{			
	if (self->sample->looptype == LOOP_DO_NOT) {
		self->pos.QuadPart += self->speed;
		if (self->pos.HighPart >= self->sample->numframes) {
			self->pos.LowPart = 0;
			self->pos.HighPart = 0;			
			return 0;			
		}
	} else
	if (self->sample->looptype == LOOP_NORMAL) {
		self->pos.QuadPart += self->speed;
		if (self->pos.HighPart >= self->sample->loopend) {
			self->pos.HighPart = self->sample->loopstart +
				self->sample->loopend - self->pos.HighPart;
		}
	} else
	if (self->sample->looptype == LOOP_BIDI) {
		if (self->forward) {
			self->pos.QuadPart += self->speed;
			if (self->pos.HighPart >= self->sample->loopend) {
				Double loopend;
				Double delta;
				
				loopend.LowPart = 0;
				loopend.HighPart = self->sample->loopend;
				delta.QuadPart = self->pos.QuadPart - loopend.QuadPart;
				self->pos.QuadPart = loopend.QuadPart - delta.QuadPart;
				self->forward = 0;
			}
		} else {
			// todo check negative values first
			self->pos.QuadPart -= self->speed;
			if (self->pos.HighPart <= self->sample->loopstart) {
				Double loopstart;
				Double delta;
				
				loopstart.LowPart = 0;
				loopstart.HighPart = self->sample->loopstart;
				delta.QuadPart = loopstart.QuadPart - self->pos.QuadPart;
				self->pos.QuadPart = loopstart.QuadPart + delta.QuadPart;
				self->forward = 1;
			}
		}
	}	
	return 1;
}

unsigned int sampleiterator_frameposition(SampleIterator* self)
{
	return self->pos.HighPart;
}

void vibrato_init(Vibrato* self)
{
	self->attack = 0;
	self->depth = 0;
	self->speed = 0;
	self->type = WAVEFORMS_SINUS;
}

void sample_init(psy_audio_Sample* self, uintptr_t numchannels)
{
	psy_audio_buffer_init(&self->channels, numchannels);
	self->stereo = 1;
	self->numframes = 0;
	self->samplerate = 44100;
	self->defaultvolume = 1.f;
	self->globalvolume = 1.f;
	self->loopstart = 0;
	self->loopend = 0;
	self->looptype  = LOOP_DO_NOT;
	self->sustainloopstart = 0;
	self->sustainloopend = 0;
	self->sustainlooptype = LOOP_DO_NOT;
	self->tune = 0;
	self->finetune = 0;
	self->panfactor = 0.5f;
	self->panenabled = 0;
	self->surround = 0;
	self->name = strdup("");
	vibrato_init(&self->vibrato);
}

void sample_dispose(psy_audio_Sample* self)
{
	uintptr_t channel;

	for (channel = 0; channel < self->channels.numchannels; ++channel) {
		dsp.memory_dealloc(self->channels.samples[channel]);
		self->channels.samples[channel] = 0;
	}
	psy_audio_buffer_dispose(&self->channels);
	self->numframes = 0;
	free(self->name);
}

psy_audio_Sample* sample_alloc(void)
{
	return (psy_audio_Sample*) malloc(sizeof(psy_audio_Sample));
}

psy_audio_Sample* sample_allocinit(uintptr_t numchannels)
{
	psy_audio_Sample* rv;

	rv = sample_alloc();
	if (rv) {
		sample_init(rv, numchannels);
	}
	return rv;
}

psy_audio_Sample* sample_clone(psy_audio_Sample* src)
{
	psy_audio_Sample* rv = 0;
	
	rv = sample_alloc();
	if (rv) {
		uintptr_t channel;

		rv->samplerate = src->samplerate;
		rv->defaultvolume = src->defaultvolume;
		rv->globalvolume = src->globalvolume;
		rv->loopstart = src->loopstart;
		rv->loopend = src->loopend;
		rv->looptype = src->looptype;
		rv->sustainloopstart = src->sustainloopstart;
		rv->sustainloopend = src->sustainloopend;
		rv->sustainlooptype = src->sustainlooptype;
		rv->tune = src->tune;
		rv->finetune = src->finetune;
		rv->panfactor = src->panfactor;
		rv->panenabled = src->panenabled;
		rv->surround = src->surround;
		rv->name = strdup(src->name);
		rv->vibrato.attack = src->vibrato.attack;
		rv->vibrato.depth = src->vibrato.depth;
		rv->vibrato.speed = src->vibrato.speed;
		rv->vibrato.type = src->vibrato.type;
		rv->numframes = src->numframes;
		rv->stereo = src->stereo;
		psy_audio_buffer_init(&rv->channels, src->channels.numchannels);
		for (channel = 0; channel < rv->channels.numchannels; ++channel) {
			rv->channels.samples[channel] = dsp.memory_alloc(src->numframes,
				sizeof(float));
		}
		psy_audio_buffer_clearsamples(&rv->channels, src->numframes);
		psy_audio_buffer_addsamples(&rv->channels, &src->channels, src->numframes, 1.0f);
	}
	return rv;
}

void sample_load(psy_audio_Sample* self, const char* path)
{
	char* delim;

	psy_audio_wave_load(self, path);
	delim = strrchr(path, '\\');	
	sample_setname(self, delim ? delim + 1 : path);	
}

void sample_save(psy_audio_Sample* self, const char* path)
{
	psy_audio_wave_save(self, path);
}

void sample_setname(psy_audio_Sample* self, const char* name)
{
	free(self->name);
	self->name = strdup(name);
}

const char* sample_name(psy_audio_Sample* self)
{
	return self->name;
}

SampleIterator sample_begin(psy_audio_Sample* self)
{
	SampleIterator rv;

	sampleiterator_init(&rv, self);
	return rv;
}
