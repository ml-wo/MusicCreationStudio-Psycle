// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(DUPLICATOR2_H)
#define DUPLICATOR2_H

#include "custommachine.h"
#include "duplicatormap.h"

typedef struct {
	psy_audio_CustomMachine custommachine;
	short macoutput[16];
	short noteoffset[16];	
	short lowkey[16];
	short highkey[16];
	psy_audio_DuplicatorMap map;	
	int isticking;
} psy_audio_Duplicator2;

void duplicator2_init(psy_audio_Duplicator2*, MachineCallback);
const psy_audio_MachineInfo* duplicator2_info(void);

#endif
