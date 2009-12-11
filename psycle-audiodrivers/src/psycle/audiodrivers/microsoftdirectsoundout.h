/******************************************************************************
*  copyright 2007 members of the psycle project http://psycle.sourceforge.net *
*                                                                             *
*  This program is free software; you can redistribute it and/or modify       *
*  it under the terms of the GNU General Public License as published by       *
*  the Free Software Foundation; either version 2 of the License, or          *
*  (at your option) any later version.                                        *
*                                                                             *
*  This program is distributed in the hope that it will be useful,            *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*  GNU General Public License for more details.                               *
*                                                                             *
*  You should have received a copy of the GNU General Public License          *
*  along with this program; if not, write to the                              *
*  Free Software Foundation, Inc.,                                            *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                  *
******************************************************************************/
#pragma once

#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
#include "audiodriver.h"
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <map>

namespace psy { namespace core {

/// output device interface implemented by direct sound.
class MsDirectSound : public AudioDriver {
	class PortEnums {
	public:
		PortEnums():guid() {};
		PortEnums(LPGUID _guid,std::string _pname):guid(_guid),portname(_pname){}
		std::string portname;
		LPGUID guid;
	};
	class PortCapt {
	public:
		PortCapt():pleft(0),pright(0),_pGuid(0),_pDs(0),_pBuffer(0),_lowMark(0),_machinepos(0) {};

		LPGUID _pGuid;
		LPDIRECTSOUNDCAPTURE8 _pDs;
		LPDIRECTSOUNDCAPTUREBUFFER8  _pBuffer;
		std::uint32_t _lowMark;
		float *pleft;
		float *pright;
		std::uint32_t _machinepos;
	};
	public:

		MsDirectSound();
		virtual ~MsDirectSound();

		AudioDriverInfo info( ) const;

		virtual void Initialize( AUDIODRIVERWORKFN pCallback, void * context );
		virtual bool Enable( bool );
		virtual bool Enabled() { return threadRunning_; }
		virtual void GetCapturePorts(std::vector<std::string>&ports);
		virtual bool AddCapturePort(std::uint32_t idx);
		virtual bool RemoveCapturePort(std::uint32_t idx);
		virtual bool CreateCapturePort(PortCapt &port);
		virtual void GetReadBuffers(std::uint32_t idx, float **pleft, float **pright,int numsamples);
		static BOOL CALLBACK DSEnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext);
		virtual int GetWritePos();
		virtual int GetPlayPos();
		virtual void Configure();
		virtual bool Initialized() { return _initialized; }
		virtual bool Configured() { return _configured; }
	protected:
		void ReadConfig();
		void WriteConfig();
		void Error(const WCHAR msg[]);
		static DWORD WINAPI PollerThreadStatic(void* pDirectSound);
		void PollerThread();
		void DoBlocks16();
		void DoBlocks24();
		bool WantsMoreBlocks();
		void DoBlocksRecording(PortCapt& port);
		bool WantsMoreBlocksRecording(PortCapt& port);
		bool Start();
		bool Stop();

	private:
		bool _initialized;
		bool _configured;
		static AudioDriverInfo _info;

		/// whether the directsound buffer is enabled and playing.
		bool dsBufferPlaying_;
		/// whether the thread is running
		bool threadRunning_;
		/// whether the thread is asked to terminate
		bool stopRequested_;
		/// a mutex to synchronise accesses to running_ and stop_requested_
		std::mutex mutex_;
		typedef std::scoped_lock<std::mutex> scoped_lock;
		/// a condition variable to wait until notified that the value of running_ has changed
		std::condition<scoped_lock> condition_;

		GUID device_guid;
		bool _exclusive;
		bool _dither;
		int _runningBufSize;
		DWORD _lowMark;
		DWORD _highMark;
		int _buffersToDo;

		std::vector<PortEnums> _capEnums;
		std::vector<PortCapt> _capPorts;
		std::vector<int> _portMapping;
		LPDIRECTSOUND8 _pDs;
		LPDIRECTSOUNDBUFFER8 _pBuffer;
		void* _callbackContext;
		AUDIODRIVERWORKFN _pCallback;
};

}}
#endif
