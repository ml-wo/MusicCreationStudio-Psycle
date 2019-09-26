// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(VSTPLUGIN_H)
#define VSTPLUGIN_H

#include "machine.h"
#include "library.h"

typedef struct {
	Machine machine;		
	Library library;
	struct AEffect* effect;
	CMachineInfo* info;	
} VstPlugin;

void vstplugin_init(VstPlugin*, MachineCallback, const char* path);
CMachineInfo* plugin_vst_test(const char* path);

#endif
