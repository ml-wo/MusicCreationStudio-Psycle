// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::engine::ports::input
#pragma once
#include "../port.hpp"
#if defined PSYCLE__EXPERIMENTAL
	#include <psycle/generic/generic.hpp>
#endif
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__PORTS__INPUT
#include <universalis/compiler/dynamic_link/begin.hpp>

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#if defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

namespace psycle
{
	namespace engine
	{
		namespace ports
		{
			typedef generic::ports::input<typenames::typenames> input_base;
			/// handles an input stream of signal coming to a node.
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK input : public input_base
			{
				friend class node;
				
				public:
					virtual ~input() throw();
				protected:
					input(parent_type &, name_type const &, int const & channels = 0); friend class generic_access;
	
				public:
					void    connect(typenames::ports::output &) throw(exception);
					void disconnect(typenames::ports::output &);
	
				public:
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES dump(std::ostream &, int const & tabulations = 0) const = 0;
			};
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#else // !defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

namespace psycle
{
	namespace engine
	{
		namespace ports
		{
			/// handles an input stream of signal coming to a node.
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK input : public port
			{
				friend class node;
				
				public:
					virtual ~input() throw();
				protected:
					input(engine::node &, name_type const &, int const & channels = 0);
	
				public:
					void         connect(output & output_port) throw(exception);
				protected:
					void virtual connect_internal_side(output &) = 0;
	
				public:
					void virtual disconnect_all() = 0;
					void         disconnect(output & output_port);
				protected:
					void virtual disconnect_internal_side(output &) = 0;
	
				public:
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES dump(std::ostream &, int const & tabulations = 0) const = 0;
			};
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#endif // !defined PSYCLE__EXPERIMENTAL
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

#include <universalis/compiler/dynamic_link/end.hpp>
