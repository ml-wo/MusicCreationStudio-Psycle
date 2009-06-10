#pragma once
#include <diversalis/standard_library.hpp>
#if DIVERSALIS__STANDARD_LIBRARY__MATH < 199901
	#include <diversalis/compiler.hpp>
	#include <diversalis/processor.hpp>
#endif
#include <universalis/compiler.hpp>
#include <boost/static_assert.hpp>
#include <cstdint>
#include "fast_unspecified_round_to_integer.hpp"
namespace psycle { namespace helpers { namespace math {

	/// converts a floating point number to an integer by truncating positive numbers toward zero and negative ones toward an unspecified direction
	template<typename Real> UNIVERSALIS__COMPILER__CONST
	std::int32_t inline truncated(Real x)
	{
		return x > 0 ? std::floor(x) : std::ceil(x);
	}
}}}

// inline implementation
namespace psycle { namespace helpers { namespace math {

	#if DIVERSALIS__STANDARD_LIBRARY__MATH >= 199901

		template<> UNIVERSALIS__COMPILER__CONST
		std::int32_t inline truncated<>(long double ld)
		{
			return fast_unspecified_round_to_integer<std::int32_t>(::truncl(ld));
		}

		template<> UNIVERSALIS__COMPILER__CONST
		std::int32_t inline truncated<>(double d)
		{
			return fast_unspecified_round_to_integer<std::int32_t>(::trunc(d));
		}

		template<> UNIVERSALIS__COMPILER__CONST
		std::int32_t inline truncated<>(float f)
		{
			return fast_unspecified_round_to_integer<std::int32_t>(::truncf(f));
		}

	#else

		template<> UNIVERSALIS__COMPILER__CONST
		std::int32_t inline truncated<>(double d)
		{
			BOOST_STATIC_ASSERT((sizeof d == 8));
			union result_union
			{
				double d;
				std::int32_t i;
			} result;
			result.d = d - 0.5 + 6755399441055744.0; // 2^51 + 2^52
			return result.i;
		}

		template<> UNIVERSALIS__COMPILER__CONST
		std::int32_t inline truncated<>(float f)
		{
			#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__COMPILER__MICROSOFT // also intel's compiler?
				///\todo not always the fastest when using sse(2)
				///\todo this custom asm is not very fast on some arch, the double "2^51 + 2^52" version might be faster
				///\todo specify the rounding mode.. is this really a truncation toward -infinity, even with negative numbers?
				std::int32_t i;
				double const half(0.5);
				__asm
				{ 
					fld f;
					fsub half;
					fistp i;
				}
				return i;
			#else
				return truncated(double(f));
			#endif
		}
		
	#endif
	
	#if defined BOOST_AUTO_TEST_CASE
		BOOST_AUTO_TEST_CASE(truncated_test)
		{
			BOOST_CHECK(truncated(+1.6) == +1);
			BOOST_CHECK(truncated(+1.4) == +1);
			BOOST_CHECK(truncated(-1.6) == -2 || truncated(-1.6) == -1);
			BOOST_CHECK(truncated(-1.4) == -2 || truncated(-1.4) == -1);
			BOOST_CHECK(truncated(+1.6f) == +1);
			BOOST_CHECK(truncated(+1.4f) == +1);
			BOOST_CHECK(truncated(-1.6f) == -2 || truncated(-1.6f) == -1);
			BOOST_CHECK(truncated(-1.4f) == -2 || truncated(-1.4f) == -1);
		}
	#endif
}}}