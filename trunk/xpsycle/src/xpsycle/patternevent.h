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
#ifndef PATTERNEVENT_H
#define PATTERNEVENT_H

#include "pattern.h"
#include <cstdint>
#include <string>

/**
@author Stefan Nattkemper
*/

namespace psycle
{
	namespace host
	{

		class PatternEvent{
		public:
			PatternEvent();
			PatternEvent(const PatternEntry & entry); // for compatible file load

			~PatternEvent();

			void setNote(std::uint8_t value);
			std::uint8_t note() const;

			void setInstrument(std::uint8_t instrument);
			std::uint8_t instrument() const;

			void setMachine(std::uint8_t machine);
			std::uint8_t machine() const;

			void setCommand(std::uint8_t command);
			std::uint8_t command() const;

			void setParameter(std::uint8_t parameter);
			std::uint8_t parameter() const;

			void setVolume(std::uint8_t parameter);
			std::uint8_t volume() const;

			PatternEntry* entry();
			PatternEntry* entry() const;

			bool isEmpty() const;

			std::string toXml(int track) const;

		private:

			std::uint8_t _note;
			std::uint8_t _inst;
			std::uint8_t _mach;
			std::uint8_t _cmd;
			std::uint8_t _parameter;
			std::uint8_t _volume;

		};
	}
}

#endif
