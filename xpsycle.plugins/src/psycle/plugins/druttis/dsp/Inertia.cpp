//////////////////////////////////////////////////////////////////////
//
//	Inertia.cpp
//
//	druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#include <packageneric/pre-compiled.private.hpp>
#include "Inertia.h"
#include <cmath>
//////////////////////////////////////////////////////////////////////
//
//	Constructor
//
//////////////////////////////////////////////////////////////////////
Inertia::Inertia()
{
	m_length = 0;
	m_target = 0.0f;
	m_ticks = 0;
	m_decr = 1;
	m_value = 0.0f;
	m_step = 0.0f;
	m_valid = false;
}
//////////////////////////////////////////////////////////////////////
//
//	Destructor
//
//////////////////////////////////////////////////////////////////////
Inertia::~Inertia()
{
}
//////////////////////////////////////////////////////////////////////
//
//	GetLength
//
//////////////////////////////////////////////////////////////////////
int Inertia::GetLength()
{
	return m_length;
}
//////////////////////////////////////////////////////////////////////
//
//	SetLength
//
//////////////////////////////////////////////////////////////////////
void Inertia::SetLength(int length)
{
	if (length != m_length)
	{
		m_length = length;
		Update();
	}
}
//////////////////////////////////////////////////////////////////////
//
//	GetTarget
//
//////////////////////////////////////////////////////////////////////
float Inertia::GetTarget()
{
	return m_target;
}
//////////////////////////////////////////////////////////////////////
//
//	SetTarget
//
//////////////////////////////////////////////////////////////////////
void Inertia::SetTarget(float target)
{
	if (target != m_target)
	{
		m_target = target;
		Update();
	}
}
//////////////////////////////////////////////////////////////////////
//
//	GetValue
//
//////////////////////////////////////////////////////////////////////
float Inertia::GetValue()
{
	return m_value;
}
//////////////////////////////////////////////////////////////////
//
//	Update
//
//////////////////////////////////////////////////////////////////
void Inertia::Update()
{
	if(m_length > 0 && m_target != m_value)
	{
		m_ticks = m_length;
		m_decr = 1;
		m_step = (m_target - m_value) / m_ticks;
	}
	else
	{
		m_ticks = 0;
		m_decr = 0;
		m_value = m_target;
		m_step = 0;
	}
	m_valid = false;
}
//////////////////////////////////////////////////////////////////
//
//	Fill
//
//////////////////////////////////////////////////////////////////
void Inertia::Fill(float *pout, int nsamples)
{
	int amt;
	--pout;
	do
	{
		amt = Clip(nsamples);
		nsamples -= amt;
		if (amt > 2)
		{
			do
			{
				*++pout = Next();
				*++pout = Next();
				amt -= 2;
			}
			while (amt > 2);
		}
		do
		{
			*++pout = Next();
		}
		while (--amt);
	}
	while (nsamples);
}
//////////////////////////////////////////////////////////////////
//
//	Fill
//
//////////////////////////////////////////////////////////////////
void Inertia::Fill(float *pbuf, float mul, int nsamples)
{
	int amt;
	--pbuf;
	do
	{
		amt = Clip(nsamples);
		nsamples -= amt;
		if (amt > 2)
		{
			do
			{
				*++pbuf = Next() * mul;
				*++pbuf = Next() * mul;
				amt -= 2;
			}
			while (amt > 2);
		}
		do
		{
			*++pbuf = Next() * mul;
		}
		while (--amt);
	}
	while (nsamples);
}
