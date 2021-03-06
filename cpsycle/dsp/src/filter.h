// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_dsp_FILTER_H
#define psy_dsp_FILTER_H

#include "filtercoeff.h"
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psy_dsp_Filter;

// FilterVtable function pointer typedefs
typedef void (*psy_dsp_fp_filter_init)(struct psy_dsp_Filter*);
typedef void (*psy_dsp_fp_filter_dispose)(struct psy_dsp_Filter*);
typedef psy_dsp_amp_t (*psy_dsp_fp_filter_work)(struct psy_dsp_Filter*, psy_dsp_amp_t sample);
typedef void (*psy_dsp_fp_filter_setcutoff)(struct psy_dsp_Filter*, float cutoff);	
typedef float (*psy_dsp_fp_filter_cutoff)(struct psy_dsp_Filter*);
typedef void (*psy_dsp_fp_filter_setressonance)(struct psy_dsp_Filter*, float ressonance);
typedef float (*psy_dsp_fp_filter_ressonance)(struct psy_dsp_Filter*);
typedef void (*psy_dsp_fp_filter_setsamplerate)(struct psy_dsp_Filter*, float samplerate);
typedef float (*psy_dsp_fp_filter_samplerate)(struct psy_dsp_Filter*);
typedef void (*psy_dsp_fp_filter_update)(struct psy_dsp_Filter*, int full);
typedef void (*psy_dsp_fp_filter_reset)(struct psy_dsp_Filter*);

typedef struct filter_vtable {
	psy_dsp_fp_filter_init init;
	psy_dsp_fp_filter_dispose dispose;
	psy_dsp_fp_filter_work work;
	psy_dsp_fp_filter_setcutoff setcutoff;
	psy_dsp_fp_filter_cutoff cutoff;
	psy_dsp_fp_filter_setressonance setressonance;
	psy_dsp_fp_filter_ressonance ressonance;
	psy_dsp_fp_filter_setsamplerate setsamplerate;
	psy_dsp_fp_filter_samplerate samplerate;
	psy_dsp_fp_filter_update update;
	psy_dsp_fp_filter_reset reset;
} filter_vtable;

typedef struct psy_dsp_Filter {
	struct filter_vtable* vtable;
} psy_dsp_Filter;

void psy_dsp_filter_init(psy_dsp_Filter*);

// vtable calls
INLINE void psy_dsp_filter_dispose(psy_dsp_Filter* self)
{
	self->vtable->dispose(self);
}

INLINE psy_dsp_amp_t psy_dsp_filter_work(psy_dsp_Filter* self, psy_dsp_amp_t
	sample)
{
	return self->vtable->work(self, sample);
}

INLINE void psy_dsp_filter_setcutoff(psy_dsp_Filter* self, float cutoff)
{
	self->vtable->setcutoff(self, cutoff);
}

INLINE float psy_dsp_filter_cutoff(psy_dsp_Filter* self)
{
	return self->vtable->cutoff(self);
}

INLINE void psy_dsp_filter_setressonance(psy_dsp_Filter* self,
	float ressonance)
{
	self->vtable->setressonance(self, ressonance);
}

INLINE float psy_dsp_filter_ressonance(psy_dsp_Filter* self)
{
	return self->vtable->ressonance(self);
}

INLINE void psy_dsp_filter_setsamplerate(psy_dsp_Filter* self,
	float samplerate)
{
	self->vtable->setsamplerate(self, samplerate);
}

INLINE float psy_dsp_filter_samplerate(psy_dsp_Filter* self)
{
	return self->vtable->samplerate(self);
}

INLINE void psy_dsp_filter_update(psy_dsp_Filter* self, int full)
{
	self->vtable->update(self, full);
}

INLINE void psy_dsp_filter_reset(psy_dsp_Filter* self)
{
	self->vtable->reset(self);
}

typedef struct {
	psy_dsp_Filter filter;	
	float samplerate;
	float cutoff;
	float q;
} psy_dsp_CustomFilter;

void psy_dsp_customfilter_init(psy_dsp_CustomFilter*);

typedef struct {
	psy_dsp_amp_t x1;
	psy_dsp_amp_t x2;
	psy_dsp_amp_t y1;
	psy_dsp_amp_t y2;
} psy_dsp_FIRWork;

void psy_dsp_firwork_init(psy_dsp_FIRWork*);
psy_dsp_amp_t psy_dsp_firwork_work(psy_dsp_FIRWork*, psy_dsp_FilterCoeff* coeffs,
	psy_dsp_amp_t sample);
void psy_dsp_firwork_reset(psy_dsp_FIRWork*);

#ifdef __cplusplus
}
#endif

#endif /* psy_dsp_FILTER_H */
