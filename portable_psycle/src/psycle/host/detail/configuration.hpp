
// Note: this public configuration was not generated by an autoconf configuration script.

#pragma once

/// [bohan] implementation of psycle::host::Song's lock using boost 1.3's read_write_mutex.
/// [bohan]
/// [bohan] I used a temporary #define (to be removed) to enable this new implementation of the gui<->audio thread synchronization.
/// [bohan] Once the new implementation is known to work well,
/// [bohan] we can remove this #define, which will trigger some #error in the places of the code that are concerned.
/// [bohan] Where the #error occurred, we can removed the old implementation.
/// [bohan]
/// [bohan] to enable this new implementation,
/// [bohan] #define PSYCLE__CONFIGURATION__READ_WRITE_MUTEX 1
/// [bohan]
/// [bohan] to disable this new implementation, do not undefine the preprocessor symbol (which will triggers the #error's), but rather
/// [bohan] #define PSYCLE__CONFIGURATION__READ_WRITE_MUTEX 0
#define PSYCLE__CONFIGURATION__READ_WRITE_MUTEX 0

/// JAZ: Define to 1 to enable the volume column for XMSampler. It will also make the machine column in the pattern to show
///      the values of the volume column instead.
#define PSYCLE__CONFIGURATION__VOLUME_COLUMN 0

/// Test for RMS Vu's
//#define PSYCLE__CONFIGURATION__RMS_VUS 1

/// unmasks fpu exceptions
/// [JAZ] : I have experienced crashes with this option enabled, which didn't seem to come from the code itself.
/// [JAZ]   It could be that the exception code handling has a bug somewhere.
#define PSYCLE__CONFIGURATION__FPU_EXCEPTIONS 0



/// string describing the configuration options.
#define PSYCLE__CONFIGURATION__DESCRIPTION(EOL) \
	"read_write_mutex = "          UNIVERSALIS__COMPILER__STRINGIZED(PSYCLE__CONFIGURATION__READ_WRITE_MUTEX) EOL \
	"fpu exceptions = "            UNIVERSALIS__COMPILER__STRINGIZED(PSYCLE__CONFIGURATION__FPU_EXCEPTIONS)   EOL \
	"volume column = "             UNIVERSALIS__COMPILER__STRINGIZED(PSYCLE__CONFIGURATION__VOLUME_COLUMN)    EOL \
	"rms vus = "                   UNIVERSALIS__COMPILER__STRINGIZED(PSYCLE__CONFIGURATION__RMS_VUS)          EOL \
	"debugging = "                                                   PSYCLE__CONFIGURATION__DEBUG

/// value to show in the string describing the configuration options (PSYCLE__CONFIGURATION__DESCRIPTION).
///\see PSYCLE__CONFIGURATION__DESCRIPTION
#if defined NDEBUG
	#define PSYCLE__CONFIGURATION__DEBUG "off"
#else
	#define PSYCLE__CONFIGURATION__DEBUG "on"
#endif

/// end-of-line character sequence on the platform.
///\todo get this definition from universalis
#define EOL "\r\n"

#include <universalis/compiler/stringized.hpp>
