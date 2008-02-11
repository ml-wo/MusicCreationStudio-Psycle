// -*- mode:c++; indent-tabs-mode:t -*-
//////////////////////////////////////////////////////////////////////
//
//				Biquad.h
//
//				druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#pragma once
#include <cmath>
//////////////////////////////////////////////////////////////////////
//
//				PI constants
//
//////////////////////////////////////////////////////////////////////
#define PI								3.14159265358979323846
#define PI2								6.28318530717958647693
#define HALFPI				1.57079632679489661923
#define LN								0.69314718055994530942

enum
{
	LPF,
	HPF,
	BPF,
	NOTCH,
	PEQ,
	LSH,
	HSH
};

//////////////////////////////////////////////////////////////////////
//
//				Biquad class
//
//////////////////////////////////////////////////////////////////////
class Biquad
{
	//////////////////////////////////////////////////////////////////
	//
	//				Variables
	//
	//////////////////////////////////////////////////////////////////
private :
	double				a0, a1, a2;
	double				b0, b1, b2;

	double				c0, c1, c2, c3, c4;

	double				x1, x2;
	double				y1, y2;

	//////////////////////////////////////////////////////////////////
	//
	//				Constructor
	//
	//////////////////////////////////////////////////////////////////
public :
	Biquad();

	//////////////////////////////////////////////////////////////////
	//
	//				Destructor
	//
	//////////////////////////////////////////////////////////////////
	~Biquad();

	//////////////////////////////////////////////////////////////////
	//
	//				Reset
	//
	//////////////////////////////////////////////////////////////////
	void Reset();

	//////////////////////////////////////////////////////////////////
	//
	//				Init
	//
	//////////////////////////////////////////////////////////////////
	void Init(int type, float dbGain, float freq, int sr, float bandwidth);

	//////////////////////////////////////////////////////////////////
	//
	//				Next
	//
	//////////////////////////////////////////////////////////////////
	inline float Next(float in)
	{
		double out = c0 * (double) in + c1 * x1 + c2 * x2 - c3 * y1 - c4 * y2;
		x2 = x1;
		x1 = (double) in;
		y2 = y1;
		y1 = out;
		return (float) out;
	}
};
