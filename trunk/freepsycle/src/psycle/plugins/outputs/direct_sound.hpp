///\file
///\brief \interface psycle::plugins::devices::outputs::direct_sound
#pragma once
#include <psycle/detail/project.hpp>
#include <universalis/operating_system/exception.hpp>
#include <universalis/compiler/numeric.hpp>

#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#error "this plugin is specific to microsoft's operating system"
#endif

#include <windows.h>

#if defined DIVERSALIS__COMPILER__FEATURE__AUTOLINK
	#pragma comment(lib, "user32") // for ::GetDesktopWindow(), see implementation file
#endif

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(push)
	#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif

#include <mmsystem.h>

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(pop)
#endif

#include <dsound.h> // dsound lib
#if defined DIVERSALIS__COMPILER__FEATURE__AUTOLINK
	#pragma comment(lib, "dsound")
#endif

#include "../resource.hpp"
namespace psycle
{
	namespace plugins
	{
		namespace outputs
		{
			/// outputs to a soundcard device via direct sound output implementation.
			class direct_sound : public resource
			{
				public:
					direct_sound(engine::plugin_library_reference &, engine::graph &, const std::string & name) throw(universalis::operating_system::exception);
					bool UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES opened()  const;
					bool UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES started() const;
				protected:
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_open()    throw(universalis::operating_system::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_start()   throw(universalis::operating_system::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_process() throw(universalis::operating_system::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_stop()    throw(universalis::operating_system::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_close()   throw(universalis::operating_system::exception);
				private:
					::IDirectSound        * direct_sound_;
					::IDirectSound inline & direct_sound_implementation() throw() { assert(direct_sound_); return *direct_sound_; }

					typedef universalis::compiler::numeric</*bits_per_channel_sample*/16>::signed_int output_sample_type;
					output_sample_type last_sample_;

					::IDirectSoundBuffer mutable * buffer_;
					::IDirectSoundBuffer inline  & buffer()       throw() { assert(buffer_); return *buffer_; }
					::IDirectSoundBuffer inline  & buffer() const throw() { assert(buffer_); return *buffer_; }

					bool write_primary_;
					unsigned int buffers_, buffer_size_, total_buffer_size_;
					/// position in byte offset
					unsigned int current_position_;
					unsigned int samples_per_buffer_;
			};
		}
	}
}
