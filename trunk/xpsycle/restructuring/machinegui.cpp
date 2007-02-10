/***************************************************************************
*   Copyright (C) 2006 by  Stefan Nattkemper                               *
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
#include "machinegui.h"
#include "configuration.h"
#include "defaultbitmaps.h"
#include <psycore/machine.h>
#include <psycore/song.h>
#include <ngrs/frameborder.h>
#include <ngrs/window.h>
#include <ngrs/slider.h>
#include <ngrs/statusmodel.h>

namespace psy {
	namespace host {


		// MachineGUI abstract class

      MachineGUI::MachineGUI( psy::core::Machine& mac )
			: ngrs::Panel()
		{
			selected_ = 0;
			mac_ = & mac;
			setMoveable( ngrs::Moveable(ngrs::nMvHorizontal | ngrs::nMvVertical | ngrs::nMvNoneRepaint | ngrs::nMvTopLimit | ngrs::nMvLeftLimit));
			setFont( ngrs::Font("Suse sans",6, ngrs::nMedium | ngrs::nStraight | ngrs::nAntiAlias));

		}


		MachineGUI::~MachineGUI()
		{

		}

		psy::core::Machine& MachineGUI::mac( )
		{
			return *mac_;
		}

		void MachineGUI::paint( ngrs::Graphics& g )
		{
			if (selected_) {

				g.setForeground( SkinReader::Instance()->machineview_color_info().sel_border_color );

				int cw = clientWidth();
				int ch = clientHeight();
				int size = 10;
				// upper left corner
				g.drawLine(0,0,size,0);
				g.drawLine(0,0,0,size);
				// upper right corner
				g.drawLine(cw-size,0,cw,0);
				g.drawLine(cw-1,0,cw-1,size);
				// lower left corner
				g.drawLine(0,ch-size,0,ch-1);
				g.drawLine(0,ch-1,size,ch-1);
				// lower right corner
				g.drawLine(cw-1,ch-size,cw-1,ch);
				g.drawLine(cw-size,ch-1,cw-1,ch-1);
			}
		}

		void MachineGUI::setSelected( bool on )
		{
			selected_ = on;
		}

		void MachineGUI::attachLine( WireGUI* line, int point )
		{
			attachedLines.push_back(LineAttachment(line,point));
			int midW = clientWidth()  / 2;
			int midH = clientHeight() / 2;
			if (point == 1) {
				line->setPoints( ngrs::Point(left()+midW, top()+midH), line->p2() );
			} else {
				line->setPoints( line->p1(), ngrs::Point(left()+midW, top()+midH) );
			}

		}

		ngrs::Region MachineGUI::linesRegion( ) const
		{
			ngrs::Region region( geometry()->rectArea() );

			for (std::vector<LineAttachment>::const_iterator itr = attachedLines.begin(); itr < attachedLines.end(); itr++) {
				LineAttachment lineAttach = *itr;
				region |= lineAttach.line()->geometry()->region();
			}
			return region;
		}

		void MachineGUI::onMoveStart( const ngrs::MoveEvent & moveEvent )
		{
			oldDrag = linesRegion();
		}

		void MachineGUI::onMove( const ngrs::MoveEvent& moveEvent )
		{
			ngrs::Region newDrag = linesRegion();
			ngrs::Region repaintArea = newDrag | oldDrag;

			ngrs::VisualComponent* parentVc =  (ngrs::VisualComponent*)( parent() );

			int parentAbsLeft = parentVc->absoluteLeft() - parentVc->scrollDx();
			int parentAbsTop  = parentVc->absoluteTop() - parentVc->scrollDy();;

			repaintArea.move( parentAbsLeft, parentAbsTop );

			window()->repaint( parentVc, repaintArea );

			oldDrag = newDrag;

            if ( window()->statusModel() ) {
				//std::string msg =  mac()._editName + "("+ stringify(left()) + ","+ stringify(top())+ ")";
				//window()->statusModel()->setText( msg );
			}
		}

		int MachineGUI::ident( ) const
		{
			// this is the ident for selection border
			return 5;
		}

		void MachineGUI::resize( )
		{
			for (std::vector<LineAttachment>::iterator itr = attachedLines.begin(); itr < attachedLines.end(); itr++) {
				LineAttachment lineAttach = *itr;
				int midW = clientWidth() / 2;
				int midH = clientHeight() / 2;
				if ( lineAttach.point() == 1 ) {
					lineAttach.line()->setPoints( ngrs::Point( left() + midW, top() + midH ), lineAttach.line()->p2() );
				} else {
					lineAttach.line()->setPoints( lineAttach.line()->p1(), ngrs::Point( left() + midW, top() + midH ) );
				}
			}
		}

		void MachineGUI::setCoordInfo( const MachineCoordInfo&  coords ) {
			coords_ = coords;
		}

		const MachineCoordInfo& MachineGUI::coords() const {
			return coords_;
		}

		void MachineGUI::updateSkin() {
			// virtual call only for subclasses
		}

		void MachineGUI::onMousePress( int x, int y, int button )
		{
			int shift = ngrs::App::system().shiftState();
			if ( (shift & ngrs::nsShift & ngrs::nsLeft) || button == 3 ) {
				// shift+left-click or right-click.
				newConnection.emit(this);
			} else if ( shift & ngrs::nsLeft ) { // left-click (w/ no shift)
				selected.emit(this);
			} else if ( button == 2) {
			}
		}

		void MachineGUI::onDeleteMachineSignal() {
			deleteRequest.emit(this);
		}

		void MachineGUI::detachLine( WireGUI* line )
		{
			std::vector<LineAttachment>::iterator it = attachedLines.begin();

			for (;it <  attachedLines.end(); it++) {
				LineAttachment lineAttachment = *it;

				if (lineAttachment.line() == line) {
					attachedLines.erase( it ); 
					break;
				}
			}
		}

		void MachineGUI::onMouseDoublePress( int x, int y, int button )
		{
		}

		void MachineGUI::onMoveEnd( const ngrs::MoveEvent& moveEvent )
		{
			((VisualComponent*) parent())->resize();
		}

		void MachineGUI::repaintVUMeter( )
		{
		}

		// end of Machine GUI class



		// the MasterGUI class
		MasterGUI::MasterGUI( psy::core::Machine& mac ) : MachineGUI( mac )
		{
			setSkin();
			setBackground( ngrs::Color( 0, 0, 200 ) );
		}

		MasterGUI::~ MasterGUI( )
		{
		}

		void MasterGUI::setSkin( )
		{
			setCoordInfo( SkinReader::Instance()->machineview_master_coords() );

			setTransparent( true );
			setHeight( coords().bgCoords.height() + 2*ident() );
			setWidth( coords().bgCoords.width()   + 2*ident() );
			setBackground( ngrs::Color( 0, 0, 200) );
		}

		void MasterGUI::paint( ngrs::Graphics& g )
		{
			MachineGUI::paint(g);
			// save old translation pos from the grpahics handler
			long xTrans = g.xTranslation();
			long yTrans = g.yTranslation();
			// move translation to have place for selection border
			g.setTranslation(xTrans + ident(), yTrans+ ident());


            g.putPixmap(0,0,coords().bgCoords.width(),coords().bgCoords.height(), SkinReader::Instance()->bitmaps().machine_skin(), coords().bgCoords.left(), coords().bgCoords.top());

			/*
			if (mac()._mute)
			g.putPixmap(coords().dMuteCoords.left(),coords().dMuteCoords.top(),coords().muteCoords.width(),coords().muteCoords.height(), Global::configuration().icons().machine_skin(), coords().muteCoords.left(), coords().muteCoords.top());

			if ( mac().song()->machineSoloed == mac()._macIndex)
			g.putPixmap(coords().dSoloCoords.left(),coords().dSoloCoords.top(),coords().soloCoords.width(),coords().soloCoords.height(), Global::configuration().icons().machine_skin(), coords().soloCoords.left(), coords().soloCoords.top());*/

			// reset translation to original
			g.setTranslation( xTrans, yTrans );
		}

		void MasterGUI::onMousePress( int x, int y, int button )
		{
			// empty; ther master gui doesn't do anything with a single
			// mouse press other than move around.
		}

		void MasterGUI::updateSkin() {
			setSkin();
		} 

		void MasterGUI::onMouseDoublePress( int x, int y, int button ) {
		}
		// end of MasterGUI class


		//
		// start of GeneratorGUI class
		//
		GeneratorGUI::GeneratorGUI( psy::core::Machine& mac ) : MachineGUI( mac )
		{
			panSlider_ = new ngrs::Slider();
			panSlider_->change.connect(this,&GeneratorGUI::onPosChanged);
			add(panSlider_);

			setSkin();

			vuPanel_ = new VUPanel(this);
			vuPanel_->setPosition(coords().dVu.left() + ident(),coords().dVu.top() + ident(),coords().dVu.width(),coords().dVu.height());
			vuPanel_->setTransparent(false);
			add(vuPanel_);

		}

		GeneratorGUI::~ GeneratorGUI( )
		{
		}

		void GeneratorGUI::paint( ngrs::Graphics& g )
		{
			MachineGUI::paint(g);
			// save old translation pos from the grpahics handler
			long xTrans = g.xTranslation();
			long yTrans = g.yTranslation();
			// move translation to have place for selection border
			g.setTranslation(xTrans + ident(), yTrans+ ident());

			g.putPixmap(0,0,coords().bgCoords.width(),coords().bgCoords.height(), SkinReader::Instance()->bitmaps().machine_skin(), coords().bgCoords.left(), coords().bgCoords.top());
//			g.drawText(coords().dNameCoords.x(),coords().dNameCoords.y()+g.textAscent(), stringify(mac()._macIndex)+ ":" + mac()._editName );


//			if ( mac()._mute )
//				g.putPixmap(coords().dMuteCoords.left(),coords().dMuteCoords.top(),coords().muteCoords.width(),coords().muteCoords.height(), SkinReader::Instance()->bitmaps().machine_skin(), coords().muteCoords.left(), coords().muteCoords.top());

//			if ( mac().song()->machineSoloed == mac()._macIndex )
//				g.putPixmap(coords().dSoloCoords.left(),coords().dSoloCoords.top(),coords().soloCoords.width(),coords().soloCoords.height(), SkinReader::Instance()->bitmaps().machine_skin(), coords().soloCoords.left(), coords().soloCoords.top());

			// reset old Translation
			g.setTranslation( xTrans, yTrans );
		}

		void GeneratorGUI::setSkin( )
		{
			setCoordInfo( SkinReader::Instance()->machineview_generator_coords() );

			setHeight( coords().bgCoords.height() + 2*ident() );
			setWidth( coords().bgCoords.width() + 2*ident() );

			setTransparent(true);

			panSlider_->setPosition(45 + ident() ,26 + ident(),91,coords().sPan.height());
			panSlider_->setOrientation(ngrs::nHorizontal);
			panSlider_->setTrackLine(false);
			panSlider_->setRange(0,127);
//			panSlider_->setPos( mac()._panning );
			panSlider_->customSliderPaint.connect(this,&GeneratorGUI::customSliderPaint);
			panSlider_->slider()->setWidth( coords().sPan.width() );
			panSlider_->slider()->setHeight( coords().sPan.height() );
		}

		void GeneratorGUI::onPosChanged( ngrs::Slider* sender )
		{
//		  mac().SetPan( (int) panSlider_->pos());		
		}

		void GeneratorGUI::onMousePress( int x, int y, int button )
		{
			MachineGUI::onMousePress(x,y,button);
			if (button==1) { // left-click
				if (coords().dMuteCoords.intersects(x-ident(),y-ident())) { 
                  // mute or unmute
				  repaint();
                } else
			    if (coords().dSoloCoords.intersects(x-ident(),y-ident())) { 
                  // solo or unsolo
                } 
			}
		}

		void GeneratorGUI::repaintVUMeter( )
		{
			vuPanel_->repaint();
		}

		void GeneratorGUI::VUPanel::paint( ngrs::Graphics& g )
		{
			/*int vol = pGui_->mac()._volumeDisplay;
			int max = pGui_->mac()._volumeMaxDisplay;

			vol *= pGui_->dGeneratorVu.width();
			vol /= 96;

			max *= pGui_->dGeneratorVu.width();
			max /= 96;

			// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
			if (vol > 0)
			{
			if (pGui_->sGeneratorVu0.width())
			{
			vol /= pGui_->sGeneratorVu0.width();// restrict to leds
			vol *= pGui_->sGeneratorVu0.width();
			}
			} else {
			vol = 0;
			}

			if (max >0 || vol >0)
			g.putPixmap(vol,0,clientWidth()-vol, pGui_->sGeneratorVu0.height(),
			Global::configuration().icons().machine_skin(),
			pGui_->sGenerator.left() + pGui_->dGeneratorVu.left() +vol,
			pGui_->sGenerator.top() + pGui_->dGeneratorVu.top()
			);

			if (max > 0) {
			if (pGui_->sGeneratorVuPeak.width()) {
			max /= pGui_->sGeneratorVuPeak.width();// restrict to leds
			max *= pGui_->sGeneratorVuPeak.width();
			g.putPixmap(max,0, pGui_->sGeneratorVuPeak.width(), pGui_->sGeneratorVuPeak.height(),
			Global::configuration().icons().machine_skin(),
			pGui_->sGeneratorVuPeak.left(),
			pGui_->sGeneratorVuPeak.top()
			); //peak
			}
			}

			if (vol > 0) {
			g.putPixmap(0,0,vol, pGui_->sGeneratorVu0.height(), Global::configuration().icons().machine_skin(),
			pGui_->sGeneratorVu0.left(), pGui_->sGeneratorVu0.top()); // leds
			}
			*/
		}

		void GeneratorGUI::customSliderPaint( ngrs::Slider* sl, ngrs::Graphics& g )
		{
			g.putPixmap(0,0,coords().sPan.width(),coords().sPan.height(), SkinReader::Instance()->bitmaps().machine_skin(), coords().sPan.left(),coords().sPan.top());
		}

		void GeneratorGUI::updateSkin() {
			setSkin();
		}

		void GeneratorGUI::onTweakSlide( int machine, int command, int value )
		{
			patternTweakSlide.emit(machine,command,value);
		}

		void GeneratorGUI::onKeyPress( const ngrs::KeyEvent & event )
		{
			if ( event.scancode() == ngrs::NK_Delete ) 
				deleteRequest.emit( this );
		}

		void GeneratorGUI::onMouseDoublePress( int x, int y, int button ) {
		}
		// end of GeneratorGUI class


		//
		// the Effekt Gui class
		//
		EffektGUI::EffektGUI( psy::core::Machine& mac ) : MachineGUI( mac )
		{
			panSlider_ = new ngrs::Slider( );
			panSlider_->change.connect( this, &EffektGUI::onPosChanged );
			add(panSlider_);

			setSkin();

			vuPanel_ = new VUPanel( this );
			vuPanel_->setPosition( coords().dVu.left() + ident(), coords().dVu.top() + ident(), coords().dVu.width(), coords().dVu.height() );
			vuPanel_->setTransparent( false );
			add( vuPanel_ );
		}

		EffektGUI::~ EffektGUI( )
		{
		}

		void EffektGUI::paint( ngrs::Graphics& g )
		{
			MachineGUI::paint(g);
			// save old translation pos from the grpahics handler
			long xTrans = g.xTranslation();
			long yTrans = g.yTranslation();
			// move translation to have place for selection border
			g.setTranslation(xTrans + ident(), yTrans+ ident());

			g.putPixmap(0,0, coords().bgCoords.width(), coords().bgCoords.height(), SkinReader::Instance()->bitmaps().machine_skin(), coords().bgCoords.left(), coords().bgCoords.top() );
//		    g.drawText( coords().dNameCoords.x(), coords().dNameCoords.y() + g.textAscent(), mac()._editName);

//			if (mac()._mute)
//				g.putPixmap( coords().dMuteCoords.left(), coords().dMuteCoords.top(), coords().muteCoords.width(), coords().muteCoords.height(), SkinReader::Instance()->bitmaps().machine_skin(), coords().muteCoords.left(), coords().muteCoords.top());

//			if (mac().song()->machineSoloed == mac()._macIndex)
//				g.putPixmap( coords().dSoloCoords.left(), coords().dSoloCoords.top(), coords(). soloCoords.width(), coords().soloCoords.height(), SkinReader::Instance()->bitmaps().machine_skin(), coords().soloCoords.left(), coords().soloCoords.top() );

			// move translation to original
			g.setTranslation(xTrans, yTrans);
		}

		void EffektGUI::setSkin( )
		{

			setCoordInfo( SkinReader::Instance()->machineview_effect_coords() );

			setHeight( coords().bgCoords.height() + 2*ident() );
			setWidth(  coords().bgCoords.width()   + 2*ident() );
			setTransparent(true);

			panSlider_->setPosition(46+ident() ,26+ident(),91, coords().sPan.height());
			panSlider_->setOrientation(ngrs::nHorizontal);
			panSlider_->setTrackLine(false);
			panSlider_->setRange(0,127);
//			panSlider_->setPos( mac()._panning );
			panSlider_->customSliderPaint.connect(this,&EffektGUI::customSliderPaint);
			panSlider_->slider()->setWidth( coords().sPan.width());
			panSlider_->slider()->setHeight( coords().sPan.height());
		}


		void EffektGUI::onPosChanged( ngrs::Slider* sender )
		{
//		  mac().SetPan( (int) panSlider_->pos());
		}

		void EffektGUI::customSliderPaint( ngrs::Slider * sl, ngrs::Graphics& g )
		{
			g.putPixmap(0,0, coords().sPan.width(), coords().sPan.height(), SkinReader::Instance()->bitmaps().machine_skin(), coords().sPan.left(), coords().sPan.top() );
		}

		void EffektGUI::onMousePress( int x, int y, int button )
		{
			MachineGUI::onMousePress(x,y,button);
			if (button==1) {
				if ( coords().dMuteCoords.intersects(x-ident(),y-ident()) ) { 
                  // mute or unmute				
				} else
                if ( coords().dSoloCoords.intersects(x-ident(),y-ident()) ) { 
                  // solo or unsolo
                }
			}
		}

		void EffektGUI::repaintVUMeter( )
		{
			vuPanel_->repaint();
		}

		void EffektGUI::updateSkin() {
			setSkin();
		}


		void EffektGUI::VUPanel::paint( ngrs::Graphics& g )
		{
			/*int vol = pGui_->mac()._volumeDisplay;
			int max = pGui_->mac()._volumeMaxDisplay;

			vol *= pGui_->dGeneratorVu.width();
			vol /= 96;

			max *= pGui_->dGeneratorVu.width();
			max /= 96;

			// BLIT [DESTX,DESTY,SIZEX,SIZEY,source,BMPX,BMPY,mode]
			if (vol > 0)
			{
			if (pGui_->sGeneratorVu0.width())
			{
			vol /= pGui_->sGeneratorVu0.width();// restrict to leds
			vol *= pGui_->sGeneratorVu0.width();
			}
			} else {
			vol = 0;
			}

			g.putPixmap(vol,0,clientWidth()-vol, pGui_->sGeneratorVu0.height(),
			Global::configuration().icons().machine_skin(),
			pGui_->sGenerator.left() + pGui_->dGeneratorVu.left() +vol,
			pGui_->sGenerator.top() + pGui_->dGeneratorVu.top()
			);

			if (max > 0) {
			if (pGui_->sGeneratorVuPeak.width()) {
			max /= pGui_->sGeneratorVuPeak.width();// restrict to leds
			max *= pGui_->sGeneratorVuPeak.width();
			g.putPixmap(max,0, pGui_->sGeneratorVuPeak.width(), pGui_->sGeneratorVuPeak.height(),
			Global::configuration().icons().machine_skin(),
			pGui_->sGeneratorVuPeak.left(),
			pGui_->sGeneratorVuPeak.top()
			); //peak
			}
			}

			if (vol > 0) {
			g.putPixmap(0,0,vol, pGui_->sGeneratorVu0.height(), Global::configuration().icons().machine_skin(),
			pGui_->sGeneratorVu0.left(), pGui_->sGeneratorVu0.top()); // leds
			}
			*/
		}

		void EffektGUI::onMouseDoublePress( int x, int y, int button ) {
		}

		void EffektGUI::onTweakSlide( int machine, int command, int value )
		{
			patternTweakSlide.emit(machine,command,value);
		}

		void EffektGUI::onKeyPress( const ngrs::KeyEvent & event )
		{
			if ( event.scancode() == ngrs::NK_Delete ) {
				deleteRequest.emit( this );
			}
		}
		// end of EffektGUI class



	} // end of host namespace
} // end of psycle namespace
