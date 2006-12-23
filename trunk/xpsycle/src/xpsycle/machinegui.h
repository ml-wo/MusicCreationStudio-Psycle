/***************************************************************************
*   Copyright (C) 2006 by Stefan Nattkemper   *
*   natti@linux   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef MACHINEGUI_H
#define MACHINEGUI_H

#include "skinreader.h"
#include "macpropdlg.h"
#include "wiregui.h"
#include <ngrs/npanel.h>

class NSlider;

namespace psycle {
	namespace host {

		class Machine;
		class FrameMachine;
		class MacPropDlg;
		class MasterDlg;

		/**
		@author Stefan Nattkemper
		*/

		class MachineGUI : public NPanel
		{
			class LineAttachment {
			public:

				LineAttachment( WireGUI* line, int pt ) 
					: point_( pt ), line_( line ) {
				}

				int point() const {
					return point_;
				}

				WireGUI* line() {
					return line_;
				}			

			private:

				int point_;
				WireGUI* line_;

			};

		public:
			MachineGUI( Machine & mac );

			~MachineGUI();

			signal1<MachineGUI*> deleteRequest;

			Machine & mac();

			void attachLine( WireGUI* line, int point );
			void detachLine( WireGUI* line );

			signal1<MachineGUI*> newConnection;
			signal3<Machine*,int,int> moved;
			signal3<int,int,int> patternTweakSlide;
			signal1<MachineGUI*> selected;

			int ident() const;

			virtual void onMouseDoublePress(int x, int y, int button);
			virtual void onMousePress(int x, int y, int button);
			virtual void onMoveStart(const NMoveEvent & moveEvent);
			virtual void onMove(const NMoveEvent & moveEvent);
			virtual void onMoveEnd(const NMoveEvent & moveEvent);
			virtual void resize();

			virtual void paint(NGraphics* g);

			virtual void repaintVUMeter();

			void setSelected( bool on );		

			void setCoordInfo( const MachineCoordInfo &  coords );
			const MachineCoordInfo & coords() const;

			virtual void updateSkin();
			virtual MacPropDlg* propsDlg();
			virtual void showPropsDlg();
			void onUpdateMachinePropertiesSignal(Machine* machine);
			void onDeleteMachineSignal();

		private:								

			bool selected_;
			NRegion oldDrag;
			Machine* mac_;
			MacPropDlg* propsDlg_;			
			MachineCoordInfo coords_;
			std::vector<LineAttachment> attachedLines;

			NRegion linesRegion() const;
		};


		class MasterGUI : public MachineGUI
		{
		public:
			MasterGUI( Machine & mac );

			~MasterGUI();


			virtual void onMousePress(int x, int y, int button);
			virtual void onMouseDoublePress(int x, int y, int button);

			virtual void paint(NGraphics* g);

			virtual void updateSkin();
			virtual void showPropsDlg() {}; // override--we don't to see a master props dlg (atm)

		private:

			MasterDlg* masterDlg;

			void setSkin();
		};


		class GeneratorGUI : public MachineGUI
		{
		public:

			class VUPanel : public NPanel {

				friend class GeneratorGUI;

			public:
				VUPanel(GeneratorGUI* pGui) {
					pGui_ = pGui;
				};

				virtual void paint(NGraphics* g);

			private:

				GeneratorGUI* pGui_;

			};

			GeneratorGUI( Machine & mac );

			~GeneratorGUI();

			FrameMachine* frameMachine;

			virtual void onMousePress( int x, int y, int button );

			virtual void repaintVUMeter();

			virtual void onMouseDoublePress( int x, int y, int button );

			virtual void paint( NGraphics* g );

			virtual void onKeyPress( const NKeyEvent & event);

			virtual void updateSkin();

		private:

			NSlider* panSlider_;
			VUPanel* vuPanel_;

			void setSkin();
			void customSliderPaint(NSlider* sl, NGraphics* g);		
			void onPosChanged( NSlider* sender );
			void onTweakSlide( int machine, int command, int value );
		};


		class EffektGUI : public MachineGUI
		{
		public:

			class VUPanel : public NPanel {

				friend class EffectGUI;

			public:
				VUPanel(EffektGUI* pGui) {
					pGui_ = pGui;
				};

				virtual void paint(NGraphics* g);

			private:

				EffektGUI* pGui_;

			};


			EffektGUI( Machine & mac );

			~EffektGUI();
			
			virtual void onMousePress(int x, int y, int button);
			virtual void onMouseDoublePress(int x, int y, int button);
			virtual void paint(NGraphics* g);
			virtual void repaintVUMeter();
			virtual void onKeyPress( const NKeyEvent & event );
			virtual void updateSkin();

		private:

			NSlider* panSlider_;
			VUPanel* vuPanel_;
			FrameMachine* frameMachine;

			void setSkin();
			void customSliderPaint(NSlider* sl, NGraphics* g);    
			void onPosChanged( NSlider* sender );
			void onTweakSlide( int machine, int command, int value );
		};

	}
}

#endif
