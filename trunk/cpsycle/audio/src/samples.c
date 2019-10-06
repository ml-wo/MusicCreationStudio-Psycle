// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "samples.h"

void samples_init(Samples* self)
{
	table_init(&self->container);
}

void samples_dispose(Samples* self)
{
	Sample* sample;
	int slot;
	
	for (slot = self->container.keymin; slot <= self->container.keymax; ++slot) {
		sample = samples_at(self, slot);
		if (sample) {
			sample_dispose(sample);			
		}
	}
	table_dispose(&self->container);
}

void samples_insert(Samples* self, Sample* sample, int slot)
{
	table_insert(&self->container, slot, sample);
}

Sample* samples_at(Samples* self, int slot)
{
	return table_at(&self->container, slot);
}
