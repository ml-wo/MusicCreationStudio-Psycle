#pragma once
#include <diversalis/processor.hpp>
#include <boost/static_assert.hpp>
#include <cstdint>
namespace psycle
{
	namespace common
	{
		namespace math
		{
			/// Cure for malicious samples
			/// Type : Filters Denormals, NaNs, Infinities
			/// References : Posted by urs[AT]u-he[DOT]com
			void inline erase_all_nans_infinities_and_denormals(float & sample)
			{
				#if !defined DIVERSALIS__PROCESSOR__X86
					// just do nothing.. not crucial for other archs
				#else
					BOOST_STATIC_ASSERT((sizeof sample == 4));
					std::uint32_t const bits(reinterpret_cast<std::uint32_t&>(sample));
					std::uint32_t const exponent_mask
					(
						#if defined DIVERSALIS__PROCESSOR__ENDIAN__LITTLE
							0x7f800000
						#else
							#error sorry, was not much thought
						#endif
					);
					std::uint32_t const exponent(bits & exponent_mask);

					// exponent < exponent_mask is 0 if NaN or Infinity, otherwise 1
					std::uint32_t const not_nan_nor_infinity(exponent < exponent_mask);

					// exponent > 0 is 0 if denormalized, otherwise 1
					std::uint32_t const not_denormal(exponent > 0);

					sample *= not_nan_nor_infinity & not_denormal;
				#endif
			}

			///\todo
			void inline erase_all_nans_infinities_and_denormals(double & sample)
			{
			}

			void inline erase_all_nans_infinities_and_denormals(float samples[], unsigned int const sample_count)
			{
				for(unsigned int i(0); i < sample_count; ++i) erase_all_nans_infinities_and_denormals(samples[i]);
			}

			void inline erase_all_nans_infinities_and_denormals(double samples[], unsigned int const sample_count)
			{
				for(unsigned int i(0); i < sample_count; ++i) erase_all_nans_infinities_and_denormals(samples[i]);
			}
		}
	}
}
