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
#include "psyfilter.h"

#include "psy3filter.h"

namespace psycle
{
	namespace host
	{

		std::vector<PsyFilter*> PsyFilter::filters;
		int PsyFilter::c = 0;


		PsyFilter::PsyFilter()
		{
			if (c==0) {
				c++;
				filters.push_back(new Psy3Filter());
			}
		}


		PsyFilter::~PsyFilter()
		{
		}

		void PsyFilter::load( const std::string & fileName, Song & song )
		{
		}

		void PsyFilter::save( const std::string & fileName, const Song & song ) const
		{
		}

		bool PsyFilter::testFormat( const std::string & fileName )
		{
			return false;
		}

		int PsyFilter::version( ) const
		{
			return 0;
		}

		void PsyFilter::loadSong( const std::string & fileName, Song & song )
		{
			std::vector<PsyFilter*>::iterator it = filters.begin();
			for (  ; it < filters.end(); it++) {
				PsyFilter* filter = *it;
				if ( filter->testFormat(fileName) ) {
					filter->load(fileName,song);
					break;
				}
			}
		}

		void PsyFilter::saveSong( const std::string & fileName, Song & song, int version ) const
		{
			std::vector<PsyFilter*>::iterator it = filters.begin();
			for (  ; it < filters.end(); it++) {
				PsyFilter* filter = *it;
				if ( filter->version() == version ) {
					filter->save(fileName,song);
					break;
				}
			}
		}


	} // end of host namespace
} // end of psycle namespace




