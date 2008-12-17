#pragma once

#include "EffectGui.hpp"

namespace psycle {
	namespace host {

		class VstFxGui : public EffectGui {
		public:
			VstFxGui(class MachineView* view,
				     class Machine* mac);
			~VstFxGui();

			virtual bool OnEvent(TestCanvas::Event* ev);

			virtual void BeforeDeleteDlg();

		private:
			void ShowDialog();

			class CVstEffectWnd* dialog_;
		};
	}
}