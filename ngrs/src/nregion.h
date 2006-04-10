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
#ifndef NREGION_H
#define NREGION_H

#include "nsystem.h"
#include "nrect.h"

/**
@author Stefan Nattkemper
*/
class NRegion{
public:
    NRegion();
    NRegion(const NRect & rect);

    ~NRegion();

    NRegion(const NRegion & src);
    const NRegion & operator= (const NRegion & rhs);

    void shrink(int dx, int dy);
    void move(int dx, int dy);
    void setRect(const NRect & rect);
    void setPolygon(XPoint*  pts , int size);
    bool isEmpty() const;
    NRect rectClipBox();

    Region xRegion() const;  // warning this pointer can change

    NRegion operator&(const NRegion & rhs);
    NRegion operator|(const NRegion & rhs);
    NRegion operator-(const NRegion & rhs);
    NRegion operator^(const NRegion & rhs);

private:

    Region region_;

};

#endif
