/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper  *
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
#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "panel.h"
#include "fontmetrics.h"

namespace ngrs {

  class Label;

  /**
  @author  Stefan
  */

  class NCheckBox : public Panel
  {
  public:
    NCheckBox();
    NCheckBox(const std::string & text);

    ~NCheckBox();

    void setText(const std::string & text);
    const std::string & text() const;

    virtual void paint(Graphics& g);

    virtual void onMousePress(int x, int y, int button);
    virtual void onMousePressed (int x, int y, int button);

    void setCheck(bool on);
    bool checked() const;

    void setWordWrap(bool on);
    bool wordWrap() const;


    virtual int preferredWidth() const;
    virtual int preferredHeight() const;

    virtual void resize();

    signal1<ButtonEvent*> clicked;

  private:

    Label* label_;
    int dx,dy;
    std::string text_;
    bool checked_;
    FontMetrics metrics;

    void init();
    void drawCheck(Graphics& g);
  };

}

#endif
