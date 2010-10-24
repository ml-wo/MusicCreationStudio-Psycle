// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::helpers::ring_buffer
#ifndef PSYCLE__HELPERS__RING_BUFFER__INCLUDED
#define PSYCLE__HELPERS__RING_BUFFER__INCLUDED
#pragma once
#include <universalis.hpp>
#include <universalis/cpu/memory_barriers.hpp>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <cstdatomic>
	
namespace psycle { namespace helpers {

// see portaudio memory barrier http://portaudio.com/trac/browser/portaudio/trunk/src/common/pa_memorybarrier.h
// see portaudio ring buffer    http://portaudio.com/trac/browser/portaudio/trunk/src/common/pa_ringbuffer.c

/// lock-free ring buffer concept.
///
/// This ring buffer is useful for push-based multi-threaded processing.
/// It helps optimising thread-synchronisation by using lock-free atomic primitives
/// (normally implemented with specific CPU instructions),
/// instead of relying on OS-based synchronisation facilities,
/// which incur the overhead of context switching and re-scheduling delay.
///
/// This class does not store the actual buffer data, but only a read position and a write position.
/// This separation of concerns makes it possible to use this class with any kind of data access interface for the buffer.
#if 0
	class ring {
		public:
			ring(std::size_t size) { assert("power of 2" && !(size & (size - 1))); }
			std::size_t size() const;
			void commit_read(std::size_t count);
			void commit_write(std::size_t count);
			void avail_for_read(std::size_t max, std::size_t & begin, std::size_t & size1, std::size_t & size2) const;
			void avail_for_write(std::size_t max, std::size_t & begin, std::size_t & size1, std::size_t & size2) const;
	};
#else
	typedef class ring_with_atomic_stdlib ring;
#endif

/// ring buffer using c++0x atomic types
class ring_with_atomic_stdlib {
	std::size_t const size_, size_mask_, size_mask2_;
	std::atomic<std::size_t> read_, write_;
	
	public:
		ring_with_atomic_stdlib(std::size_t size) : size_(size), size_mask_(size - 1), size_mask2_(size * 2 - 1), read_(0), write_(0) {
			assert("power of 2" && !(size & size_mask_));
		}
		
		std::size_t size() const { return size_; }
		
		void commit_read(std::size_t count) {
			auto const read = read_.load(std::memory_order_relaxed);
			read_.store((read + count) & size_mask2_, std::memory_order_release);
		}

		void commit_write(std::size_t count) {
			auto const write = write_.load(std::memory_order_relaxed);
			write_.store((write + count) & size_mask2_, std::memory_order_release);
		}

		void avail_for_read(
			std::size_t max,
			std::size_t & UNIVERSALIS__COMPILER__RESTRICT_REF begin,
			std::size_t & UNIVERSALIS__COMPILER__RESTRICT_REF size1,
			std::size_t & UNIVERSALIS__COMPILER__RESTRICT_REF size2
		) const {
			auto const read = read_.load(std::memory_order_relaxed);
			begin = read & size_mask_;
			auto const write = write_.load(std::memory_order_acquire);
			auto const avail = std::min(max, (write - read) & size_mask2_);
			if(begin + avail > size_) {
				size1 = size_ - begin;
				size2 = avail - size1;
			} else {
				size1 = avail;
				size2 = 0;
			}
		}

		void avail_for_write(
			std::size_t max,
			std::size_t & UNIVERSALIS__COMPILER__RESTRICT_REF begin,
			std::size_t & UNIVERSALIS__COMPILER__RESTRICT_REF size1,
			std::size_t & UNIVERSALIS__COMPILER__RESTRICT_REF size2
		) const {
			auto const write = write_.load(std::memory_order_relaxed);
			begin = write & size_mask_;
			auto const read = read_.load(std::memory_order_acquire);
			auto const avail = std::min(max, size_ - ((write - read) & size_mask2_));
			if(begin + avail > size_) {
				size1 = size_ - begin;
				size2 = avail - size1;
			} else {
				size1 = avail;
				size2 = 0;
			}
		}
};

/// ring buffer using explict memory barriers
class ring_with_explicit_memory_barriers {
	std::size_t const size_, size_mask_, size_mask2_;
	std::size_t read_, write_;

	public:
		ring_with_explicit_memory_barriers(std::size_t size) : size_(size), size_mask_(size - 1), size_mask2_(size * 2 - 1), read_(), write_() {
			assert("power of 2" && !(size & size_mask_));
		}
		
		std::size_t size() const { return size_; }
		
		void commit_read(std::size_t count) {
			universalis::cpu::memory_barriers::write();
			read_ = (read_ + count) & size_mask2_;
		}

		void commit_write(std::size_t count) {
			universalis::cpu::memory_barriers::write();
			write_ = (write_ + count) & size_mask2_;
		}

		void avail_for_read(
			std::size_t max,
			std::size_t & UNIVERSALIS__COMPILER__RESTRICT_REF begin,
			std::size_t & UNIVERSALIS__COMPILER__RESTRICT_REF size1,
			std::size_t & UNIVERSALIS__COMPILER__RESTRICT_REF size2
		) const {
			begin = read_ & size_mask_;
			universalis::cpu::memory_barriers::read();
			auto const avail = std::min(max, (write_ - read_) & size_mask2_);
			if(begin + avail > size_) {
				size1 = size_ - begin;
				size2 = avail - size1;
			} else {
				size1 = avail;
				size2 = 0;
			}
		}

		void avail_for_write(
			std::size_t max,
			std::size_t & UNIVERSALIS__COMPILER__RESTRICT_REF begin,
			std::size_t & UNIVERSALIS__COMPILER__RESTRICT_REF size1,
			std::size_t & UNIVERSALIS__COMPILER__RESTRICT_REF size2
		) const {
			begin = write_ & size_mask_;
			universalis::cpu::memory_barriers::read();
			auto const avail = std::min(max, size_ - ((write_ - read_) & size_mask2_));
			if(begin + avail > size_) {
				size1 = size_ - begin;
				size2 = avail - size1;
			} else {
				size1 = avail;
				size2 = 0;
			}
		}
};

/// ring buffer using compiler volatile
/// WARNING: It doesn't work reliably on cpu archs with weak memory ordering, and it also suffers from undeterministic wakeups which make it slower.
class ring_with_compiler_volatile {
	std::size_t const size_, size_mask_, size_mask2_;
	std::size_t volatile read_, write_;
	
	public:
		ring_with_compiler_volatile(std::size_t size) : size_(size), size_mask_(size - 1), size_mask2_(size * 2 - 1), read_(), write_() {
			assert("power of 2" && !(size & size_mask_));
		}
		
		std::size_t size() const { return size_; }
		
		void commit_read(std::size_t count) {
			read_ = (read_ + count) & size_mask2_;
		}

		void commit_write(std::size_t count) {
			write_ = (write_ + count) & size_mask2_;
		}

		void avail_for_read(
			std::size_t max,
			std::size_t & UNIVERSALIS__COMPILER__RESTRICT_REF begin,
			std::size_t & UNIVERSALIS__COMPILER__RESTRICT_REF size1,
			std::size_t & UNIVERSALIS__COMPILER__RESTRICT_REF size2
		) const {
			begin = read_ & size_mask_;
			auto const avail = std::min(max, (write_ - read_) & size_mask2_);
			if(begin + avail > size_) {
				size1 = size_ - begin;
				size2 = avail - size1;
			} else {
				size1 = avail;
				size2 = 0;
			}
		}

		void avail_for_write(
			std::size_t max,
			std::size_t & UNIVERSALIS__COMPILER__RESTRICT_REF begin,
			std::size_t & UNIVERSALIS__COMPILER__RESTRICT_REF size1,
			std::size_t & UNIVERSALIS__COMPILER__RESTRICT_REF size2
		) const {
			begin = write_ & size_mask_;
			auto const avail = std::min(max, size_ - ((write_ - read_) & size_mask2_));
			if(begin + avail > size_) {
				size1 = size_ - begin;
				size2 = avail - size1;
			} else {
				size1 = avail;
				size2 = 0;
			}
		}
};

}}

/******************************************************************************************/
#if defined BOOST_AUTO_TEST_CASE
	#include <universalis/compiler/typenameof.hpp>
	#include <thread>
	#include <utility>
	#include <random>
	#include <chrono>
	#include <typeinfo>
	#include <sstream>
	
	namespace psycle { namespace helpers {

	namespace ring_buffer_test_namespace {
	
		template<typename Ring, typename RandGen>
		void writer_loop(std::size_t buf[], Ring & ring, RandGen & rand_gen, std::size_t elements_to_process) {
			std::size_t counter = 0;
			while(counter < elements_to_process) {
				std::size_t begin, size1, size2;
				ring.avail_for_write(rand_gen(), begin, size1, size2);
				if(size1) {
					for(std::size_t i = begin, e = begin + size1; i < e; ++i) buf[i] = ++counter;
					if(size2) {
						for(std::size_t i = 0; i < size2; ++i) buf[i] = ++counter;
						ring.commit_write(size1 + size2);
					} else ring.commit_write(size1);
				}
			}
		}

		template<typename Ring, typename RandGen>
		void reader_loop(std::size_t buf[], Ring & ring, RandGen & rand_gen, std::size_t elements_to_process) {
			std::size_t counter = 0, errors = 0;
			while(counter < elements_to_process) {
				std::size_t begin, size1, size2;
				ring.avail_for_read(rand_gen(), begin, size1, size2);
				if(size1) {
					for(std::size_t i = begin, e = begin + size1; i < e; ++i) errors += buf[i] != ++counter;
					if(size2) {
						for(std::size_t i = 0; i < size2; ++i) errors += buf[i] != ++counter;
						ring.commit_read(size1 + size2);
					} else ring.commit_read(size1);
				}
			}
			BOOST_CHECK(!errors);
		}

		template<typename Ring>
		void test(std::random_device::result_type writer_rand_gen_seed, std::random_device::result_type reader_rand_gen_seed) {
			std::size_t const size = 256;
			std::size_t const elements_to_process = 100 * 1000 * 1000;
			std::size_t buf[size];
			Ring ring(size);
			typedef std::variate_generator<std::mt19937, std::uniform_int<std::size_t>> rand_gen_type;
			rand_gen_type::result_type const rand_gen_dist_lower = ring.size() / 4;
			rand_gen_type::result_type const rand_gen_dist_upper = ring.size() / 2;
			rand_gen_type writer_rand_gen { rand_gen_type::engine_type(reader_rand_gen_seed), rand_gen_type::distribution_type(rand_gen_dist_lower, rand_gen_dist_upper) };
			rand_gen_type reader_rand_gen { rand_gen_type::engine_type(writer_rand_gen_seed), rand_gen_type::distribution_type(rand_gen_dist_lower, rand_gen_dist_upper) };
			{
				std::ostringstream s;
				s <<
					"____________________________\n\n"
					"ring typename: " << universalis::compiler::typenameof(ring) << "\n"
					"ring buffer size: " << size << "\n"
					"elements to process: " << double(elements_to_process) << "\n"
					"rand gen typename: " << universalis::compiler::typenameof(typeid(rand_gen_type)) << "\n"
					"rand gen dist range: " << rand_gen_dist_lower << ' ' << rand_gen_dist_upper << "\n"
					"writer rand gen seed: " << writer_rand_gen_seed << "\n"
					"reader rand gen seed: " << reader_rand_gen_seed;
				BOOST_MESSAGE(s.str());
			}
			BOOST_MESSAGE("running ... ");
			auto const t0 = std::chrono::high_resolution_clock::now();
			std::thread writer_thread(writer_loop<Ring, rand_gen_type>, buf, std::ref(ring), std::ref(writer_rand_gen), elements_to_process);
			std::thread reader_thread(reader_loop<Ring, rand_gen_type>, buf, std::ref(ring), std::ref(reader_rand_gen), elements_to_process);
			writer_thread.join();
			reader_thread.join();
			auto const t1 = std::chrono::high_resolution_clock::now();
			BOOST_MESSAGE("done.");
			auto const duration = std::chrono::nanoseconds(t1 - t0).count() * 1e-9;
			{
				std::ostringstream s;
				s <<
					"duration: " << duration << " seconds\n"
					"troughput: " << elements_to_process / duration << " elements/second\n";
				BOOST_MESSAGE(s.str());
			}
		}
	}
	
	BOOST_AUTO_TEST_CASE(ring_buffer_test) {
		using namespace ring_buffer_test_namespace;
		std::random_device rand_dev;
		std::random_device::result_type const writer_rand_gen_seed = rand_dev();
		std::random_device::result_type const reader_rand_gen_seed = rand_dev();
		test<ring_with_atomic_stdlib>(writer_rand_gen_seed, reader_rand_gen_seed);
		test<ring_with_explicit_memory_barriers>(writer_rand_gen_seed, reader_rand_gen_seed);
		test<ring_with_compiler_volatile>(writer_rand_gen_seed, reader_rand_gen_seed);
	}

	}}
#endif

#endif
