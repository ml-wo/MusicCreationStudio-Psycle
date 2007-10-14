// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\interface declarations needed by psycle::plugins.
#pragma once
#include <psycle/engine.hpp>
#include <cmath>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__PLUGIN
#include <universalis/compiler/dynamic_link/begin.hpp>
///\internal
/// extensible modular audio frawework.
namespace psycle {
///\internal
/// functionalities used by the plugin side only, not by the host.
/// Place your plugins in this namespace.
namespace plugins {

typedef engine::real real;

///\internal
/// plugins driving an underlying output device.
namespace outputs {}

#define PSYCLE__PLUGINS__CALLING_CONVENTION  UNIVERSALIS__COMPILER__CALLING_CONVENTION__C

#define PSYCLE__PLUGINS__NODE_INSTANTIATOR(typename) \
	extern "C" { \
		UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT \
		psycle::engine::node &  \
		PSYCLE__PLUGINS__CALLING_CONVENTION \
		PSYCLE__ENGINE__NODE_INSTANTIATOR__SYMBOL(new) ( \
			psycle::engine::plugin_library_reference & plugin_library_reference, \
			psycle::engine::graph & graph, \
			std::string const & name \
		) \
		throw(psycle::engine::exception) { \
			return psycle::engine::node::virtual_factory_access::create_on_heap<typename>(plugin_library_reference, graph, name); \
		} \
		\
		UNIVERSALIS__COMPILER__DYNAMIC_LINK__EXPORT \
		void \
		PSYCLE__PLUGINS__CALLING_CONVENTION \
		PSYCLE__ENGINE__NODE_INSTANTIATOR__SYMBOL(delete)(psycle::engine::node & node) { \
			node.free_heap(); \
		} \
	}
}}
#include <universalis/compiler/dynamic_link/end.hpp>

