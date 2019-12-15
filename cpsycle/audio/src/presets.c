// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "presets.h"
#include <stdlib.h>

void presets_init(Presets* self)
{	
	self->container = 0;
}

void presets_dispose(Presets* self)
{
	List* p;

	for (p = self->container; p != 0; p = p->next) {
		free(p->entry);
	}
	free(self->container);
	self->container = 0;
}

Presets* presets_alloc(void)
{
	return malloc(sizeof(Presets));
}

Presets* presets_allocinit(void)
{
	Presets* rv;

	rv = presets_alloc();
	if (rv) {
		presets_init(rv);
	}
	return rv;
}

void presets_append(Presets* self, Preset* preset)
{
	list_append(&self->container, preset);
}