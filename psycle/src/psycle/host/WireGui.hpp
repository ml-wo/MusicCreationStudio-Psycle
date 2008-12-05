#pragma once

#include "canvas.hpp"

namespace psycle {
	namespace host {

		class WireGui : public TestCanvas::Line {
		public:
			WireGui(class MachineView* view);
			~WireGui();

			void UpdatePosition();

			void SetStart(class MachineGui* start) {
				start_ = start;
			}

			void setGuiConnectors(MachineGui* from, MachineGui* to,
								  MachineGui* start);

			MachineGui* fromGUI() const { return fromGUI_; }
			MachineGui* toGUI() const { return toGUI_; }
			MachineGui* start() const { return start_; }

			void dragging_start(int pickpoint);
			void dragging(double x, double y);
			void dragging_stop();
			bool dragging() const { return dragging_; }

			virtual bool OnEvent(TestCanvas::Event* ev);

		private:

			class MachineView* view_;
			MachineGui* fromGUI_;
			MachineGui* toGUI_;
			MachineGui* start_;

			int drag_picker_;
			bool dragging_;
		};

	}
}