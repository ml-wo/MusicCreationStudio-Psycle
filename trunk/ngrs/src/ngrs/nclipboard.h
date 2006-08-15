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
#ifndef NCLIPBOARD_H
#define NCLIPBOARD_H

#include <string>
#include <vector>

/**
@author Stefan Nattkemper
*/

const unsigned char CF_TEXT = 1;


class NClipBoard{

class ClipBoardData : public std::vector<unsigned char> {

public :

  ClipBoardData();
  ~ClipBoardData();

  void setFormat( unsigned char format);
  unsigned char format( ) const;

};


public:
    NClipBoard();

    ~NClipBoard();

    void setAsText( const std::string & text );
    std::string asText() const;

private:

    ClipBoardData data_;

};

#endif
