/***************************************************************************
  *   Copyright (C) 2006 by Stefan   *
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
#include "patternview.h"
#include "configuration.h"
#include "global.h"
#include "song.h"
#include "inputhandler.h"
#include "player.h"
#include "machine.h"
#include "defaultbitmaps.h"
#include "zoombar.h"
#include <ngrs/napp.h>
#include <ngrs/nalignlayout.h>
#include <ngrs/nwindow.h>
#include <ngrs/nmenuitem.h>
#include <ngrs/nmenuseperator.h>
#include <ngrs/nframeborder.h>
#include <ngrs/nfontmetrics.h>
#include <ngrs/ntoolbar.h>
#include <ngrs/ncombobox.h>
#include <ngrs/nitem.h>
#include <ngrs/nitemevent.h>
#include <ngrs/nlabel.h>
#include <ngrs/nsystem.h>
#include <ngrs/nsplitbar.h>

namespace psycle { namespace host {


template<class T> inline T str_hex(const std::string &  value) {
   T result;

   std::stringstream str;
   str << value;
   str >> std::hex >> result;

   return result;
}


/// The pattern Main Class , a container for the inner classes LineNumber, Header, and PatternDraw
PatternView::PatternView( Song* song )
  : NPanel()
{
  _pSong = song;

  setLayout(NAlignLayout());


  vBar = new NScrollBar();
    vBar->setWidth(15);
    vBar->setOrientation(nVertical);
    vBar->posChange.connect(this,&PatternView::onVScrollBar);
  add(vBar, nAlRight);

  // create the patternview toolbar
  add(toolBar = new NToolBar() ,nAlTop);
  initToolBar();

  // create the left linenumber panel
  add(lineNumber_ = new LineNumber(this), nAlLeft);
	
	// sub group tweaker and patterndraw
	NPanel* drawGroup = new NPanel();
		drawGroup->setLayout( NAlignLayout() );
		// create tweak gui
		drawGroup->add(tweakGUI = new TweakGUI(this), nAlRight);
		// splitbar
		drawGroup->add( new NSplitBar(), nAlRight);
  	// create the pattern panel
		NPanel* subGroup = new NPanel();
			subGroup->setLayout( NAlignLayout() );
      // create the headertrack panel
      subGroup->add(header      = new Header(this), nAlTop);
			// hbar with beat zoom
  		NPanel* hBarPanel = new NPanel();
    		hBarPanel->setLayout( NAlignLayout() );
    		zoomHBar = new ZoomBar();
    		zoomHBar->setRange(1,100);
    		zoomHBar->setPos(4);
    		zoomHBar->posChanged.connect(this, &PatternView::onZoomHBarPosChanged);
    		hBarPanel->add(zoomHBar, nAlRight);
    		hBar = new NScrollBar();
      		hBar->setOrientation(nHorizontal);
      		hBar->setPreferredSize(100,15);
      		hBar->posChange.connect(this,&PatternView::onHScrollBar);
    		hBarPanel->add(hBar, nAlClient);
  		subGroup->add(hBarPanel, nAlBottom);
      // create the drawArea
      subGroup->add(drawArea    = new PatternDraw(this), nAlClient);
    drawGroup->add( subGroup, nAlClient );
	
	add(drawGroup, nAlClient);
	

  setFont(NFont("System",8,nMedium | nStraight ));

  addEvent(1); // add one byte event to trackerline
  addEvent(1); // add one byte event to trackerline
  addEvent(2); // add two byte event to trackerline

  editPosition_ = prevEditPosition_ = playPos_  = outtrack = 0;

  editOctave_ = 4;

  for(int i=0;i<MAX_TRACKS;i++) notetrack[i]=120;

  patternStep_ = 1;

  moveCursorWhenPaste_ = false;

  pattern_ = 0;

  selectedMacIdx_ = 255;
}

PatternView::~PatternView()
{
}

void PatternView::onZoomHBarPosChanged(ZoomBar* zoomBar, double newPos) {
  setBeatZoom( (int) newPos );
  repaint();
}

void PatternView::onHScrollBar( NObject * sender, int pos )
{
  int newPos = (pos / colWidth()) * colWidth();

  if (newPos != drawArea->dx()) {
    header->setScrollDx(newPos);
    header->repaint();

    int diffX  = newPos - drawArea->dx();
    if (diffX < drawArea->clientWidth()) {
      NRect rect = drawArea->blitMove(diffX,0, drawArea->absoluteSpacingGeometry());
      drawArea->setDx(newPos);
      window()->repaint(drawArea,rect);
    } else {
      drawArea->setDx(newPos);
      drawArea->repaint(drawArea);
    }
  }
}

void PatternView::onVScrollBar( NObject * sender, int pos )
{
  if (pos >= 0) {
  int newPos = (pos / rowHeight()) * rowHeight();

  if (newPos != drawArea->dy()) {
    int diffY  = newPos - lineNumber_->dy();
      if (diffY < drawArea->clientHeight()) {
        NRect rect = lineNumber_->blitMove(0,diffY,NRect(lineNumber_->absoluteSpacingLeft(),lineNumber_->absoluteSpacingTop()+headerHeight(),lineNumber_->spacingWidth(),lineNumber_->spacingHeight()-headerHeight()));
        lineNumber_->setDy(newPos);

        if (diffY < 0) {
           rect.setHeight(rect.height()+headerHeight());
        }

        window()->repaint(lineNumber_,rect);
        rect = drawArea->blitMove(0,diffY,drawArea->absoluteSpacingGeometry());
        drawArea->setDy(newPos);
        window()->repaint(drawArea,rect);
    } else {
        lineNumber_->setDy(newPos);
        drawArea->setDy(newPos);
        lineNumber_->repaint();
        drawArea->repaint();
    }
  }
  }
}

void PatternView::initToolBar( )
{
  meterCbx = new NComboBox();
    meterCbx->add(new NItem("4/4"));
    meterCbx->add(new NItem("3/4"));
    meterCbx->setPreferredSize(50,15);
    meterCbx->setIndex(0);
  toolBar->add(meterCbx);
  toolBar->add(new NButton("add Bar"))->clicked.connect(this,&PatternView::onAddBar);
  toolBar->add(new NButton("delete Bar"))->clicked.connect(this,&PatternView::onDeleteBar);
  toolBar->add(new NLabel("Pattern Step"));
  patternCombo_ = new NComboBox();
    for (int i = 1; i <=16; i++) 
      patternCombo_->add(new NItem(stringify(i)));
    patternCombo_->setIndex(0);
    patternCombo_->itemSelected.connect(this,&PatternView::onPatternStepChange);
    patternCombo_->setWidth(40);
    patternCombo_->setHeight(20);
  toolBar->add(patternCombo_);

  toolBar->add(new NLabel("Octave"));
  octaveCombo_ = new NComboBox();
    for (int i=0; i<9; i++) octaveCombo_->add(new NItem(stringify(i)));
    octaveCombo_->itemSelected.connect(this,&PatternView::onOctaveChange);
    octaveCombo_->setWidth(40);
    octaveCombo_->setHeight(20);
    octaveCombo_->setIndex(4);
    setEditOctave(4);
  toolBar->add(octaveCombo_);

  toolBar->add(new NLabel("Tracks"));
  trackCombo_ = new NComboBox();
    trackCombo_->setWidth(40);
    trackCombo_->setHeight(20);
    trackCombo_->itemSelected.connect(this,&PatternView::onTrackChange);
    for(int i=4;i<=MAX_TRACKS;i++) {
      trackCombo_->add(new NItem(stringify(i)));
    }
    trackCombo_->setIndex(12);  // starts at 4 .. so 16 - 4 = 12 ^= 16
  toolBar->add(trackCombo_);


}

void PatternView::onAddBar( NButtonEvent * ev )
{
  if ( pattern_ ) {
    std::string meter = meterCbx->text();
    int pos = meter.find("/");
    std::string num   = meter.substr( 0, pos);
    std::string denom = meter.substr( pos+1 );

    pattern_->addBar( TimeSignature(str<int>(num), str<int>(denom)) );
    resize();
    repaint();
  }
}

void PatternView::onDeleteBar(NButtonEvent* ev)
{
  if ( pattern_ ) {
    float position = cursor().y() / (float) pattern_->beatZoom();
    pattern_->removeBar(position);
    resize();
    repaint();
  }
}

void PatternView::resize( )
{
  // calls the AlignLayout to reorder scroll-,-header,-linenumber- and patternpanels
  NPanel::resize();
  // set the Range to the new headerwidth
  hBar->setRange(header->preferredWidth() - clientWidth());
  int count = (drawArea->clientHeight()-headerHeight()) / rowHeight();
  vBar->setRange((lineNumber()-1-count)*rowHeight());
}

void PatternView::setSeparatorColor( const NColor & separatorColor )
{
  separatorColor_ = separatorColor;
}

const NColor & PatternView::separatorColor( )
{
  return separatorColor_;
}

int PatternView::trackNumber( ) const
{
  return _pSong->tracks();
}

int PatternView::rowHeight( ) const
{
  return 12;
}

int PatternView::headerHeight( ) const
{
  return header->height()-1;
}

int PatternView::colWidth( ) const
{
  int col = noteCellWidth();
  for (std::vector<int>::const_iterator it = eventSize.begin(); it < eventSize.end(); it++) {
    switch (*it) {
    case 1: col+=2*cellWidth();
    break;
    case 2: col+=4*cellWidth();
    break;
    }
  }
  return std::max(header->skinColWidth(),col);
}

int PatternView::noteCellWidth( ) const
{
  NFontMetrics metrics(font());
  int width = metrics.textWidth("C#-10");
  return width;
}

int PatternView::cellWidth( ) const
{
  //NFontMetrics metrics(font());
  //int width = metrics.textWidth("_");
  return 12;
}

int PatternView::lineNumber( ) const
{
  return ( pattern_) ? pattern_->beatZoom() * pattern_->beats() : 0;  
}

int PatternView::playPos( ) const
{
  return playPos_;
}

void PatternView::setPrevEditPosition( int pos )
{
  prevEditPosition_ = pos;
}

int PatternView::prevEditPosition( ) const
{
  return prevEditPosition_;
}

const NPoint3D & PatternView::cursor( ) const
{
  return cursor_;
}

void PatternView::setCursor( const NPoint3D & cursor )
{
  cursor_ = cursor;
  lineChanged.emit(cursor_.y());
}

void PatternView::moveCursor( int dx, int dy, int dz )
{
  int newX = std::min(std::max(cursor_.x()+dx,0),trackNumber()-1);
  int newY = std::min(std::max(0,cursor_.y()+dy),lineNumber()-1);

  int newZ = cursor_.z()+dz;

  if (newZ >= cellCount() ) {
      newX++;
      if (newX > trackNumber()-1) {
        newX = trackNumber()-1;
        newZ = cursor_.z();
      }
      else 
        newZ=0;
  } else

  if (newZ < 0 ) {
      newX--;
      if (newX < 0) {
        newX = 0;
        newZ = 0;
      }
      else
        newZ= cellCount()-1;
  }

  setCursor(NPoint3D(newX,newY,newZ));
}

void PatternView::addEvent( int byteLength )
{
  eventSize.push_back(byteLength);
}

void PatternView::setEditPosition( int pos )
{
  editPosition_ = pos;
  int count = (drawArea->clientHeight()-headerHeight()) / rowHeight();
  vBar->setRange((lineNumber()-1-count)*rowHeight());
}

int PatternView::editPosition( ) const
{
  return editPosition_;
}

std::string PatternView::noteToString( int value )
{
  if (value==255) return "";
  switch (value) {
      case cdefTweakM: return "twk"; break;
      case cdefTweakE: return "twf"; break;
      case cdefMIDICC: return "mcm"; break;
      case cdefTweakS: return "tws"; break;
      case 120       : return "off"; break;
      case 255       : return "";    break;
  }

  int octave = value / 12;

  switch (value % 12) {
      case 0:   return "C-" + stringify(octave); break;
      case 1:   return "C#" + stringify(octave); break;
      case 2:   return "D-" + stringify(octave); break;
      case 3:   return "D#" + stringify(octave); break;
      case 4:   return "E-" + stringify(octave); break;
      case 5:   return "F-" + stringify(octave); break;
      case 6:   return "F#" + stringify(octave); break;
      case 7:   return "G-" + stringify(octave); break;
      case 8:   return "G#" + stringify(octave); break;
      case 9:   return "A-" + stringify(octave); break;
      case 10:  return "A#" + stringify(octave); break;
      case 11:  return "B-" + stringify(octave); break;
  }
  return "err";
}

NScrollBar * PatternView::vScrBar( )
{
  return vBar;
}

NScrollBar * PatternView::hScrBar( )
{
  return hBar;
}

void PatternView::setEditOctave( int octave )
{
  editOctave_ = octave;
}

int PatternView::editOctave( ) const
{
  return editOctave_;
}

void PatternView::setMoveCursorWhenPaste( bool on) {
  moveCursorWhenPaste_ = on;
}

bool PatternView::moveCursorWhenPaste() const {
  return moveCursorWhenPaste_;
}

/// End of PatternView main Class




/// The Header Panel Class
PatternView::Header::Header( PatternView * pPatternView ) : pView(pPatternView) , NPanel()
{
  setSkin();
  setHeight(bgCoords.height());
  skinColWidth_ = bgCoords.width();
  setBackground(Global::pConfig()->pvc_row);
  setTransparent(false);
}

PatternView::Header::~ Header( )
{
}

void PatternView::Header::setSkin( )
{
  bgCoords.setPosition(0,0,109,18);
  noCoords.setPosition(0,18,7,12);
  sRecCoords.setPosition(70,18,11,11);
  dRecCoords.setXY(52,3);
  sMuteCoords.setPosition(81,18,11,11);
  dMuteCoords.setXY(75,3);
  sSoloCoords.setPosition(92,18,11,11);
  dSoloCoords.setXY(97,3);
  dgX0Coords.setXY(24,3);
  dg0XCoords.setXY(31,3);
}

void PatternView::Header::paint( NGraphics * g )
{
  NBitmap & bitmap = Global::pConfig()->icons().pattern_header_skin();

  int startTrack = scrollDx() / pView->colWidth();
  int trackCount = spacingWidth() / pView->colWidth();

  g->setForeground(pView->separatorColor());
  for (int i = startTrack; i <= std::min(startTrack + trackCount ,pView->trackNumber() - 1); i++) {
    const int trackX0 = i/10;
    const int track0X = i%10;
    int xOff = i* pView->colWidth();
    int center = (pView->colWidth() - skinColWidth()) / 2;
    xOff += center;
    g->putBitmap(xOff,0,bgCoords.width(),bgCoords.height(), bitmap, 
                  bgCoords.left(), bgCoords.top());
    g->putBitmap(xOff+dgX0Coords.x(),0+dgX0Coords.y(),noCoords.width(),noCoords.height(), bitmap,
                  trackX0*noCoords.width(), noCoords.top());
    g->putBitmap(xOff+dg0XCoords.x(),0+dg0XCoords.y(),noCoords.width(),noCoords.height(), bitmap,
                  track0X*noCoords.width(), noCoords.top());

    // blit the mute LED
    if (  pView->pSong()->_trackMuted[i]) {
        g->putBitmap(xOff+dMuteCoords.x(),0+dMuteCoords.y(),sMuteCoords.width(),sMuteCoords.height(), bitmap,
                  sMuteCoords.left(), sMuteCoords.top());
    }

    // blit the solo LED
    if ( pView->pSong()->_trackSoloed == i) {
        g->putBitmap(xOff+dSoloCoords.x(),0+dSoloCoords.y(),sSoloCoords.width(),sSoloCoords.height(), bitmap,
                  sSoloCoords.left(), sSoloCoords.top());
    }

    // blit the record LED
    if ( pView->pSong()->_trackArmed[i]) {
        g->putBitmap(xOff+dRecCoords.x(),0+dRecCoords.y(),sRecCoords.width(),sRecCoords.height(), bitmap,
                  sRecCoords.left(), sRecCoords.top());
    }

    if (i!=0) g->drawLine(i*pView->colWidth(),0,i*pView->colWidth(),clientWidth()); // col seperator
  }
}

void PatternView::Header::onMousePress( int x, int y, int button )
{
    // check if left mouse button pressed
  if (button == 1)
  {
      // determine the track column, the mousepress occured on
      int track = x / pView->colWidth();
      // find out the start offset of the header bitmap
      NPoint off(track * pView->colWidth() + (pView->colWidth() - skinColWidth()) / 2,0);
      // the rect area of the solo led
      NRect solo(off.x() + dSoloCoords.x(), off.y() + dSoloCoords.y(), sSoloCoords.width(), sSoloCoords.height());
      // now check point intersection for solo
      if (solo.intersects(x,y)) {
          onSoloLedClick(track);
      } else {
        // the rect area of the solo led
        NRect mute(off.x() + dMuteCoords.x(), off.y() + dMuteCoords.y(), sMuteCoords.width(), sMuteCoords.height());
        // now check point intersection for solo
        if (mute.intersects(x,y)) {
            onMuteLedClick(track);
        } else
        {
            // the rect area of the record led
            NRect record(off.x() + dRecCoords.x(), off.y() + dRecCoords.y(), sRecCoords.width(), sRecCoords.height());
            // now check point intersection for solo
            if (record.intersects(x,y)) {
              onRecLedClick(track);
            }
        }
      }
  }
}

void PatternView::Header::onSoloLedClick( int track )
{
  if ( pView->pSong()->_trackSoloed != track )
  {
    for ( int i=0;i<MAX_TRACKS;i++ ) {
      pView->pSong()->_trackMuted[i] = true;
    }
    pView->pSong()->_trackMuted[track] = false;
    pView->pSong()->_trackSoloed = track;
  }
  else
  {
    for ( int i=0;i<MAX_TRACKS;i++ )
    {
      pView->pSong()->_trackMuted[i] = false;
    }
    pView->pSong()->_trackSoloed = -1;
  }
  repaint();
}

void PatternView::Header::onMuteLedClick( int track )
{
  pView->pSong()->_trackMuted[track] = !(pView->pSong()->_trackMuted[track]);
  repaint();
}

void PatternView::Header::onRecLedClick(int track) {
  pView->pSong()->_trackArmed[track] = ! pView->pSong()->_trackArmed[track];
  pView->pSong()->_trackArmedCount = 0;
  for ( int i=0;i<MAX_TRACKS;i++ ) {
      if ( pView->pSong()->_trackArmed[i] )
      {
        pView->pSong()->_trackArmedCount++;
      }
  }
  repaint();
}

int PatternView::Header::preferredWidth( )
{
  return (pView->trackNumber()+2)*pView->colWidth();
}

int PatternView::Header::skinColWidth( )
{
  return skinColWidth_;
}

/// End of Header Class


///
/// The Line Number Panel Class
///
  PatternView::LineNumber::LineNumber( PatternView * pPatternView ) : NPanel(), dy_(0)
  {
    setBorder( NFrameBorder() );
    pView = pPatternView;
    setWidth(60);
    setBackground(Global::pConfig()->pvc_row);
    setTransparent(false);
  }

  PatternView::LineNumber::~ LineNumber( )
  {
  }

  void PatternView::LineNumber::paint( NGraphics * g )
  {
    TimeSignature signature;
    int startLine = dy_ / pView->rowHeight();
    int rDiff   = g->repaintArea().rectClipBox().top() - absoluteTop() + pView->headerHeight();
    int offT = rDiff / pView->rowHeight();
    if (offT < 0) offT = 0;
    offT = 0;
    int offB = (rDiff+g->repaintArea().rectClipBox().height()) / pView->rowHeight();
    if (offB < 0) offB = 0;
    int count = std::min(clientHeight() / pView->rowHeight(),offB);

    for (int i = offT; i < count; i++)
      g->drawLine(0,i*pView->rowHeight()+pView->headerHeight(),
                  clientWidth(),i*pView->rowHeight()+pView->headerHeight());

    g->setForeground(pView->separatorColor());
    g->drawLine(0,0,0,clientHeight());
    g->drawLine(clientWidth()-1,0,clientWidth()-1,clientHeight());

      for (int i = offT; i < count; i++) {
        if (i+startLine == pView->cursor().y()) {
          g->setForeground(Global::pConfig()->pvc_cursor);
          g->fillRect(0,i*pView->rowHeight()+pView->headerHeight(),clientWidth()-1,pView->rowHeight());
        }
        std::string text = stringify(i+startLine);
        if ( pView->pattern() ) {
          float position = (i+ startLine) / (float) pView->pattern()->beatZoom();
          SinglePattern::iterator it = pView->pattern()->find_nearest(i+startLine);
          if (it != pView->pattern()->end()) {
            // check out how many hidden lines there are
            int lastLine = d2i (it->first * pView->pattern_->beatZoom());
            int y = lastLine;
            SinglePattern::iterator it2 = it;
            int count = 0;
            do {
              y = d2i (it2->first * pView->pattern_->beatZoom());
              if ( y != lastLine) break;
              it2++;
              count++;
            } while (it2 != pView->pattern()->end() && y == lastLine);

            if ( count > 1 ) {
              text+= std::string(":") + stringify( count );
            }
            // check if line is on beatzoom raster else draw arrow up or down hint
            if (it->first != position) {
              int xOff = clientWidth()-g->textWidth(text)- 10 ;
              int yOff = i*pView->rowHeight()+pView->rowHeight()+pView->headerHeight() - 3;
              g->drawLine( xOff , yOff+1, xOff, yOff - pView->rowHeight() + 5);
              if (it->first < position) {
                g->drawLine( xOff , yOff - pView->rowHeight() + 5, xOff-3, yOff - pView->rowHeight() + 8);
                g->drawLine( xOff , yOff - pView->rowHeight() + 5, xOff+4, yOff - pView->rowHeight() + 9);
              } else {
                 g->drawLine( xOff , yOff +1, xOff-3, yOff - 2);
                g->drawLine(  xOff , yOff +1, xOff+4, yOff - 3);
              }
            }
          }
          if ( pView->pattern()->barStart(position, signature) ) {
            std::string caption = stringify(signature.numerator())+"/"+stringify(signature.denominator());
            g->drawText(0,i*pView->rowHeight()+pView->rowHeight()+pView->headerHeight()-1,caption);
          }
        }
        g->drawText(clientWidth()-g->textWidth(text)-3,i*pView->rowHeight()+pView->rowHeight()+pView->headerHeight()-1,text);
      }

    g->drawText(1,pView->headerHeight()-1,"Line");
  }

  void PatternView::LineNumber::setDy( int dy ) {
    dy_ = dy;
  }

  int PatternView::LineNumber::dy( ) const {
    return dy_;
  }

/// End of Line Number Panel


PatternView::TweakHeader::TweakHeader() {

}

PatternView::TweakHeader::~TweakHeader() {

}

PatternView::TweakGUI::TweakGUI( PatternView* pPatternView)
{
}

PatternView::TweakGUI::~TweakGUI() {

}


///
///
/// The PatternDraw class, that displays the data
///
///


PatternView::PatternDraw::PatternDraw( PatternView * pPatternView ) : CustomPatternView()
{
  setTransparent(false);
  setBackground(Global::pConfig()->pvc_row);

  pView = pPatternView;
  editPopup_ = new NPopupMenu();
  add(editPopup_);
    editPopup_->add(new NMenuItem("Undo"));
    editPopup_->add(new NMenuItem("Redo"));
    editPopup_->add(new NMenuSeperator());
    NMenuItem* blockCutItem_ = new NMenuItem("Block cut");
      blockCutItem_->click.connect(this,&PatternView::PatternDraw::onPopupBlockCut);
    editPopup_->add(blockCutItem_);
    NMenuItem* blockCopyItem_ = new NMenuItem("Block copy");
      blockCopyItem_->click.connect(this,&PatternView::PatternDraw::onPopupBlockCopy);
    editPopup_->add(blockCopyItem_);
    NMenuItem* blockPasteItem_ = new NMenuItem("Block paste");
      blockPasteItem_->click.connect(this,&PatternView::PatternDraw::onPopupBlockPaste);
    editPopup_->add(blockPasteItem_);
    NMenuItem* blockPasteMixItem_ = new NMenuItem("Block mix paste");
      blockPasteMixItem_->click.connect(this,&PatternView::PatternDraw::onPopupBlockMixPaste);
    editPopup_->add(blockPasteMixItem_);

    NMenuItem* blockDelItem = new NMenuItem("Block delete");
        blockDelItem->click.connect(this,&PatternView::PatternDraw::onPopupBlockDelete);
    editPopup_->add(blockDelItem);

    editPopup_->add(new NMenuSeperator());
    editPopup_->add(new NMenuItem("Interpolate Effect"));
    editPopup_->add(new NMenuItem("Change Generator"));
    editPopup_->add(new NMenuItem("Change Instrument"));
    editPopup_->add(new NMenuSeperator());

    NMenuItem* blockT1Item = new NMenuItem("Transpose+1");
        blockT1Item->click.connect(this,&PatternView::PatternDraw::onPopupTranspose1);
    editPopup_->add(blockT1Item);

    NMenuItem* blockT_1Item = new NMenuItem("Transpose-1");
        blockT_1Item->click.connect(this,&PatternView::PatternDraw::onPopupTranspose_1);
    editPopup_->add(blockT_1Item);

    NMenuItem* blockT12Item = new NMenuItem("Transpose12");
        blockT12Item->click.connect(this,&PatternView::PatternDraw::onPopupTranspose12);
    editPopup_->add(blockT12Item);

    NMenuItem* blockT_12Item = new NMenuItem("Transpose-12");
        blockT_12Item->click.connect(this,&PatternView::PatternDraw::onPopupTranspose_12);
    editPopup_->add(blockT_12Item);

    editPopup_->add(new NMenuSeperator());

    editPopup_->add(new NMenuItem("Block Swing Fill"));
    editPopup_->add(new NMenuItem("Block Track Fill"));
    editPopup_->add(new NMenuSeperator());
    NMenuItem* blockPatPropItem_ = new NMenuItem("Pattern properties");
      blockPatPropItem_->click.connect(this,&PatternView::PatternDraw::onPopupPattern);
    editPopup_->add(blockPatPropItem_);

    patDlg = new PatDlg();
}

PatternView::PatternDraw::~ PatternDraw( )
{
}

int PatternView::PatternDraw::colWidth() const {
  return pView->colWidth();
}

int PatternView::PatternDraw::rowHeight() const {
	return pView->rowHeight();
}

int PatternView::PatternDraw::lineNumber() const {
	return pView->lineNumber();
}

int PatternView::PatternDraw::trackNumber() const {
	return pView->trackNumber();
}

void PatternView::PatternDraw::customPaint(NGraphics* g, int startLine, int endLine, int startTrack, int endTrack)
{
  if (pView->pattern()) {
    TimeSignature signature;

    g->setForeground(Global::pConfig()->pvc_rowbeat);

    int trackWidth = ((endTrack+1) * colWidth()) - dx();
    int lineHeight = ((endLine +1) * rowHeight()) - dy();

    for (int y = startLine; y <= endLine; y++) {
      float position = y / (float) pView->pattern()->beatZoom();
      if (!(y == pView->playPos()) /*|| pView->editPosition() != Global::pPlayer()->_playPosition*/) {
      if ( !(y % pView->beatZoom())) {
          if ((pView->pattern()->barStart(position, signature) )) {
              g->setForeground(Global::pConfig()->pvc_row4beat);
              g->fillRect(0, y*rowHeight() - dy(),trackWidth, rowHeight());
              g->setForeground(Global::pConfig()->pvc_rowbeat);
          } else {
            g->fillRect(0, y* rowHeight() - dy(),trackWidth, rowHeight());
          }
        }
      } else  {
        g->setForeground(Global::pConfig()->pvc_playbar);
        g->fillRect(0, y*rowHeight() - dy(), trackWidth, rowHeight());
        g->setForeground(Global::pConfig()->pvc_rowbeat);
      }
    }

    drawSelBg(g,selection());

    g->setForeground(pView->foreground());

    for (int y = startLine; y <= endLine; y++)
      g->drawLine(0,y* rowHeight() - dy(),trackWidth,y* rowHeight()-dy());

    for (int i = startTrack; i <= endTrack; i++) // 3px space at begin of trackCol
      g->fillRect(i*colWidth()-dx(),0,3,lineHeight);

    g->setForeground(pView->separatorColor());
    for (int i = startTrack; i <= endTrack; i++)  // col separators
      g->drawLine(i* colWidth()-dx(),0,i* colWidth()-dx(),lineHeight);

    g->setForeground(pView->foreground());

    for (int x = startTrack; x <= endTrack; x++) {
      int COL = pView->noteCellWidth();
      for (std::vector<int>::iterator it = pView->eventSize.begin(); it < pView->eventSize.end(); it++) {
        switch (*it) {
        case 1:
            g->drawLine(x*pView->colWidth()+COL-dx(),0,x*colWidth()+COL-dx(),lineHeight);
            COL+=2 * pView->cellWidth();
        break;
        case 2:
            g->drawLine(x*colWidth()+COL-dx(),0,x*colWidth()+COL-dx(),lineHeight);
            COL+=4 * pView->cellWidth();
        break;
        }
      }
    }

    drawPattern(g,startLine,endLine,startTrack,endTrack);
  }
}

void PatternView::PatternDraw::drawText(NGraphics* g, int track, int line, int eventOffset, const std::string & text )
{
  int xOff = track * pView->colWidth()+3 + eventOffset        - dx();
  int yOff = line  * pView->rowHeight() + pView->rowHeight()  - dy();

  g->drawText(xOff,yOff,text);
}

void PatternView::PatternDraw::drawData( NGraphics * g, int track, int line, int eventOffset, const std::string & text )
{
  int xOff = track * pView->colWidth()+3 + eventOffset        - dx();
  int yOff = line  * pView->rowHeight() + pView->rowHeight()  - dy();

  int col = 0;
  for (int i = 0; i < text.length(); i++) {
      g->drawText(xOff + col,yOff,text.substr(i,1));
      col += pView->cellWidth();
  }
}

void PatternView::PatternDraw::drawCellBg( NGraphics * g, int track, int line, int col, const NColor & bgColor )
{
  int xOff = track * pView->colWidth()+3 - dx();
  int yOff = line  * pView->rowHeight()  - dy();

  int cellWidth = pView->noteCellWidth()-3;
  int colOffset = 0;

  if (col>0) {
    cellWidth = pView->cellWidth();
    colOffset = pView->noteCellWidth()-3 + (col-1)*pView->cellWidth();
  }

  g->setForeground(bgColor);
  g->fillRect(xOff + colOffset,yOff, cellWidth,pView->rowHeight());
}


void PatternView::PatternDraw::drawPattern( NGraphics * g, int startLine, int endLine, int startTrack, int endTrack )
{
  if ( pView->pattern() ) {
   drawCellBg(g,pView->cursor().x(),pView->cursor().y(),pView->cursor().z(),Global::pConfig()->pvc_cursor );

  char tbuf[16];

  float position = startLine / (float) pView->pattern_->beatZoom();
  float endPosition = endLine / (float) pView->pattern_->beatZoom();

  SinglePattern::iterator it = pView->pattern_->find_lower_nearest(startLine);

  int lastLine = -1;
  for ( ; it != pView->pattern_->end(); it++ ) {
    PatternLine & line = it->second;

    int y = d2i (it->first * pView->pattern_->beatZoom());
    if (y > endLine) break;

    if (y != lastLine) {

    PatternLine::iterator eventIt = line.lower_bound(startTrack);
    for(; eventIt != line.end() && eventIt->first < endTrack; ++eventIt) {
      PatternEvent event = eventIt->second;
      int x = eventIt->first;
      drawText(g, x,y,0,pView->noteToString( event.note() ));
      unsigned char* patOffset = (unsigned char*) event.entry();
      patOffset++;
      int COL = pView->noteCellWidth();
      for (std::vector<int>::iterator it = pView->eventSize.begin(); it < pView->eventSize.end(); it++) {
      int value = *patOffset;
      switch (*it) {
        case 1  :{
                  sprintf(tbuf,"%.2X",value); 
                  if (value!=255) {
                    drawData(g,x,y,COL,tbuf);
                  }
                  COL+=2 * pView->cellWidth();
                  patOffset++;
                  } break;
        case 2  :{
                  if (*patOffset == 0 && *(patOffset+1) == 0 && (*(patOffset-3) <= 120 || *(patOffset-3) == 255 )) 
                  { patOffset+=2;} else {
                    sprintf(tbuf,"%.2X",value);
                    patOffset++;
                    int value = *patOffset;
                    sprintf(&tbuf[2],"%.2X",value);
                    if (value!=255) drawData(g,x,y,COL,tbuf);
                    patOffset++;
                  }
                  COL+=4 * pView->cellWidth();
                  } break;
      }
      }
    }
    }
    lastLine = y;
  }
  }
}


void PatternView::PatternDraw::onMousePress( int x, int y, int button )
{
  CustomPatternView::onMousePress( x, y, button );
  if (button == 6) {
      std::cout << "wheel mouse scroll left" << std::endl;
      // wheel mouse scroll left
      int startTrack  = dx() / pView->colWidth();
      int newTrack = std::max(0,startTrack-1);
      pView->hScrBar()->setPos( (newTrack) * pView->colWidth());
  } else
  if (button == 7) {
    // wheel mouse scrolling right
      std::cout << "wheel mouse scroll right" << std::endl;
      int startTrack  = dx() / pView->colWidth();
      int newTrack = std::max(0,std::min(pView->trackNumber(),startTrack+1));
      pView->hScrBar()->setPos( (newTrack) * pView->colWidth());
  } else
  if (button == 4) {
      // wheel mouse scroll up
      int startLine  = dy() / pView->rowHeight();
      int newLine = std::max(0,startLine-1);
      pView->vScrBar()->setPos( (newLine) * pView->rowHeight());
  } else
  if (button == 5) {
    // wheel mouse scrolling down
      int startLine  = dy() / pView->rowHeight();
      int newLine = std::max(0,std::min(pView->lineNumber(),startLine+1));
      pView->vScrBar()->setPos( (newLine) * pView->rowHeight());
  } else 
  if (button == 2) {
    // todo linux paste
  } else
  if (button == 3) {
      editPopup_->setPosition(x + absoluteLeft() + window()->left(), y + absoluteTop() + window()->top(),100,100);
      editPopup_->setVisible(true);
  }
}

void PatternView::PatternDraw::onMouseOver( int x, int y )
{
  CustomPatternView::onMouseOver( x, y);
}

char inline hex_value(char c) { if(c >= 'A') return 10 + c - 'A'; else return c - '0'; }

void PatternView::PatternDraw::onMousePressed( int x, int y, int button )
{
  if (button==1) {
    if ( !doSelect() ) pView->setCursor(intersectCell(x,y));
    pView->repaint();    
  }
  CustomPatternView::onMousePressed(x,y,button);
}

void PatternView::PatternDraw::onKeyPress( const NKeyEvent & event )
{
 ///\todo: Verify the correct usage of InputHandler->getEnumCodeByKey, concretely in relation to StateMasks.
  if ( !pView->pattern() ) return;

  if (doDrag() != (NApp::system().keyState() & ShiftMask) && 
                  !(NApp::system().keyState() & ControlMask)) {
      if (!doDrag() ) {
        clearOldSelection();
        startSel(pView->cursor());
        selCursor = pView->cursor();
        selCursor.setZ(0);
      }
  }

  switch (event.scancode()) {
    case XK_Tab : {
	int oldTrack = pView->cursor().x();
	int curLine = pView->cursor().y();
	//oddity.. this if never passes on my system. also, pressing shift-tab in the pattern view and holding shift is
	//equivalent to holding down the left mouse button..
        if (NApp::system().keyState() & ShiftMask) {
		pView->moveCursor(-1,0,0); std::cout<<"bam"<<std::endl;
		int newTrack = pView->cursor().x();
    repaintBlock( NSize( newTrack,curLine, oldTrack, curLine ) );
	} else {
		pView->moveCursor(1,0,0);
		int newTrack = pView->cursor().x();
		repaintBlock( NSize( oldTrack,curLine, newTrack, curLine ) );
	}
	window()->repaint(pView,pView->repaintLineNumberArea(curLine,curLine));
    }
    break;
    case XK_Insert : {
        int startLine = dy() / pView->rowHeight();
	int lastLine = pView->pattern()->beats()*pView->beatZoom();
        int lineCount = clientHeight() / pView->rowHeight();
        int curLine = pView->cursor().y();
	int curTrack = pView->cursor().x();

	double lineInc = 1.0f / (float)pView->beatZoom();

	SinglePattern::reverse_iterator it = pView->pattern()->rbegin();
	SinglePattern::reverse_iterator end
		= (SinglePattern::reverse_iterator
		( pView->pattern()->lower_bound(curLine*lineInc)) );

	for(; it != end; ++it)
	{
		double beatPos = it->first;
		PatternLine &curLine = it->second;
		if(curLine.count(curTrack))
		{
			PatternEvent &tempEvent(curLine[curTrack]);
			double newpos = beatPos + lineInc;
			if(newpos < pView->pattern()->beats())
				(*pView->pattern())[newpos][curTrack]=tempEvent;
			curLine.erase(curTrack);
		}
	}
	pView->pattern()->clearEmptyLines();

        window()->repaint(this,repaintTrackArea(curLine,lastLine,pView->cursor().x(),pView->cursor().x()));
        window()->repaint(pView,pView->repaintLineNumberArea(curLine,lastLine));
    }
    break;
    case XK_BackSpace : {
        pView->clearCursorPos();
        int startLine  = dy() / pView->rowHeight();
        int lineCount  = clientHeight() / pView->rowHeight();
        int oldLine = pView->cursor().y();
        pView->moveCursor(0,-1,0);
        int newLine = pView->cursor().y();
        window()->repaint(this,repaintTrackArea(newLine,oldLine,pView->cursor().x(),pView->cursor().x()));
        window()->repaint(pView,pView->repaintLineNumberArea(newLine,oldLine));

          if (newLine < startLine) {
            pView->vScrBar()->setPos( (newLine) * pView->rowHeight());
          }
    }
    break;
    case XK_Delete: {
        pView->clearCursorPos();
        int startLine  = dy() / pView->rowHeight();
        int lineCount  = clientHeight() / pView->rowHeight();
        int oldLine = pView->cursor().y();
        pView->moveCursor(0,1,0);
        int newLine = pView->cursor().y();
        window()->repaint(this,repaintTrackArea(oldLine,newLine,pView->cursor().x(),pView->cursor().x()));
        window()->repaint(pView,pView->repaintLineNumberArea(oldLine,newLine));

          if (newLine > startLine + lineCount-1) {
            pView->vScrBar()->setPos( (startLine+1) * pView->rowHeight());
          }
    }
    break;
    case XK_Left: {
        if (NApp::system().keyState() & ShiftMask) {
          // selMode
          selCursor.setX(std::max(selCursor.x()-1,0));
          doSel(selCursor);
        } else {
          int oldTrack = pView->cursor().x();
          pView->moveCursor(0,0,-1);
          int newTrack = pView->cursor().x();
          int startTrack  = dx() / pView->colWidth();
          if (newTrack < startTrack) {
              pView->hScrBar()->setPos( (newTrack) * pView->colWidth());
          }
        window()->repaint(this,repaintTrackArea(pView->cursor().y(),pView->cursor().y(),newTrack,oldTrack));
        }
    }
    break;
    case XK_Right: {
        if (NApp::system().keyState() & ShiftMask) {
          // selMode
          selCursor.setX(std::min(selCursor.x()+1,pView->trackNumber()-1));
          doSel(selCursor);
        } else {
          int oldTrack = pView->cursor().x();
          pView->moveCursor(0,0,1);
          int newTrack    = pView->cursor().x();
          int trackCount  = clientWidth() / pView->colWidth();
          int startTrack  = dx() / pView->colWidth();
          if (newTrack > startTrack + trackCount -1) {
            pView->hScrBar()->setPos( (startTrack+2) * pView->colWidth());
          }
          window()->repaint(this,repaintTrackArea(pView->cursor().y(),pView->cursor().y(),oldTrack,newTrack));
        }
      }
      break;
      case XK_Down : {
        if (NApp::system().keyState() & ShiftMask) {
          // selMode
          selCursor.setY(std::min(selCursor.y()+1,pView->lineNumber()-1));
          doSel(selCursor);
        } else {
          int startLine  = dy() / pView->rowHeight();
          int lineCount  = clientHeight() / pView->rowHeight();
          int oldLine = pView->cursor().y();
          pView->moveCursor(0,pView->patternStep(),0);
          // first clear old line to avoid flicker at scroll down
          window()->repaint(this,repaintTrackArea(oldLine,oldLine,pView->cursor().x(),pView->cursor().x()));
          window()->repaint(pView,pView->repaintLineNumberArea(oldLine,oldLine));
          int newLine = pView->cursor().y();
          if (newLine > startLine + lineCount-1) {
            pView->vScrBar()->setPos( (startLine+1) * pView->rowHeight());
          }
            window()->repaint(this,repaintTrackArea(newLine,newLine,pView->cursor().x(),pView->cursor().x()));
            window()->repaint(pView,pView->repaintLineNumberArea(newLine,newLine));

        }
      }
      break;
      case XK_Up: {
        if (NApp::system().keyState() & ShiftMask) {
          // selMode
          selCursor.setY(std::max(selCursor.y()-1,0));
          doSel(selCursor);
        } else {
          int oldLine = pView->cursor().y();
          pView->moveCursor(0,-pView->patternStep(),0);
          int startLine  = dy() / pView->rowHeight();
          int newLine = pView->cursor().y();
          // frist clear oldLine to avoid flicker at bitblit
          window()->repaint(this,repaintTrackArea(oldLine,oldLine,pView->cursor().x(),pView->cursor().x()));
          window()->repaint(pView,pView->repaintLineNumberArea(oldLine,oldLine));
          if (newLine < startLine) {
            pView->vScrBar()->setPos( (newLine) * pView->rowHeight());
          }
          window()->repaint(this,repaintTrackArea(newLine,newLine,pView->cursor().x(),pView->cursor().x()));
          window()->repaint(pView,pView->repaintLineNumberArea(newLine,newLine));
        }
      }
      break;
      case XK_Page_Up: {
        int lines = (pView->pSong()->LinesPerBeat()*Global::pConfig()->pv_timesig);

        int oldLine = pView->cursor().y();
        pView->moveCursor(0,-lines,0);
        int startLine  = dy() / pView->rowHeight();
        int newLine = pView->cursor().y();
        if (newLine <= startLine) {
            pView->vScrBar()->setPos( (newLine) * pView->rowHeight());
        }
        window()->repaint(this,repaintTrackArea(newLine,oldLine,pView->cursor().x(),pView->cursor().x()));
        window()->repaint(pView,pView->repaintLineNumberArea(newLine,oldLine));
      }
      break;
      case XK_Page_Down:{
        int lines = (pView->pSong()->LinesPerBeat()*Global::pConfig()->pv_timesig);

        int startLine  = dy() / pView->rowHeight();
        int lineCount  = clientHeight() / pView->rowHeight();
        int oldLine = pView->cursor().y();
        pView->moveCursor(0,lines,0);
        int newLine = pView->cursor().y();
        if (newLine > startLine + lineCount-1) {
            pView->vScrBar()->setPos( (startLine+2) * pView->rowHeight());
        }
        window()->repaint(this,repaintTrackArea(oldLine,newLine,pView->cursor().x(),pView->cursor().x()));
        window()->repaint(pView,pView->repaintLineNumberArea(oldLine,newLine));
      }
      break;
      default: {
          switch
            (Global::pConfig()->inputHandler.getEnumCodeByKey(Key(NApp::system().keyState() & ControlMask,event.scancode()))) 
          {
            case cdefBlockCopy :
                copyBlock(false);
            break;
            case cdefBlockDelete :
                deleteBlock();
                repaint();
            break;
            case cdefBlockPaste :
                pasteBlock(pView->cursor().x(),pView->cursor().y(),false);
            break;
            case cdefBlockMix :
                pasteBlock(pView->cursor().x(),pView->cursor().y(),true);
            break;
            case cdefTransposeBlockInc:
                transposeBlock(1);
            break;
            case cdefTransposeBlockInc12:
                transposeBlock(12);
            break;
            case cdefTransposeBlockDec:
                transposeBlock(-1);
            break;
            case cdefTransposeBlockDec12:
                transposeBlock(-12);
            break;
            case cdefBlockDouble:
                scaleBlock(2.0f);
            break;
            case cdefBlockHalve:
                scaleBlock(0.5f);
            break;
            case cdefRowClear:
                pView->clearCursorPos();
            break;

          default: {
              if (event.buffer()!="") {
              int note = Global::pConfig()->inputHandler.getEnumCodeByKey(Key(0,event.scancode()));

              if (note == cdefKeyStop && pView->cursor().z()==0) pView->noteOffAny(); else
              {
                  if (pView->cursor().z()==0) {
                    int startLine  = dy() / pView->rowHeight();
                    int lineCount  = clientHeight() / pView->rowHeight();
                    int oldLine = pView->cursor().y();

                    pView->enterNote( note );
                    pView->moveCursor(0,1,0);
                    int newLine = pView->cursor().y();
                    if (newLine > startLine + lineCount-1) {
                      pView->vScrBar()->setPos( (startLine+2) * pView->rowHeight());
                    }
                  window()->repaint(this,repaintTrackArea(oldLine,newLine,pView->cursor().x(),pView->cursor().x()));
                  window()->repaint(pView,pView->repaintLineNumberArea(oldLine,newLine));
                } else  {

                  int off = (pView->cursor().z()+1) / 2;
                  float position = pView->cursor().y() / (float) pView->pattern_->beatZoom();

                    PatternEvent data = (*pView->pattern_)[position][pView->cursor().x()];
                  unsigned char* patOffset = (unsigned char*) data.entry() + off;
                  unsigned char newByte;
                  if (pView->cursor().z() % 2 == 1) 
                    newByte = (*patOffset & 0x0F) | (0xF0 & (hex_value(event.scancode()) << 4));
                  else
                    newByte = (*patOffset & 0xF0) | (0x0F & (hex_value(event.scancode())));

                  if (*patOffset == 255) {
                    // set to 0
                      newByte = (*patOffset & 0x00) | (0xF0 & (hex_value(event.scancode()) << 4));
                  }
                  *patOffset = newByte;
                  (*pView->pattern_)[position][pView->cursor().x()]=data;

                    int eventIdx      =  pView->eventFromCol(pView->cursor().z());
                    int eventLen      =  pView->eventLength(eventIdx);
                    int eventColStart =  pView->colStartFromEvent(eventIdx);

                    int startLine  = dy() / pView->rowHeight();
                    int lineCount  = clientHeight() / pView->rowHeight();
                    int oldLine = pView->cursor().y();

                    if (pView->cursor().z() - eventColStart >= 2*eventLen - 1) {
                      pView->moveCursor(0,1,0);
                      pView->setCursor(NPoint3D(pView->cursor().x(),pView->cursor().y(),eventColStart));
                    } else
                        pView->moveCursor(0,0,1);

                    int newLine = pView->cursor().y();
                    if (newLine > startLine + lineCount-1) {
                      pView->vScrBar()->setPos( (startLine+2) * pView->rowHeight());
                    }
                    window()->repaint(this,repaintTrackArea(oldLine,newLine,pView->cursor().x(),pView->cursor().x()));
                    window()->repaint(pView,pView->repaintLineNumberArea(oldLine,newLine));
                }
            }
          }
      }}}
      break;
  }
}

void PatternView::enterNote( int note ) {
 if ( pattern() ) {
   PatternEvent event = pattern()->event( cursor().y(), cursor().x() );

   Machine* tmac = pSong()->_pMachine[ pSong()->seqBus ];

   event.setNote( editOctave() * 12 + note );
   if (tmac) event.setMachine( tmac->_macIndex );

   if (tmac && tmac->_type == MACH_SAMPLER ) {
     event.setInstrument( pSong()->instSelected );
	 }

   pattern()->setEvent( cursor().y(), cursor().x(), event );

   if (tmac) PlayNote( editOctave() * 12 + note, 127, false, tmac);

   
 }
}



NRect PatternView::repaintLineNumberArea(int startLine, int endLine)
{
  int top    = startLine    * rowHeight()  + drawArea->absoluteTop()  - drawArea->dy();
  int bottom = (endLine+1)  * rowHeight()  + drawArea->absoluteTop()  - drawArea->dy();

  return NRect(lineNumber_->absoluteLeft(),top,lineNumber_->clientWidth(),bottom - top);
}

int PatternView::cellCount( ) const
{
  int count = 1;
  for (std::vector<int>::const_iterator it = eventSize.begin(); it < eventSize.end(); it++)
    switch (*it) {
        case 1  : count+=2; break;
        case 2  : count+=4; break;
    }
  return count;
}


void PatternView::PatternDraw::drawSelBg( NGraphics * g, const NSize & selArea )
{
  int x1Off = selArea.left() * pView->colWidth()  ;
  int y1Off = selArea.top()  * pView->rowHeight() ;

  int x2Off = selArea.right()  * pView->colWidth() ;
  int y2Off = selArea.bottom() * pView->rowHeight();

  g->setForeground(Global::pConfig()->pvc_selection);
  g->fillRect(x1Off - dx(), y1Off -dy(), x2Off-x1Off, y2Off-y1Off);

}

NPoint3D PatternView::PatternDraw::intersectCell( int x, int y )
{
  int track = ( x + dx() ) / pView->colWidth();
  int line  = ( y + dy() ) / pView->rowHeight();

  int colOff   = ( x + dx() ) -  (track*pView->colWidth() + pView->noteCellWidth() );
  int cell = 0;
  if (colOff >= 0) cell = colOff / pView->cellWidth() + 1;

  return NPoint3D(track,line,cell);
}


void PatternView::PatternDraw::onPopupBlockCopy( NButtonEvent * ev )
{
  copyBlock(false);
}

void PatternView::PatternDraw::onPopupBlockCut( NButtonEvent * ev )
{
  copyBlock(true);
}

void PatternView::PatternDraw::copyBlock( bool cutit )
{  
	isBlockCopied=true;
	pasteBuffer.clear();
	SinglePattern copyPattern = pView->pattern()->block( selection().left(), selection().right(), selection().top(), selection().bottom() );
	
	float start = selection().top()    / (float) pView->pattern()->beatZoom();
	float end   = selection().bottom() / (float) pView->pattern()->beatZoom();

  std::string xml = "<patsel beats='" + stringify( end - start ); 
	xml+= "' tracks='"+ stringify( selection().right() - selection().left() );
	xml+= "'>"; 
	xml+= copyPattern.toXml();
  xml+= "</patsel>";

	NApp::system().clipBoard().setAsText( xml );


	if (cutit) {
		pView->pattern()->deleteBlock( selection().left(), selection().right(), selection().top(), selection().bottom() );
		pView->repaint();
	}
}


void PatternView::PatternDraw::onPopupBlockDelete( NButtonEvent * ev )
{
  deleteBlock();
  repaint();
}

void PatternView::PatternDraw::onPopupBlockMixPaste( NButtonEvent * ev )
{
  pasteBlock(pView->cursor().x(),pView->cursor().y(),true);

  if (pView->moveCursorWhenPaste()) {
/*       if (pView->cursor().y()+ blockNLines < pView->lineNumber() ) {
          pView->setCursor(NPoint3D(pView->cursor().x(),pView->cursor().y()+blockNLines,pView->cursor().z()));
       } else
       pView->setCursor(NPoint3D(pView->cursor().x(),pView->lineNumber()-1,pView->cursor().z()));*/
  }
  pView->repaint();
}

void PatternView::PatternDraw::onPopupBlockPaste( NButtonEvent * ev )
{
  pasteBlock(pView->cursor().x(),pView->cursor().y(),false);

  if (pView->moveCursorWhenPaste()) {
/*       if (pView->cursor().y()+ blockNLines < pView->lineNumber() ) {
          pView->setCursor(NPoint3D(pView->cursor().x(),pView->cursor().y()+blockNLines,pView->cursor().z()));
       } else
       pView->setCursor(NPoint3D(pView->cursor().x(),pView->lineNumber()-1,pView->cursor().z()));*/
  }
  pView->repaint();
}

void PatternView::PatternDraw::pasteBlock(int tx,int lx,bool mix,bool save)
{
  if ( NApp::system().clipBoard().asText() != "" ) {
    NXmlParser parser;
		lastXmlLineBeatPos = 0.0;
		xmlTracks = 0;
		xmlBeats = 0;
    parser.tagParse.connect(this, &PatternView::PatternDraw::onTagParse);
    parser.parseString( NApp::system().clipBoard().asText() );
	
		if (!mix)
			pView->pattern()->copyBlock(tx,lx,pasteBuffer,xmlTracks,xmlBeats);
		else
			pView->pattern()->mixBlock(tx,lx,pasteBuffer,xmlTracks,xmlBeats);
  }
}

void PatternView::updatePlayBar(bool followSong)
{
/*  if ( ((NVisualComponent*) parent())->visible() && (Global::pPlayer()->_lineChanged) && (editPosition() == Global::pPlayer()->_playPosition) && !followSong )
  {
      int trackCount  = clientWidth() / colWidth();
      int startTrack  = drawArea->dx() / colWidth();

      int oldPlayPos = playPos_;
      playPos_ = Global::pPlayer()->_lineCounter;
      if (oldPlayPos < playPos_)
        
        window()->repaint(drawArea,drawArea->repaintTrackArea(oldPlayPos,playPos_,startTrack,trackCount+startTrack));
      else if (oldPlayPos > playPos_) {
        window()->repaint(drawArea,drawArea->repaintTrackArea(oldPlayPos,oldPlayPos,startTrack,trackCount+startTrack));
        window()->repaint(drawArea,drawArea->repaintTrackArea(playPos_,playPos_,startTrack,trackCount+startTrack));
      }
  } else 
  if (visible() && followSong) {

    if (editPosition() != Global::pPlayer()->_playPosition) {
      setEditPosition(Global::pPlayer()->_playPosition);
      playPos_ = Global::pPlayer()->_lineCounter;
      int startLine  = playPos_;
      vScrBar()->posChange.disconnect_all();
      vScrBar()->setPos( (startLine) * rowHeight());
      vBar->posChange.connect(this,&PatternView::onVScrollBar);
      drawArea->setDy(startLine * rowHeight());
      lineNumber_->setDy(startLine * rowHeight());
      repaint();
    } else
    if ((Global::pPlayer()->_lineChanged) && editPosition() == Global::pPlayer()->_playPosition) {
      int startLine  = drawArea->dy() / rowHeight();
      int lineCount  = drawArea->clientHeight() / rowHeight();
      int oldLine = playPos_;
      playPos_ = Global::pPlayer()->_lineCounter;

      if (oldLine < playPos_) {
        int newLine = playPos_;
        if (newLine > startLine + lineCount-1) {
          vScrBar()->setPos( (startLine+1) * rowHeight());
        }
        int trackCount  = drawArea->clientWidth() / colWidth();
        int startTrack  = drawArea->dx() / colWidth();

        window()->repaint(this,drawArea->repaintTrackArea(oldLine,newLine,startTrack,trackCount+startTrack));
      } else if (playPos_ < oldLine) {
        vScrBar()->setPos(0);
      }
    }

  }*/
}


void PatternView::PlayNote(int note,int velocity,bool bTranspose,Machine*pMachine)
{
    // stop any notes with the same value
    //StopNote(note,bTranspose,pMachine);

    if(note<0) return;

    // octave offset
    if(note<120) {
        if(bTranspose)
        note+=pSong()->currentOctave*12;
        if (note > 119)
        note = 119;
      }

      // build entry
      PatternEvent entry;
      entry.setNote( note );
      entry.setInstrument( pSong()->auxcolSelected );
      entry.setMachine( pSong()->seqBus );	// Not really needed.

      if(velocity != 127 && Global::pConfig()->midi().velocity().record())
      {
          int par = Global::pConfig()->midi().velocity().from() + (Global::pConfig()->midi().velocity().to() - Global::pConfig()->midi().velocity().from()) * velocity / 127;
          if (par > 255) par = 255; else if (par < 0) par = 0;
          switch(Global::pConfig()->midi().velocity().type())
          {
            case 0:
              entry.setCommand( Global::pConfig()->midi().velocity().command() );
              entry.setParameter( par );
            break;
            case 3:
              entry.setInstrument( par );
            break;
          }
      } else
      {
          entry.setCommand( 0 );
          entry.setParameter( 0 );
      }

      // play it
      if(pMachine==NULL)
      {
        int mgn = pSong()->seqBus;

        if (mgn < MAX_MACHINES) {
            pMachine = pSong()->_pMachine[mgn];
        }
      }

      if (pMachine) {
        // pick a track to play it on	
//        if(bMultiKey)
        {
          int i;
          for (i = outtrack+1; i < pSong()->tracks(); i++)
          {
            if (notetrack[i] == 120) {
              break;
            }
          }
          if (i >= pSong()->tracks()) {
            for (i = 0; i <= outtrack; i++) {
                if (notetrack[i] == 120) {
                  break;
                }
            }
          }
          outtrack = i;
        }// else  {
          // outtrack=0;
        // }
        // this should check to see if a note is playing on that track
        if (notetrack[outtrack] < 120) {
            //StopNote(notetrack[outtrack], bTranspose, pMachine);
        }

        // play
        notetrack[outtrack]=note;
        pMachine->Tick(outtrack, entry );
      }
}



void PatternView::PatternDraw::transposeBlock(int trp)
{
	int right = selection().right();
	int left =  selection().left();
	double top = selection().top() / (double)pView->beatZoom();
	double bottom = selection().bottom() / (double)pView->beatZoom();

	pView->pattern()->transposeBlock(left, right, top, bottom, trp);

	window()->repaint(this,repaintTrackArea(selection().top(),selection().bottom(),left,right));
//   }
}


void PatternView::noteOffAny()
{
  if (pattern_) {
    PatternEvent event;
    event.setNote(120);
    pattern()->setEvent( cursor().y(), cursor().x(), event );

    window()->repaint(drawArea,drawArea->repaintTrackArea(cursor().y(),cursor().y(),cursor().x(),cursor().x()));
  }
}


void PatternView::clearCursorPos( )
{
  if ( pattern() ) {
    pattern_->clearPosition( cursor().y(), cursor().x(), cursor().z() );
  }
}


int PatternView::eventFromCol(int col) const {
  int count = 0; int index = 0;
  for (std::vector<int>::const_iterator it = eventSize.begin(); it < eventSize.end(); it++) {
    switch (*it) {
        case 1  : count+=2; break;
        case 2  : count+=4; break;
    }
    if (count >= col) return index;
    index++;
  }
  return -1;  // out of range
}


int PatternView::colStartFromEvent( int event )
{
  int col = 1; 
  int count = 0;
  for (std::vector<int>::iterator it = eventSize.begin(); it < eventSize.end(); it++) {
    if (count >= event) return col;
    switch (*it) {
        case 1  : col+=2; break;
        case 2  : col+=4; break;
    }
    count++;
  }
  return -1;  // out of range
}


int PatternView::eventLength(int event) const {
  if (event >= 0 && event < eventSize.size()) return eventSize.at(event); else return -1;
}

void PatternView::PatternDraw::onPopupPattern( NButtonEvent * ev )
{
}

void PatternView::PatternDraw::onPopupTranspose1( NButtonEvent * ev )
{
	transposeBlock(1);
}

void PatternView::PatternDraw::onPopupTranspose12( NButtonEvent * ev )
{
	transposeBlock(12);
}

void PatternView::PatternDraw::onPopupTranspose_1( NButtonEvent * ev )
{
	transposeBlock(-1);
}

void PatternView::PatternDraw::onPopupTranspose_12( NButtonEvent * ev )
{
	transposeBlock(-12);
}


void PatternView::PatternDraw::onKeyRelease(const NKeyEvent & event) {
	if ( !pView->pattern() ) return;

  if ( event.scancode() == XK_Shift_L || event.scancode() == XK_Shift_R )
 		endSel();

  if (pView->cursor().z()==0) {
    int outnote = Global::pConfig()->inputHandler.getEnumCodeByKey(Key(0,event.scancode()));
    pView->StopNote(outnote);
  }
}

void PatternView::copyBlock( bool cutit )
{
  drawArea->copyBlock(cutit);
}

void PatternView::pasteBlock( int tx, int lx, bool mix, bool save )
{
  drawArea->pasteBlock(tx,lx,mix,save);
}

void PatternView::blockTranspose( int trp )
{
  drawArea->transposeBlock(trp);
}

void PatternView::PatternDraw::deleteBlock( )
{
    int right = selection().right();
    int left = selection().left();
    double top = selection().top() / (double) pView->beatZoom();
    double bottom = selection().bottom() / (double) pView->beatZoom();

    pView->pattern()->deleteBlock(left, right, top, bottom);
    repaint();
}

void PatternView::deleteBlock( )
{
  drawArea->deleteBlock();
}

void PatternView::setPatternStep( int step )
{
  patternStep_ = step;
}

int PatternView::patternStep() const {
  return patternStep_;
}

void PatternView::StopNote( int note, bool bTranspose, Machine * pMachine )
{
  if (!(note >=0 && note < 128)) return;

  // octave offset
  if(note<120) {
      if(bTranspose) note+=pSong()->currentOctave*12;
      if (note > 119) note = 119;
  }

  if(pMachine==NULL) {
      int mgn = pSong()->seqBus;

      if (mgn < MAX_MACHINES) {
          pMachine = pSong()->_pMachine[mgn];
      }

  for(int i=0; i<pSong()->tracks(); i++) {
      if(notetrack[i]==note) {
        notetrack[i]=120;
        // build entry
        PatternEvent entry;
        entry.setNote( 120+0 );
        entry.setInstrument( pSong()->auxcolSelected );
        entry.setMachine( pSong()->seqBus );
        entry.setCommand( 0 );
        entry.setParameter( 0 );

        // play it

        if (pMachine) {
          pMachine->Tick( i, entry );
        }
      }
    }

  }
}

void PatternView::PatternDraw::scaleBlock(float factor )
{
	int right = selection().right();
	int left =  selection().left();
	double top = selection().top() / (double)pView->beatZoom();
	double bottom = selection().bottom() / (double)pView->beatZoom();
	pView->pattern()->scaleBlock(left, right, top, bottom, factor);
	window()->repaint(this,repaintTrackArea(selection().top(),selection().bottom()*std::max(factor,1.0f),left,right));
	pView->pattern()->clearEmptyLines();
}

void PatternView::doubleLength( )
{
  drawArea->scaleBlock(2.0f);
}
void PatternView::halveLength()
{
  drawArea->scaleBlock(0.5f);
}

int PatternView::beatZoom( ) const
{
  if (pattern_)
     return pattern_->beatZoom();
  else
     return 4;
}

void PatternView::setPattern( SinglePattern * pattern )
{
  pattern_ = pattern;
  resize();
}

SinglePattern * PatternView::pattern( )
{
  return pattern_;
}

Song * PatternView::pSong( )
{
  return _pSong;
}

void PatternView::setBeatZoom( int tpb )
{
  if (pattern_) {
     pattern_->setBeatZoom( std::max(tpb, 1) );
     int count = (drawArea->clientHeight() - headerHeight()) / rowHeight();
     vBar->setRange( ( lineNumber() - 1 - count) * rowHeight());
  }
}

void PatternView::onOctaveChange( NItemEvent * ev )
{
  setEditOctave( str<int> ( ev->item()->text() ) );
}

void PatternView::onTrackChange( NItemEvent * ev )
{
  pSong()->setTracks( str<int>( ev->item()->text() ) );
  if (cursor().x() >= pSong()->tracks() )
  {
    setCursor(NPoint3D(pSong()->tracks() ,cursor().y(),0));
  }
  repaint();
}

}}

void psycle::host::PatternView::setActiveMachineIdx( int idx )
{
  selectedMacIdx_ = idx;
}

int psycle::host::PatternView::selectedMachineIndex( ) const
{
  return selectedMacIdx_;
}

void psycle::host::PatternView::onPatternStepChange( NItemEvent * ev )
{
  if (patternCombo_->selIndex()!=-1) {
      setPatternStep(patternCombo_->selIndex()+1);
  }
}

void psycle::host::PatternView::PatternDraw::onTagParse( const NXmlParser & parser, const std::string & tagName  )
{
	if (tagName == "patsel") {
		xmlTracks = str<int>   (parser.getAttribValue("tracks"));
		xmlBeats = str<float> (parser.getAttribValue("beats"));
		std::cout << "tracks:" << xmlTracks << std::endl;
		std::cout << "beats:" << xmlBeats << std::endl;
	} else
  if (tagName == "patline") {
			lastXmlLineBeatPos = str<float> (parser.getAttribValue("pos"));     
	} else
	if ( tagName == "patevent" && pView->pattern() ) {
		int trackNumber = str_hex<int> (parser.getAttribValue("track"));

		PatternEvent data;
		data.setMachine( str_hex<int> (parser.getAttribValue("mac")) );
		data.setInstrument( str_hex<int> (parser.getAttribValue("inst")) );
		data.setNote( str_hex<int> (parser.getAttribValue("note")) );
		data.setParameter( str_hex<int> (parser.getAttribValue("param")) );
		data.setParameter( str_hex<int> (parser.getAttribValue("cmd")) );

		pasteBuffer[lastXmlLineBeatPos][trackNumber]=data;
	}
}
