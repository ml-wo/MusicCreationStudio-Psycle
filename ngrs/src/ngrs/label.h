/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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
#ifndef LABEL_H
#define LABEL_H

#include "visualcomponent.h"
#include <string>
#include "fontmetrics.h"

namespace ngrs {

  class RectShape;

  /**
  @author  Stefan
  */
  class Label : public VisualComponent
  {
  public:
    Label();
    Label( const std::string & text );

    ~Label();

    virtual void paint(Graphics& g);
    void setText(const std::string & text);
    const std::string & text() const;

    void setMnemonic(char c);
    char mnemonic() const;

    virtual int preferredWidth() const;
    virtual int preferredHeight() const;

    void setVAlign(int align);
    int vAlign() const;
    void setHAlign(int align);
    int hAlign() const;

    void setTextOrientation(int orientation);

    void resize();

    void setWordWrap(bool on);
    bool wordWrap() const;

  private:

    int valign_, halign_;
    int orientation_;

    bool wbreak_;

    Bitmap rotateBmp;
    std::string text_;
    FontMetrics metrics;
    RectShape* rectShape;

    std::vector<std::string::size_type> breakPoints;

    void init();
    char mnemonic_;

    void computeBreakPoints();
    int findWidthMax(long width, const std::string & data, bool wbreak);
  };

}

#endif
