// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__AUDIODRIVERS__MICROSOFT_DIRECT_SOUD_OUT__INCLUDED
#define PSYCLE__AUDIODRIVERS__MICROSOFT_DIRECT_SOUD_OUT__INCLUDED
#pragma once

#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE

#include "audiodriver.h"
#include <universalis/stdlib/condition_variable.hpp>

#include <universalis/os/include_windows_without_crap.hpp>

#if 1
	// MICROSOFT BUG : Weirdly, <dsound.h> needs WAVEFORMATEX and co, which it doesn't define, not it include any other header to bring its definition!
	// So, we include the mme system header to get WAVEFORMATEX and co.
	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma warning(push)
		#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
	#endif

		#include <MMSystem.h>
		#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
			#pragma comment(lib, "winmm")
		#endif

	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma warning(pop)
	#endif
#endif

#include <dsound.h>
#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	#pragma comment(lib, "dsound")
#endif

#include <map>

namespace psycle { namespace audiodrivers {

using namespace universalis::stdlib;

class DSoundUiInterface {
	public:
		DSoundUiInterface::DSoundUiInterface() {}
		virtual ~DSoundUiInterface() {}

		virtual int DoModal() = 0;

		virtual void SetValues(
			GUID device_guid, bool exclusive, bool dither,
			int sample_rate, int buffer_size, int buffer_count) = 0;

		virtual void GetValues(
			GUID & device_guid, bool & exclusive, bool & dither,
			int & sample_rate, int & buffer_size, int & buffer_count) = 0;
			
		virtual void WriteConfig(
			GUID device_guid, bool exclusive, bool dither,
			int sample_rate, int buffer_size, int buffer_count) = 0;

		virtual void ReadConfig(
			GUID & device_guid, bool & exclusive, bool & dither,
			int & sample_rate, int & buffer_size, int & buffer_count) = 0;

		virtual void Error(std::string const & msg) = 0;
};

/// output device interface implemented by direct sound.
class MsDirectSound : public AudioDriver {
	class PortEnums {
		public:
			PortEnums() : guid_() {};
			PortEnums(LPGUID const & guid, std::string const & port_name) : guid_(guid), port_name_(port_name) {}
			std::string port_name_;
			LPGUID guid_;
	};

	class PortCapt {
		public:
			PortCapt() : guid_(), capture_(), buffer_(), low_mark_(), high_mark_(), left_(), right_(), machine_pos_() {}
			LPGUID guid_;
			LPDIRECTSOUNDCAPTURE capture_;
			LPDIRECTSOUNDCAPTUREBUFFER buffer_;
			unsigned int low_mark_, high_mark_;
			float * left_, * right_;
			unsigned int machine_pos_;
	};

	public:
		MsDirectSound(DSoundUiInterface* = 0);
		~MsDirectSound() throw();

		/*override*/ AudioDriverInfo info() const;
		/*override*/ void Configure();

	protected:
		/*override*/ void do_open() throw(std::exception);
		/*override*/ void do_start() throw(std::exception);
		/*override*/ void do_stop() throw(std::exception);
		/*override*/ void do_close() throw(std::exception);

	public:
		void GetCapturePorts(std::vector<std::string> & ports);
		void AddCapturePort(uint32_t idx);
		void RemoveCapturePort(uint32_t idx);
		void CreateCapturePort(PortCapt & port);
		void GetReadBuffers(uint32_t idx, float ** left, float ** right, int num_samples);
		static BOOL CALLBACK DSEnumCallback(LPGUID guid, LPCSTR description, LPCSTR module, LPVOID context);
		int GetWritePos();
		int GetPlayPos();
		void AddConfigGui(DSoundUiInterface & ui) { ui_ = &ui; }

	protected:
		/*override*/ void ReadConfig();
		/*override*/ void WriteConfig();

	private:
		void DoBlocksRecording(PortCapt & port);
		bool WantsMoreBlocksRecording(PortCapt & port);

		static AudioDriverInfo info_;

		GUID device_guid_;
		bool exclusive_, dither_;
		unsigned int low_mark_, high_mark_;

		std::vector<PortEnums> cap_enums_;
		std::vector<PortCapt> cap_ports_;
		std::vector<int> port_mapping_;
		LPDIRECTSOUND direct_sound_;
		LPDIRECTSOUNDBUFFER buffer_;

		DSoundUiInterface * ui_;
		void error(std::string const & msg, universalis::compiler::location const & location);

	///\name thread
	///\{
		private:
			thread * thread_;
			/// the function executed by the thread
			void thread_function();
			/// whether the thread is asked to terminate
			bool stop_requested_;
			mutex mutex_;
			typedef class unique_lock<mutex> scoped_lock;
	///\}
};

}}
#endif
#endif
