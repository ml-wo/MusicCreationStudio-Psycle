#pragma once
#include "PatHelperCommand.hpp"

namespace psycle { namespace host {

class InsertCurrCommand : public PatHelperCommand {
	public:
		InsertCurrCommand(class PatternView* pat_view);
		virtual void Execute();
};

}}
