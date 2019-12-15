// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "gear.h"

static void OnSize(Gear*, ui_component* sender, ui_size*);
static void ConnectSongSignals(Gear*);
static void OnDelete(Gear*, ui_component* sender);
static void OnSongChanged(Gear*, Workspace*);
static void OnClone(Gear*, ui_component* sender);
static void OnExchange(Gear* self, ui_component* sender);
static void OnParameters(Gear*, ui_component* sender);
static void OnMaster(Gear*, ui_component* sender);

void InitGearButtons(GearButtons* self, ui_component* parent)
{
	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);	
	ui_button_init(&self->createreplace, &self->component);
	ui_button_settext(&self->createreplace, "Create/Replace");
	ui_button_init(&self-> del, &self->component);
	ui_button_settext(&self->del, "Delete");	
	ui_button_init(&self->parameters, &self->component);
	ui_button_settext(&self->parameters, "Parameters");
	ui_button_init(&self->properties, &self->component);
	ui_button_settext(&self->properties, "Properties");
	ui_button_init(&self->exchange, &self->component);
	ui_button_settext(&self->exchange, "Exchange");
	ui_button_init(&self->clone, &self->component);
	ui_button_settext(&self->clone, "Clone");
	ui_button_init(&self->showmaster, &self->component);
	ui_button_settext(&self->showmaster, "Show master");
	{
		ui_margin margin = { 0, 3, 3, 0 };
		List* p;
		List* q;
		for (p = q = ui_component_children(&self->component, 0); p != 0; p = p->next) {
			ui_component* component;
			
			component = (ui_component*)p->entry;
			ui_component_setalign(component, UI_ALIGN_TOP);
			ui_component_setmargin(component, &margin);
			ui_component_resize(component, 0, 20);
		}
		list_free(q);
	}
}

void InitGear(Gear* self, ui_component* parent, Workspace* workspace)
{		
	self->workspace = workspace;
	self->machines = &workspace->song->machines;
	psy_signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
	ui_component_init(&self->component, parent);
	ui_component_resize(&self->component, 300, 0);	
	tabbar_init(&self->tabbar, &self->component);	
	ui_component_move(&self->tabbar.component, 0, 0);
	ui_component_resize(&self->tabbar.component, 0, 20);
	tabbar_append(&self->tabbar, "Generators");
	tabbar_append(&self->tabbar, "Effects");	
	tabbar_append(&self->tabbar, "Instruments");
	tabbar_append(&self->tabbar, "Waves");	
	tabbar_select(&self->tabbar, 0);		
	ui_notebook_init(&self->notebook, &self->component);
	psy_signal_connect(&self->component.signal_size, self, OnSize);	
	InitMachinesBox(&self->machinesboxgen, &self->notebook.component, 
		&workspace->song->machines, MACHINEBOX_GENERATOR, self->workspace);
	InitMachinesBox(&self->machinesboxfx, &self->notebook.component, 
		&workspace->song->machines, MACHINEBOX_FX, self->workspace);
	InitInstrumentsBox(&self->instrumentsbox, &self->notebook.component, 
		&workspace->song->instruments);
	samplesbox_init(&self->samplesbox, &self->notebook.component, 
		&workspace->song->samples, &workspace->song->instruments);
	ConnectSongSignals(self);
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);
	tabbar_select(&self->tabbar, 0);
	InitGearButtons(&self->buttons, &self->component);
	psy_signal_connect(&self->buttons.del.signal_clicked, self, OnDelete);
	psy_signal_connect(&self->buttons.clone.signal_clicked, self, OnClone);
	psy_signal_connect(&self->buttons.parameters.signal_clicked, self,
		OnParameters);
	psy_signal_connect(&self->buttons.showmaster.signal_clicked, self,
		OnMaster);
	psy_signal_connect(&self->buttons.exchange.signal_clicked, self,
		OnExchange);
	ui_component_resize(&self->buttons.component, 100, 0);
}

void OnSize(Gear* self, ui_component* sender, ui_size* size)
{
	ui_size buttonssize;

	buttonssize = ui_component_size(&self->buttons.component);
	ui_component_move(&self->tabbar.component, 0, size->height - 20);
	ui_component_resize(&self->tabbar.component, size->width, 20);
	ui_component_setposition(&self->notebook.component, 0, 0,
		size->width - buttonssize.width, size->height - 20);
	ui_component_setposition(&self->buttons.component,
		size->width - buttonssize.width , 0,
		buttonssize.width, size->height - 20);
}

void ConnectSongSignals(Gear* self)
{
	// psy_signal_connect(&self->instruments->signal_insert, self, OnInstrumentInsert);
	// psy_signal_connect(&self->instruments->signal_slotchange, self, OnInstrumentSlotChanged);	
}

void OnDelete(Gear* self, ui_component* sender)
{
	switch (self->tabbar.selected) {
		case 0:
			MachinesBoxRemove(&self->machinesboxgen);
		break;
		case 1:
			MachinesBoxRemove(&self->machinesboxfx);
		break;
		default:
		break;
	}
}

void OnSongChanged(Gear* self, Workspace* workspace)
{	
	self->machines = &workspace->song->machines;		
	SetMachines(&self->machinesboxgen, &workspace->song->machines);
	SetMachines(&self->machinesboxfx, &workspace->song->machines);
	SetInstruments(&self->instrumentsbox, &workspace->song->instruments);
	samplesbox_setsamples(&self->samplesbox, &workspace->song->samples,
		&workspace->song->instruments);
	ConnectSongSignals(self);
	ui_component_invalidate(&self->component);
}

void OnClone(Gear* self, ui_component* sender)
{
	switch (self->tabbar.selected) {
		case 0:
			MachinesBoxClone(&self->machinesboxgen);
		break;
		case 1:
			MachinesBoxClone(&self->machinesboxfx);
		break;
		default:
		break;
	}
}

void OnExchange(Gear* self, ui_component* sender)
{
	switch (self->tabbar.selected) {
		case 0:
			MachinesBoxExchange(&self->machinesboxgen);
		break;
		case 1:
			MachinesBoxExchange(&self->machinesboxfx);
		break;
		default:
		break;
	}
}


void OnParameters(Gear* self, ui_component* sender)
{
	switch (self->tabbar.selected) {
		case 0:
			MachinesBoxShowParameters(&self->machinesboxgen);
		break;
		case 1:
			MachinesBoxShowParameters(&self->machinesboxfx);
		break;
		default:
		break;
	}
}

void OnMaster(Gear* self, ui_component* sender)
{
	workspace_showparameters(self->workspace, MASTER_INDEX);
}