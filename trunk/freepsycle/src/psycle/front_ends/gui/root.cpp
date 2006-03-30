// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright ? 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\implementation psycle::front_ends::gui::root
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include "root.hpp"
#include <psycle/paths.hpp>
#include <gdkmm/screen.h>
namespace psycle
{
	namespace front_ends
	{
		namespace gui
		{
			root::root(engine::hello & hello)
			:
				hello_(hello),
				button_("get a hello message from psycle's engine"),
				label_(/*"message from psycle's engine"*/),
				graph_("graph", resolver()),
				hello_frame_("hello"),
				graph_frame_(graph_instance().graph_instance().name())
			{
				set_size_request(1000, 700);
				set_title(paths::package::name() + " " + paths::package::version::string());
				set_border_width(4);
				button().signal_clicked().connect(sigc::mem_fun(*this, &root::on_button_clicked));
//				h_box_.pack_start(button(), Gtk::PACK_SHRINK);
//				h_box_.pack_start(label(), Gtk::PACK_EXPAND_PADDING, 4);
//				hello_frame_.add(h_box_);
//				v_box_.pack_start(hello_frame_, Gtk::PACK_SHRINK);
				graph_frame_.add(graph_instance());
				v_box_.pack_start(graph_frame_);
				add(v_box_);
				show_all_children();
				//set_resize_mode(Gtk::RESIZE_IMMEDIATE);
			}
		
			root::~root() throw()
			{
			}
		
			void root::on_button_clicked()
			{
				label().set_text(hello().say_hello());
			}
		}
	}
}
