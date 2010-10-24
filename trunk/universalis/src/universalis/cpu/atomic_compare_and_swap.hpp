// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2006-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface universalis::cpu::atomic::compare_and_swap

#ifndef UNIVERSALIS__CPU__ATOMIC_COMPARE_AND_SWAP__INCLUDED
#define UNIVERSALIS__CPU__ATOMIC_COMPARE_AND_SWAP__INCLUDED
#pragma once

#include <diversalis/compiler.hpp>
#if defined DIVERSALIS__COMPILER__GNU && DIVERSALIS__COMPILER__VERSION >= 40100 // 4.1.0
	// gcc's __sync_* built-in functions documented at http://gcc.gnu.org/onlinedocs/gcc-4.1.0/gcc/Atomic-Builtins.html
#else
	#include <glib/gatomic.h>
#endif

namespace universalis { namespace cpu {

// see c++ standard http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n2047.html
// see apache portable runtime http://svn.apache.org/viewvc/apr/apr/trunk/atomic
// see apache harmony atomic http://svn.apache.org/viewvc/harmony/enhanced/drlvm/trunk/vm/port/include/port_atomic.h
// see apache harmony barrier http://svn.apache.org/viewvc/harmony/enhanced/drlvm/trunk/vm/port/include/port_barriers.h
// see glibc */bits/atomic.h at http://sources.redhat.com/cgi-bin/cvsweb.cgi/libc/sysdeps/?cvsroot=glibc
// see glib atomic http://svn.gnome.org/viewvc/glib/trunk/glib/gatomic.c?view=markup
// see glib/gstreamer ring buffer http://webcvs.freedesktop.org/gstreamer/gst-plugins-base/gst-libs/gst/audio/gstringbuffer.c
// see portaudio memory barrier http://portaudio.com/trac/browser/portaudio/trunk/src/common/pa_memorybarrier.h
// see portaudio ring buffer    http://portaudio.com/trac/browser/portaudio/trunk/src/common/pa_ringbuffer.c
// see liblf data structures http://www.audiomulch.com/~rossb/code/lockfree/liblfds
// see liblf data structures http://tech.groups.yahoo.com/group/liblf-dev
// see microsoft/intel _Interlocked* intrinsics

/// atomic compare and swap with full memory barrier.
/// does not fail spuriously.
template<typename Value>
bool inline atomic_compare_and_swap(Value & address, Value old_value, Value new_value) {
	#if defined DIVERSALIS__COMPILER__GNU && DIVERSALIS__COMPILER__VERSION >= 40100 // 4.1.0
		return __sync_bool_compare_and_swap(&address, old_value, new_value);
	#else
		// no return statement to error on purpose -- see template specialisations below
	#endif
}

#if !defined DIVERSALIS__COMPILER__GNU || DIVERSALIS__COMPILER__VERSION < 40100 // 4.1.0
	template<>
	bool inline atomic_compare_and_swap< ::gpointer>(::gpointer & address, ::gpointer old_value, ::gpointer new_value) {
		return ::g_atomic_pointer_compare_and_exchange(&address, old_value, new_value);
	}

	template<>
	bool inline atomic_compare_and_swap< ::gint>(::gint & address, ::gint old_value, ::gint new_value) {
		return ::g_atomic_int_compare_and_exchange(&address, old_value, new_value);
	}
#endif

}}

/******************************************************************************************/
#if defined BOOST_AUTO_TEST_CASE
	#include <universalis/stdlib/thread.hpp>
	#include <universalis/os/clocks.hpp>
	#include <vector>
	#include <boost/bind.hpp>
	#include <sstream>

	namespace universalis { namespace cpu {
		using namespace universalis::stdlib;
		class atomic_compare_and_swap_test_class {
			public:
				void test() {
					i = 0;
					thread t(boost::bind(&atomic_compare_and_swap_test_class::thread_function, this));
					while(!atomic_compare_and_swap(i, 1, 2));
					t.join();
				}

			private:
				int i;

				void thread_function() {
					i = 1;
					while(!atomic_compare_and_swap(i, 2, 2));
			}
		};

		BOOST_AUTO_TEST_CASE(atomic_compare_and_swap_test) {
			atomic_compare_and_swap_test_class test;
			test.test();
		}

		class atomic_compare_and_swap_speed_test_class {
			private:
				nanoseconds static cpu_time_clock() {
					#if 0
						return hiresolution_clock<utc_time>::universal_time().nanoseconds_since_epoch();
					#elif 0
						return universalis::os::clocks::thread_cpu_time::current();
					#elif 0
						return universalis::os::clocks::process_cpu_time::current();
					#else
						return universalis::os::clocks::monotonic::current();
					#endif
				}

				class tls {
					public:
						tls(int i) : t(), count(), i(i) {}
						nanoseconds t;
						unsigned int count;
						int i;
				};

				void thread_function(tls * const tls_pointer) {
					tls & tls(*tls_pointer);
					--shared_start;
					while(!atomic_compare_and_swap(shared_start, 0, 0));
					while(tls.count < end) {
						nanoseconds const t0(cpu_time_clock());
						for(unsigned int i(0); i < inner_loop; ++i) {
							if(atomic_compare_and_swap(shared, -1, -1)) return;
							shared = tls.i;
							while(atomic_compare_and_swap(shared, tls.i, tls.i));
						}
						nanoseconds const t1(cpu_time_clock());
						if(++tls.count > start) tls.t += t1 - t0;
					}
					shared = -1;
				}

				unsigned int inner_loop, start, end;

				int shared, shared_start;

			public:
				void test(unsigned int threads) {
					inner_loop = 50;
					start = 10;
					end = start + 10;
					shared_start = threads;
					shared = 0;

					std::vector<tls*> tls_(threads);
					for(unsigned int i(0); i < threads; ++i) tls_[i] = new tls(i);

					std::vector<thread*> threads_(threads);
					for(unsigned int i(0); i < threads; ++i)
						threads_[i] = new thread(boost::bind(&atomic_compare_and_swap_speed_test_class::thread_function, this, tls_[i]));

					for(unsigned int i(0); i < threads; ++i) threads_[i]->join();

					for(unsigned int i(0); i < threads; ++i) {
						delete threads_[i];
						{
							std::ostringstream s;
							s << threads << " threads: " << i << ": " << tls_[i]->t.get_count() * 1e-9 / (tls_[i]->count - start) / inner_loop << 's';
							BOOST_MESSAGE(s.str());
						}
						delete tls_[i];
					}
				}
		};

		BOOST_AUTO_TEST_CASE(atomic_compare_and_swap_speed_test) {
			atomic_compare_and_swap_speed_test_class test;
			test.test(2);
			test.test(4);
			test.test(8);
		}
	}}
#endif

#endif