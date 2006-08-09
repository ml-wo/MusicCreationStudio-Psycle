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
#ifndef WAVEFILEOUT_H
#define WAVEFILEOUT_H

#include <audiodriver.h>
#include "riff.h"

namespace psycle
{
	namespace host
	{

		/**
		@author Stefan Nattkemper
		*/

		class WaveFileOut : public AudioDriver
		{
			public:
				WaveFileOut();

				~WaveFileOut();

				virtual void Initialize(AUDIODRIVERWORKFN pCallback, void * context);

				// starts stops file writing
				virtual bool Enable(bool e);

				void setFileName( const std::string & fileName);
				const std::string & fileName() const;

			private:

				std::string fileName_;

				void* _callbackContext; // Player callback
				AUDIODRIVERWORKFN _pCallback;
				bool _initialized;

				int iret1;
				int threadOpen;

				pthread_t threadid;

				int kill_thread;

				static int audioOutThread(void * ptr);

				void writeBuffer();

				WaveFile _outputWaveFile;

				float _pSamplesL[441];
			/// right data
				float _pSamplesR[441];

		};

	}
}

#endif
