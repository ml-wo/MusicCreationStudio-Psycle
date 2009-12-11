#pragma once
#include <psycle/helpers/math/erase_denormals.hpp>

// Comb filter class declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

class comb
{
public:
					comb();
			void				setbuffer(float *buf, int size);
	inline  float				process(float inp);
			void				mute();
			void				setdamp(float val);
			float				getdamp();
			void				setfeedback(float val);
			float				getfeedback();
private:
	float				feedback;
	float				filterstore;
	float				damp1;
	float				damp2;
	float				*buffer;
	int								bufsize;
	int								bufidx;
};

// Big to inline - but crucial for speed

inline float comb::process(float input)
{
	float output;

	output = buffer[bufidx];
	psycle::helpers::math::fast_erase_denormals_inplace(output);

	filterstore = (output*damp2) + (filterstore*damp1);
	psycle::helpers::math::fast_erase_denormals_inplace(filterstore);

	buffer[bufidx] = input + (filterstore*feedback);

	if(++bufidx>=bufsize) bufidx = 0;

	return output;
}
