// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT
#ifndef psy_ui_xt_LABELIMP_H
#define psy_ui_xt_LABELIMP_H

#include "uilabel.h"
#include "uixtcomponentimp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_xt_LabelImp {
	psy_ui_LabelImp imp;
	psy_ui_xt_ComponentImp xt_component_imp;	
	struct psy_ui_Component* component;	
} psy_ui_xt_LabelImp;

void psy_ui_xt_labelimp_init(psy_ui_xt_LabelImp* self,
	struct psy_ui_Component* component,
	struct psy_ui_ComponentImp* parent);

psy_ui_xt_LabelImp* psy_ui_xt_labelimp_alloc(void);
psy_ui_xt_LabelImp* psy_ui_xt_labelimp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_xt_ComponentImp_H */
#endif /* PSYCLE_TK_XT */
