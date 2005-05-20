// This file is an exact copy of the following file from freepsycle:
// http://bohan.dyndns.org/cgi-bin/archzoom.cgi/psycle@sourceforge.net/psycle--mainline--0--patch-286/src/project.private.hpp
///\file
///\brief project-wide compiler, operating system, and processor specific tweaks.
///\meta generic
#pragma once
#include "project.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// autoconf

#if !defined HAVE_CONFIG_H && defined COMPILER__FEATURE__NOT_CONCRETE
	/// HAVE_CONFIG_H is automatically defined by autoconf/automake, but then it's not defined when invoking other pseudo compilers.
	#define HAVE_CONFIG_H
#endif
#if defined HAVE_CONFIG_H
	// includes the configuration header generated by the configure script.
	#include "configuration.private.hpp"
#else
	#error "HAVE_CONFIG_H is not defined."
#endif



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// pre-compiled headers
// If we're not compiling the sources of the package themselves, but are just being #included from other sources,
// then it's not up to us to decide wether use pre-compiled headers or not, and neither even what source (.cpp) and/or header (.hpp) files have been pre-compiled.
// That's why pre-compiled headers are included by this private file, which is only included (and always, as the first inclusion) by sources (.cpp) files, never by public header (.hpp) files.



#if defined CONFIGURATION__OPTION__ENABLE__PRE_COMPILATION
	// On compilers which don't support pre-compilation, this inclusion would just slow them down considerably.
	#if defined COMPILER__FEATURE__PRE_COMPILATION
		#include "pre_compiled_headers.private.hpp"
	#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Psycle Headers to Precompile.
#include "psycle/host/global.hpp"
