//////////////////////////////////////////////////////////////////////
//
//				DspMath.h
//
//				druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#pragma once
//////////////////////////////////////////////////////////////////////
//
//				Constants
//
//////////////////////////////////////////////////////////////////////
//#define PI 3.1415926536f
//#define 2PI 6.2831853072f
//////////////////////////////////////////////////////////////////////
//
//				Convert ms to samples
//
//////////////////////////////////////////////////////////////////////
inline int millis2samples(int ms, int sr)
{
	return ms * sr / 1000;
}
