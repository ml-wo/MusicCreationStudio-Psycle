//////////////////////////////////////////////////////////////////////
//
//				Lfo.h
//
//				druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Wavetable.h"

//////////////////////////////////////////////////////////////////////
//				Lfo class
//////////////////////////////////////////////////////////////////////

class Lfo
{
	//////////////////////////////////////////////////////////////////
	//				Variables
	//////////////////////////////////////////////////////////////////

private:

	Wavetable				_waveform;
	float								_phase;
	float								_increment;

	//////////////////////////////////////////////////////////////////
	//				Methods
	//////////////////////////////////////////////////////////////////

public:

	Lfo();
	virtual ~Lfo();
	void Init();
	void Reset();

	inline void SetWaveformSamples(float *samples, int length)
	{
		_waveform.SetSamples(samples, length);
	}

	inline void SetIncrement(float increment)
	{
		_increment = increment;
	}

	//////////////////////////////////////////////////////////////////
	//				GetSample
	//////////////////////////////////////////////////////////////////

	inline float GetSample(float in)
	{
		float out = _waveform.GetSample(_buf, _phase + in, 255);
		_phase = _waveform.ClipPhase(_phase + _increment);
		return out;
	}

};
