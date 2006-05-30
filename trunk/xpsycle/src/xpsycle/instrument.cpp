/***************************************************************************
  *   Copyright (C) 2006 by Stefan   *
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
#include "constants.h"
#include "instrument.h"
#include "filter.h"
#include "datacompression.h"
#include "fileio.h"
#include <cstdio>


template<typename object_array> inline object_array * zapArray(object_array *& pointer, object_array * const new_value = 0)
{
    if(pointer) delete [] pointer;
    return pointer = new_value;
}


Instrument::Instrument()
{
  // clear everythingout
  waveDataL = 0;
  waveDataR = 0;
  waveLength = 0;
  Delete();
}

Instrument::~Instrument()
{
  zapArray(waveDataL);
  zapArray(waveDataR);
  waveLength = 0;
}

void Instrument::Delete()
{
  // Reset envelope
  ENV_AT = 1; // 16
  ENV_DT = 1; // 16386
  ENV_SL = 100; // 64
  ENV_RT = 16; // OVERLAPTIME

  ENV_F_AT = 16;
  ENV_F_DT = 16384;
  ENV_F_SL = 64;
  ENV_F_RT = 16384;

  ENV_F_CO = 64;
  ENV_F_RQ = 64;
  ENV_F_EA = 128;
  ENV_F_TP = F_NONE;

  _loop = false;
  _lines = 16;

  _NNA = 0; // NNA set to Note Cut [Fast Release]

  _pan = 128;
  _RPAN = false;
  _RCUT = false;
  _RRES = false;

  DeleteLayer();

  sprintf(_sName,"empty");
}

void Instrument::DeleteLayer(void)
{
  sprintf(waveName,"empty");

  zapArray(waveDataL);
  zapArray(waveDataR);
  waveLength = 0;
  waveStereo=false;
  waveLoopStart=0;
  waveLoopEnd=0;
  waveLoopType=0;
  waveVolume=100;
  waveFinetune=0;
  waveTune=0;
}

bool Instrument::Empty()
{
  if (waveLength > 0) return false;
  return true;
}




void Instrument::LoadFileChunk(RiffFile* pFile,int version,bool fullopen)
		{
			Delete();
			// assume version 0 for now
			pFile->Read(_loop);
			pFile->Read(_lines);
			pFile->Read(_NNA);

			pFile->Read(ENV_AT);
			pFile->Read(ENV_DT);
			pFile->Read(ENV_SL);
			pFile->Read(ENV_RT);
			
			pFile->Read(ENV_F_AT);
			pFile->Read(ENV_F_DT);
			pFile->Read(ENV_F_SL);
			pFile->Read(ENV_F_RT);

			pFile->Read(ENV_F_CO);
			pFile->Read(ENV_F_RQ);
			pFile->Read(ENV_F_EA);
			pFile->Read(ENV_F_TP);

			pFile->Read(_pan);
			pFile->Read(_RPAN);
			pFile->Read(_RCUT);
			pFile->Read(_RRES);

			pFile->ReadString(_sName, sizeof _sName);

			// now we have to read waves

			int numwaves;
			pFile->Read(numwaves);
			for (int i = 0; i < numwaves; i++)
			{
				char Header[5];

				pFile->ReadChunk(&Header,4);
				Header[4] = 0;
				std::uint32_t version;
				std::uint32_t size;

				if (strcmp(Header,"WAVE")==0)
				{

					pFile->Read(version);
					pFile->Read(size);
					//fileformat supports several waves, but sampler only supports one.
					if (version > CURRENT_FILE_VERSION_WAVE || i > 0)
					{
						// there is an error, this file is newer than this build of psycle
						//MessageBox(NULL,"Wave Segment of File is from a newer version of psycle!",NULL,NULL);
						pFile->Skip(size);
					}
					else
					{
						std::uint32_t index;
						pFile->Read(index);

						pFile->Read(waveLength);
						pFile->Read(waveVolume);
						pFile->Read(waveLoopStart);
						pFile->Read(waveLoopEnd);
						
						pFile->Read(waveTune);
						pFile->Read(waveFinetune);
						pFile->Read(waveLoopType);
						pFile->Read(waveStereo);
						
						pFile->ReadString(waveName, sizeof waveName);
						
						pFile->Read(size);
						byte* pData;
						
						if ( !fullopen )
						{
							pFile->Skip(size);
							waveDataL=new std::int16_t[2];
						}
						else
						{
							pData = new std::uint8_t[size+4];// +4 to avoid any attempt at buffer overflow by the code <-- ?
							pFile->ReadChunk(pData,size);
							///\todo SoundDesquash should be object-oriented and provide access to this via its interface
							if(waveLength != *reinterpret_cast<std::uint32_t const *>(pData + 1))
							{
									std::ostringstream s;
									s << "instrument: " << index << ", name: " << waveName << std::endl;
									s << "sample data: unpacked length mismatch: " << waveLength << " versus " << *reinterpret_cast<std::uint32_t const *>(pData + 1) << std::endl;
									s << "You should reload this wave sample and all the samples after this one!";
//									loggers::warning(s.str());
//									MessageBox(0, s.str().c_str(), "Loading wave sample data", MB_ICONWARNING | MB_OK);
							}
							DataCompression::SoundDesquash(pData,&waveDataL);
							delete[] pData;
						}

						if (waveStereo)
						{
							pFile->Read(size);
							if ( !fullopen )
							{
								pFile->Skip(size);
								delete[] waveDataR;
								waveDataR = new std::int16_t[2];
							}
							else
							{
								pData = new std::uint8_t[size+4]; // +4 to avoid any attempt at buffer overflow by the code <-- ?
								pFile->ReadChunk(pData,size);
								///\todo SoundDesquash should be object-oriented and provide access to this via its interface
								if(waveLength != *reinterpret_cast<std::uint32_t const *>(pData + 1))
								{
										std::ostringstream s;
										s << "instrument: " << index << ", name: " << waveName << std::endl;
										s << "stereo wave sample data: unpacked length mismatch: " << waveLength << " versus " << *reinterpret_cast<std::uint32_t const *>(pData + 1) << std::endl;
										s << "You should reload this wave sample and all the samples after this one!";
//										loggers::warning(s.str());
//										MessageBox(0, s.str().c_str(), "Loading stereo wave sample data", MB_ICONWARNING | MB_OK);
								}
								DataCompression::SoundDesquash(pData,&waveDataR);
								delete[] pData;
							}
						}
					}
				}
				else
				{
					pFile->Read(version);
					pFile->Read(size);
					// there is an error, this file is newer than this build of psycle
					//MessageBox(NULL,"Wave Segment of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
			}
		}

		void Instrument::SaveFileChunk(RiffFile* pFile)
		{
			pFile->Write(_loop);
			pFile->Write(_lines);
			pFile->Write(_NNA);

			pFile->Write(ENV_AT);
			pFile->Write(ENV_DT);
			pFile->Write(ENV_SL);
			pFile->Write(ENV_RT);
			
			pFile->Write(ENV_F_AT);
			pFile->Write(ENV_F_DT);
			pFile->Write(ENV_F_SL);
			pFile->Write(ENV_F_RT);

			pFile->Write(ENV_F_CO);
			pFile->Write(ENV_F_RQ);
			pFile->Write(ENV_F_EA);
			pFile->Write(ENV_F_TP);

			pFile->Write(_pan);
			pFile->Write(_RPAN);
			pFile->Write(_RCUT);
			pFile->Write(_RRES);

			pFile->WriteChunk(_sName, std::strlen(_sName) + 1);

			// now we have to write out the waves, but only if valid

			int numwaves = (waveLength > 0) ? 1 : 0; // The sampler has never supported more than one sample per instrument, even when the GUI did.

			pFile->Write(numwaves);
			if (waveLength > 0)
			{
				std::uint8_t * pData1(0);
				std::uint8_t * pData2(0);
				std::uint32_t size1=0,size2=0;
				size1 = DataCompression::SoundSquash(waveDataL,&pData1,waveLength);
				if (waveStereo)
				{
					size2 = DataCompression::SoundSquash(waveDataR,&pData2,waveLength);
				}

				std::uint32_t index = 0;
				pFile->Write("WAVE");
				std::uint32_t version = CURRENT_FILE_VERSION_PATD;
				std::uint32_t size =
					sizeof index +
					sizeof waveLength +
					sizeof waveVolume +
					sizeof waveLoopStart +
					sizeof waveLoopEnd +
					sizeof waveTune +
					sizeof waveFinetune +
					sizeof waveStereo +
					std::strlen(waveName) + 1 +
					size1 +
					size2;

				pFile->Write(version);
				pFile->Write(size);
				pFile->Write(index);

				pFile->Write(waveLength);
				pFile->Write(waveVolume);
				pFile->Write(waveLoopStart);
				pFile->Write(waveLoopEnd);

				pFile->Write(waveTune);
				pFile->Write(waveFinetune);
				pFile->Write(waveLoopType);
				pFile->Write(waveStereo);

				pFile->WriteChunk(waveName, std::strlen(waveName) + 1);

				pFile->Write(size1);
				pFile->WriteChunk(pData1,size1);
				delete[] pData1;
				if (waveStereo)
				{
					pFile->Write(size2);
					pFile->WriteChunk(pData2,size2);
				}
				delete[] pData2;
			}
		}