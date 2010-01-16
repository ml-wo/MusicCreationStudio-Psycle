#include "PatHelperCommand.hpp"
#include "PatternView.hpp"
#include "Project.hpp"

namespace psycle {
	namespace host {
		
		PatHelperCommand::PatHelperCommand(PatternView* pat_view) 
			: pat_view_(pat_view) {
		}
		
		PatHelperCommand::~PatHelperCommand() {
		}


		void PatHelperCommand::PrepareUndoStorage() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			prev_pattern_ = *pat_view_->pattern();
#endif
		}

		void PatHelperCommand::PrepareRedoStorage() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			// Redo store
			next_pattern_ = *pat_view_->pattern();
#endif
		}

		void PatHelperCommand::Undo() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			pat_view_->pattern()->Clear();
			pat_view_->pattern()->timeSignatures() = prev_pattern_.timeSignatures();
			pat_view_->pattern()->insert(prev_pattern_, 0);
			pat_view_->Repaint(PatternView::draw_modes::pattern);
#endif
		}

		void PatHelperCommand::Redo() {
#if PSYCLE__CONFIGURATION__USE_PSYCORE
			pat_view_->pattern()->Clear();
			pat_view_->pattern()->timeSignatures() = next_pattern_.timeSignatures();
			pat_view_->pattern()->insert(next_pattern_, 0);
			pat_view_->Repaint(PatternView::draw_modes::pattern);
#endif
		}


	}	// namespace host
}	// namespace psycle