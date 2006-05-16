/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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
#ifndef NGRIDLAYOUT_H
#define NGRIDLAYOUT_H

#include "nlayout.h"

/**
@author Stefan
*/
class NGridLayout : public NLayout
{
public:
    NGridLayout();
    NGridLayout(int cols, int rows);
    virtual NGridLayout* clone()  const;   // Uses the copy constructor

    ~NGridLayout();

    void setRows(int number);
    void setColumns(int number);

    void setHgap(int hgap);
    void setVgap(int vgap);

    virtual void align(NVisualComponent* parent);

    virtual int preferredWidth(const NVisualComponent* target) const;
    virtual int preferredHeight(const NVisualComponent* target) const;

private:

    int cols_;
    int rows_;
    int hgap_;
    int vgap_;

    int findMaxColWidth() const;
    int findMaxRowHeight() const;

};

#endif
