/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#include "ncustombutton.h"
#include "nlabel.h"
#include "nbevelborder.h"
#include "ngradient.h"
#include "nproperty.h"

NCustomButton::NCustomButton()
 : NPanel()
{
  init();
}

NCustomButton::NCustomButton( const std::string & text ) : NPanel()
{
  init();
  label_->setText(text);
}

void NCustomButton::init( )
{
  toggle_ = down_ = false;

  setTransparent(false);

  label_ = new NLabel();
    label_->setSpacing(4,2,4,2);
    label_->setHAlign(nAlCenter);
    label_->setVAlign(nAlCenter);
    label_->setEvents(false);
  add(label_);

  resize();

    // runtime
//  if (properties()) properties()->registrate<std::string>("text", *this, &NCustomButton::text, &NCustomButton::setText);
}

NCustomButton::~NCustomButton()
{

}


void NCustomButton::setText( const std::string & text )
{
  label_->setText(text);
}


void NCustomButton::resize( )
{
  label_->setPosition(0,0,spacingWidth(),spacingHeight());
}

int NCustomButton::preferredWidth( ) const
{
  return label_->preferredWidth() + spacing().left()+spacing().right()+borderLeft()+borderRight();
}

int NCustomButton::preferredHeight( ) const
{
  return label_->preferredHeight() + spacing().top()+spacing().bottom()+borderTop()+borderBottom();
}

void NCustomButton::onMousePress( int x, int y, int button )
{
  setDown(!down_);
  NButtonEvent ev(this,x,y,button,"btnpress");
  click.emit(&ev);
  sendMessage(&ev);
}

void NCustomButton::onMousePressed( int x, int y, int button )
{
  NPanel::onMousePressed(x,y,button);
  if (!toggle_) {
    setDown(false);
  }
  if (NRect(0,0,width(),height()).intersects(x,y)) {
    NButtonEvent ev(this,x,y,button,"btnpressed");
    clicked.emit(&ev);
    sendMessage(&ev);
  }
}


const std::string & NCustomButton::text( ) const
{
  return label_->text();
}

char NCustomButton::mnemonic( )
{
  return label_->mnemonic();
}

void NCustomButton::setMnemonic(char c )
{
  label_->setMnemonic(c);
}


bool NCustomButton::toggle( ) const
{
  return toggle_;
}

bool NCustomButton::down( ) const
{
  return down_;
}

void NCustomButton::setToggle( bool on )
{
  toggle_ = on;
}

void NCustomButton::setDown( bool on )
{
  down_ = on;
}

void NCustomButton::onMessage( NEvent * ev )
{
  if (toggle() && ev->text() == "toggle:'up'") {
    setDown(false);
  }
}

void NCustomButton::setTextHAlign( int align )
{
  label_->setHAlign(align);
}

void NCustomButton::setTextVAlign( int align )
{
  label_->setVAlign(align);
}

NLabel * NCustomButton::label( )
{
  return label_;
}

NLabel * NCustomButton::label( ) const
{
  return label_;
}



