#if 0 // file not used
	//////////////////////////////////////////////////////////////////////
	//
	//				Main.h ( Core Machine Interface Implementation )
	//
	//				druttis@darkface.pp.se
	//
	//////////////////////////////////////////////////////////////////////

	#pragma once

	#include <psycle/plugin_interface.hpp>
	#include "../CDsp.h"

	#ifdef __GENERATOR__
	#include "Track.h"
	#endif

	//////////////////////////////////////////////////////////////////////
	//				Info definitions
	//////////////////////////////////////////////////////////////////////

	#define MAC_NAME				"CoreMachine"
	#define MAC_VERSION				"1.0"
	#define MAC_AUTHOR				"Author"

	//////////////////////////////////////////////////////////////////////
	//				Info definitions
	//////////////////////////////////////////////////////////////////////

	#ifdef __GENERATOR__
	#define				MAX_TRACKS				32
	#define				MAX_VOICES				2
	#endif

	#define DEFAULT_INTERNAL_TICK_LENGTH 32
#endif