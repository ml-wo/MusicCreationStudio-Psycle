// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "undoredobar.h"

static void undoredobar_onundo(UndoRedoBar*, psy_ui_Component* sender);
static void undoredobar_onredo(UndoRedoBar*, psy_ui_Component* sender);
static void undoredobar_initalign(UndoRedoBar* self);

void undoredobar_init(UndoRedoBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);	
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
	psy_ui_button_init(&self->undobutton, &self->component);
	psy_ui_button_settext(&self->undobutton,
		workspace_translate(workspace, "undo"));
	psy_signal_connect(&self->undobutton.signal_clicked, self,
		undoredobar_onundo);
	psy_ui_button_init(&self->redobutton, &self->component);
	psy_ui_button_settext(&self->redobutton,
		workspace_translate(workspace, "redo"));
	psy_signal_connect(&self->redobutton.signal_clicked, self,
		undoredobar_onredo);
	undoredobar_initalign(self);	
}

void undoredobar_initalign(UndoRedoBar* self)
{		
	psy_ui_Margin margin;

	psy_ui_margin_init(&margin, psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0));	
	psy_list_free(psy_ui_components_setalign(		
		psy_ui_component_children(&self->component, 0),
		psy_ui_ALIGN_LEFT,
		&margin));
}

void undoredobar_onundo(UndoRedoBar* self, psy_ui_Component* sender)
{
	workspace_undo(self->workspace);
}

void undoredobar_onredo(UndoRedoBar* self, psy_ui_Component* sender)
{
	workspace_redo(self->workspace);
}
