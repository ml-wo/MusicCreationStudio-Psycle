// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
///\brief
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <universalis/detail/project.private.hpp>

// weird, must be included last or mingw 3.4.1 segfaults
//#include "code_description.hpp"

#include <universalis/operating_system/exceptions/code_description.hpp>

// weird, must be included last or mingw 3.4.1 segfaults
#include "code_description.hpp"

namespace universalis
{
	namespace processor
	{
		namespace exceptions
		{
			std::string code_description(int const & code) throw()
			{
				return operating_system::exceptions::detail::code_description
					(
						code
						#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
							, /* from_processor */ true
						#endif
					);
			}
		}
	}
}
