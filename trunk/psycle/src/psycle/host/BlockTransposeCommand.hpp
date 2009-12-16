#include "Command.hpp"
#include "configuration_options.hpp"

#if PSYCLE__CONFIGURATION__USE_PSYCORE
#include <psycle/core/singlepattern.h>
#endif

namespace psycle {
	namespace host {

		class BlockTransposeCommand : public CommandUndoable {
		public:
			BlockTransposeCommand(class PatternView* pat_view, int trp);
			~BlockTransposeCommand();

			virtual void Execute();
			virtual void Undo();
			virtual void Redo();

		private:
			PatternView* pat_view_;
			int trp_;
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			psycle::core::Pattern prev_pattern_;
			psycle::core::Pattern next_pattern_;
#endif
		};

	}	// namespace host
}	// namespace psycle