// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2009-2009 members of the psycle project http://psycle.sourceforge.net

///\interface psycle::core::cpu_time_clock

#ifndef PSYCLE__CORE__CPU_TIME_CLOCK__INCLUDED
#define PSYCLE__CORE__CPU_TIME_CLOCK__INCLUDED
#pragma once

#include <psycle/core/detail/project.private.hpp>
#include <universalis/os/clocks.hpp>
#include <universalis/stdlib/date_time.hpp>

namespace psycle { namespace core {

using namespace universalis::stdlib;

nanoseconds inline cpu_time_clock() {
	return universalis::os::clocks::monotonic::current();
}

}}
#endif
