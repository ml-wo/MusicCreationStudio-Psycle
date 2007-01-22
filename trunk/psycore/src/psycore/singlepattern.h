/***************************************************************************
 *   Copyright (C) 2006 by  Stefan Nattkemper   *
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
#ifndef SINGLEPATTERN_H
#define SINGLEPATTERN_H

#include "patternline.h"
#include "timesignature.h"
#include "../../ngrs/src/ngrs/sigslot.h" // todo put sigslot in a common dir
#include <map>
#include <vector>
#include <string>

/**
@author  Stefan Nattkemper
*/

namespace psy
{
	namespace core
	{


    class TweakTrackInfo {
		public:

			enum TweakType { twk, tws, mdi, aut };

			TweakTrackInfo();
			TweakTrackInfo( int mac, int param, TweakType type );

			~TweakTrackInfo();

			int machineIdx()    const;
			int parameterIdx()  const;
			TweakType type()          const;

      bool operator<(const TweakTrackInfo & key) const;

		private:

			int macIdx_;
			int paramIdx_;
			TweakType type_;

		};


		class SinglePattern  {
		public:
			SinglePattern();

			virtual ~SinglePattern();

			void setID(int id);
			int id() const;

			void setBeatZoom(int zoom);
			int beatZoom() const;


			void setEvent( int line, int track, const PatternEvent & event );
			PatternEvent event( int line, int track );

			void setTweakEvent( int line, int track, const PatternEvent & event );
			PatternEvent tweakEvent( int line, int track );


			void addBar( const TimeSignature & signature );
			void removeBar( float pos);

			float beats() const;

			bool barStart(double pos, TimeSignature & signature) const;
			void clearBars();

			const TimeSignature & playPosTimeSignature(double pos) const;

			void setName(const std::string & name);
			const std::string & name() const;

			float beatsPerLine() const;

			void clearTrack( int linenr , int tracknr );
			void clearTweakTrack( int linenr , int tracknr );
            void clear();
			bool lineIsEmpty( int linenr ) const;

            std::map<double, PatternLine>::iterator find_nearest( int linenr );
			std::map<double, PatternLine>::const_iterator find_nearest( int linenr ) const;

            std::map<double, PatternLine>::const_iterator begin() const;
            std::map<double, PatternLine>::const_iterator end() const;

			std::map<double, PatternLine>::iterator find_lower_nearest( int linenr );
			std::map<double, PatternLine>::const_iterator find_lower_nearest( int linenr ) const;

            std::map<double, PatternLine>::const_iterator lower_bound(double key) const;
            void insert( double pos, const PatternLine& pattern );

			void clearEmptyLines();

			void scaleBlock(int left, int right, double top, double bottom, float factor);
			void transposeBlock(int left, int right, double top, double bottom, int trp);
			void deleteBlock(int left, int right, double top, double bottom);


			std::vector<TimeSignature> &  timeSignatures();
			const std::vector<TimeSignature> &  timeSignatures() const;

			std::string toXml() const;

			SinglePattern block( int left, int right, int top, int bottom );
			void copyBlock(int left, int top, const SinglePattern & pattern, int tracks, float maxBeats);
			void mixBlock(int left, int top, const SinglePattern & pattern, int tracks, float maxBeats);

			void deleteBlock( int left, int right, int top, int bottom );

			TweakTrackInfo tweakInfo( int track ) const;
			int tweakTrack( const TweakTrackInfo & info);

		private:

			int beatZoom_;
			std::string name_;

			std::vector<TimeSignature> timeSignatures_;
			TimeSignature zeroTime;

			int id_;
			static int idCounter;

            std::map<double, PatternLine> patternMap;
			std::map<TweakTrackInfo, int> tweakInfoMap;

		};

	}
}

#endif
