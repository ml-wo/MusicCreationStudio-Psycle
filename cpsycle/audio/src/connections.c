// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include <stdlib.h>
#include <string.h>
#include "connections.h"
#include <assert.h>

static WireSocketEntry* wiresocketentry_allocinit(int slot, float volume, int send);
static void wiresocketentry_dispose(WireSocketEntry*);
static WireSocketEntry* connection_input(Connections* self, int outputslot, int inputslot);

void machinesockets_init(MachineSockets* self)
{
	self->outputs = 0;
	self->inputs = 0;
}

void machinesockets_dispose(MachineSockets* self)
{
	WireSocket* p;

	for (p = self->inputs; p != 0; p = p->next) {
		WireSocketEntry* entry;

		entry = (WireSocketEntry*)p->entry;
		wiresocketentry_dispose(entry);
		free(entry);
	}
	list_free(p);
	for (p = self->outputs; p != 0; p = p->next) {
		WireSocketEntry* entry;

		entry = (WireSocketEntry*)p->entry;
		wiresocketentry_dispose(entry);
		free(entry);
	}
	list_free(p);
}

MachineSockets* machinesockets_allocinit(void)
{	
	MachineSockets* rv;
	
	rv = malloc(sizeof(MachineSockets));
	if (rv) {
		machinesockets_init(rv);
	}
	return rv;
}

WireSocketEntry* wiresocketentry_allocinit(int slot, float volume, int send)
{
	WireSocketEntry* rv;
		
	rv = (WireSocketEntry*) malloc(sizeof(WireSocketEntry));
	if (rv) {
		PinConnection* pins;

		rv->slot = slot;
		rv->volume = 1.0;
		rv->send = send;
		pins = (PinConnection*)malloc(sizeof(PinConnection));
		pins->src = 0;
		pins->dst = 0;
		rv->mapping = list_create(pins);
		pins = (PinConnection*)malloc(sizeof(PinConnection));
		pins->src = 0;
		pins->dst = 0;
		list_append(&rv->mapping, pins);
	}
	return rv;
}

void wiresocketentry_dispose(WireSocketEntry* self)
{	
	List* p;

	for (p = self->mapping; p != 0; p = p->next) {
		free(p->entry);
	}
	list_free(self->mapping);
}

WireSocket* connection_at(WireSocket* socket, int slot)
{	
	WireSocket* p;
	
	p = socket;
	while (p && ((WireSocketEntry*)(p->entry))->slot != slot) {
		p = p->next;
	}
	return p;
}

WireSocketEntry* connection_input(Connections* self, int outputslot, int inputslot)
{
	WireSocketEntry* rv = 0;
	MachineSockets* sockets;

	sockets = connections_at(self, outputslot);	
	if (sockets) {
		WireSocket* input_socket;

		input_socket = connection_at(sockets->outputs, inputslot);
		if (input_socket) {			
			rv = (WireSocketEntry*) input_socket->entry;
		}
	}
	return rv;
}

void connections_init(Connections* self)
{
	table_init(&self->container);
	table_init(&self->sends);
	signal_init(&self->signal_connected);
	signal_init(&self->signal_disconnected);
	self->filemode = 0;
}

void connections_dispose(Connections* self)
{
	TableIterator it;

	for (it = table_begin(&self->container);
		!tableiterator_equal(&it, table_end()); tableiterator_inc(&it)) {		
		MachineSockets* sockets;

		sockets = (MachineSockets*)tableiterator_value(&it);
		machinesockets_dispose(sockets);
		free(sockets);
	}
	table_dispose(&self->container);
	table_dispose(&self->sends);
	signal_dispose(&self->signal_connected);
	signal_dispose(&self->signal_disconnected);
}

MachineSockets* connections_at(Connections* self, int slot)
{
	return table_at(&self->container, slot);	
}

MachineSockets* connections_initslot(Connections* self, int slot)
{
	MachineSockets* sockets;

	sockets = machinesockets_allocinit();
	table_insert(&self->container, slot, sockets);
	return sockets;
}

int connections_connect(Connections* self, int outputslot, int inputslot, int send)
{
	if (outputslot != inputslot && 
			!connections_connected(self, outputslot, inputslot)) {
		MachineSockets* connections;

		connections = connections_at(self, outputslot);		
		if (!connections) {
			connections = connections_initslot(self, outputslot);
		}
		if (connections) {
			list_append(&connections->outputs,
				wiresocketentry_allocinit(inputslot, 1.f, send));
		}
		connections = connections_at(self, inputslot);
		if (!connections) {
			connections = connections_initslot(self, inputslot);
		}
		if (connections) {
			list_append(&connections->inputs,
				wiresocketentry_allocinit(outputslot, 1.f, send));
		}
		if (!self->filemode) {
			signal_emit(&self->signal_connected, self, 2, outputslot, inputslot);
		}
		return 1;
	}
	return 0;
}

void connections_disconnect(Connections* self, int outputslot, int inputslot)
{	
	MachineSockets* connections;

	connections = connections_at(self, outputslot);
	if (connections) {				
		WireSocket* p;

		p = connection_at(connections->outputs, inputslot);		
		if (p) {
			free(p->entry);
			list_remove(&connections->outputs, p);			
		}		
		connections = connections_at(self, inputslot);
		p = connection_at(connections->inputs, outputslot);
		if (p) {		
			free(p->entry);
			list_remove(&connections->inputs, p);			
		}
		if (!self->filemode) {
			signal_emit(&self->signal_disconnected, self, 2, outputslot, inputslot);
		}
	}	
}

void connections_disconnectall(Connections* self, int slot)
{
	MachineSockets* connections;
	connections = connections_at(self, slot);
	if (connections) {		
		WireSocket* out;
		WireSocket* in;
				
		out = connections->outputs;
		while (out) {			
			int dstslot;
			MachineSockets* dst;
			WireSocket* dstinput;
			dst = connections_at(self,
				((WireSocketEntry*) out->entry)->slot);
			dstslot = ((WireSocketEntry*) out->entry)->slot;
			dstinput = connection_at(dst->inputs, slot);
			free(dstinput->entry);
			list_remove(&dst->inputs, dstinput);
			out = out->next;
			if (!self->filemode) {
				signal_emit(&self->signal_disconnected, self, 2, slot, dstslot);
			}
		}
		list_free(connections->outputs);
		connections->outputs = 0;
		
		in = connections->inputs;
		while (in) {			
			MachineSockets* src;
			WireSocket* srcoutput;
			int srcslot;

			src = connections_at(self,
				((WireSocketEntry*) in->entry)->slot);
			srcslot = ((WireSocketEntry*) in->entry)->slot;
			srcoutput = connection_at(src->outputs, slot);
			free(srcoutput->entry);
			list_remove(&src->outputs, srcoutput);
			in = in->next;
			if (!self->filemode) {
				signal_emit(&self->signal_disconnected, self, 2, srcslot, slot);
			}
		}
		list_free(connections->inputs);
		connections->inputs = 0;		
	}	
}

int connections_connected(Connections* self, int outputslot, int inputslot)
{	
	WireSocket* p = 0;
	MachineSockets* sockets;	

	sockets = connections_at(self, outputslot);
	if (sockets) {						
		p = connection_at(sockets->outputs, inputslot);
#if defined(_DEBUG)
		if (p) {			
			sockets = connections_at(self, inputslot);
			assert(sockets);
			p = connection_at(sockets->inputs, outputslot);
			assert(p);
		}
#endif
	}
	return p != 0;
}

void connections_setwirevolume(Connections* self, int outputslot,
	int inputslot, amp_t factor)
{	
	WireSocketEntry* input_entry;

	input_entry = connection_input(self, outputslot, inputslot);		
	if (input_entry) {
		input_entry->volume = factor;		
	}
}

amp_t connections_wirevolume(Connections* self, int outputslot, int inputslot)
{	
	WireSocketEntry* input_entry;

	input_entry = connection_input(self, outputslot, inputslot);		
	return input_entry ? input_entry->volume : 1.f;	
}
