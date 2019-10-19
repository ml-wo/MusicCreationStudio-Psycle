// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(CONNECTIONS_H)
#define CONNECTIONS_H

#include <hashtbl.h>
#include <list.h>
#include <dsptypes.h>

#define MASTER_INDEX 128
#define MAX_STREAM_SIZE 256

typedef struct {
	int slot;	
	amp_t volume;	
} WireSocketEntry;

typedef List WireSocket;

typedef struct {	
	WireSocket* inputs;
	WireSocket* outputs;
} MachineSockets;

void machinesockets_init(MachineSockets*);
void machinesockets_dispose(MachineSockets*);

WireSocket* connection_at(WireSocket*, int slot);

typedef struct {
	Table container;	
} Connections;

void connections_init(Connections*);
void connections_dispose(Connections*);
MachineSockets* connections_initslot(Connections*, int slot);
MachineSockets* connections_at(Connections*, int slot);
int connections_connect(Connections*, int outputslot, int inputslot);
void connections_disconnect(Connections*, int outputslot, int inputslot);
int connections_connected(Connections*, int outputslot, int inputslot);
void connections_disconnectall(Connections*, int slot);


#endif
