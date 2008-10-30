//////////////////////////////////////////////////////////////////////////////
//
//				Chorus.h
//
//				druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////////
//				CChorus class
//////////////////////////////////////////////////////////////////////////////

class CChorus  
{
private:
	CDelayL				_delay;
	float				_lfo_phase;
	float				_lfo_inc;
	float				_mix;
public:
	CChorus();
	virtual ~CChorus();
	void Init(int maxDelay);
	void Clear();
	void SetMinDelay(
};
