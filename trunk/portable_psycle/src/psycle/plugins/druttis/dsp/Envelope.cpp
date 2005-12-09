//////////////////////////////////////////////////////////////////////
//
//	Envelope.cpp
//
//	druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#include <project.private.hpp>
#include <math.h>
#include "Envelope.h"
//////////////////////////////////////////////////////////////////////
//
//	Constructor
//
//////////////////////////////////////////////////////////////////////
Envelope::Envelope()
{
	m_delay = 0;
	m_attack = 0;
	m_decay = 0;
	m_sustain = 0.0f;
	m_length = -1;
	m_release = 0;
	Reset();
}
//////////////////////////////////////////////////////////////////////
//
//	Destructor
//
//////////////////////////////////////////////////////////////////////
Envelope::~Envelope()
{
}
//////////////////////////////////////////////////////////////////////
//
//	GetDelay
//
//////////////////////////////////////////////////////////////////////
int Envelope::GetDelay()
{
	return m_delay;
}
//////////////////////////////////////////////////////////////////////
//
//	SetDelay
//
//////////////////////////////////////////////////////////////////////
void Envelope::SetDelay(int delay)
{
	if (delay < 0)
		delay = 0;
	m_delay = delay;
}
//////////////////////////////////////////////////////////////////////
//
//	GetAttack
//
//////////////////////////////////////////////////////////////////////
int Envelope::GetAttack()
{
	return m_attack;
}
//////////////////////////////////////////////////////////////////////
//
//	SetAttack
//
//////////////////////////////////////////////////////////////////////
void Envelope::SetAttack(int attack)
{
	if (attack < 0)
		attack = 0;
	m_attack = attack;
}
//////////////////////////////////////////////////////////////////////
//
//	GetDecay
//
//////////////////////////////////////////////////////////////////////
int Envelope::GetDecay()
{
	return m_decay;
}
//////////////////////////////////////////////////////////////////////
//
//	SetDecay
//
//////////////////////////////////////////////////////////////////////
void Envelope::SetDecay(int decay)
{
	if (decay < 0)
		decay = 0;
	m_decay = decay;
}
//////////////////////////////////////////////////////////////////////
//
//	GetSustain
//
//////////////////////////////////////////////////////////////////////
float Envelope::GetSustain()
{
	return m_sustain;
}
//////////////////////////////////////////////////////////////////////
//
//	SetSustain
//
//////////////////////////////////////////////////////////////////////
void Envelope::SetSustain(float sustain)
{
	if (sustain < 0.0f)
	{
		sustain = 0.0f;
	}
	else if (sustain > 1.0f)
	{
		sustain = 1.0f;
	}
	m_sustain = sustain;
}
//////////////////////////////////////////////////////////////////////
//
//	GetLength
//
//////////////////////////////////////////////////////////////////////
int Envelope::GetLength()
{
	return m_length;
}
//////////////////////////////////////////////////////////////////////
//
//	SetLength
//
//////////////////////////////////////////////////////////////////////
void Envelope::SetLength(int length)
{
	if (length < -1)
		length = -1;
	m_length = length;
}
//////////////////////////////////////////////////////////////////////
//
//	GetRelease
//
//////////////////////////////////////////////////////////////////////
int Envelope::GetRelease()
{
	return m_release;
}
//////////////////////////////////////////////////////////////////////
//
//	SetRelease
//
//////////////////////////////////////////////////////////////////////
void Envelope::SetRelease(int release)
{
	if (release < 0)
		release = 0;
	m_release = release;
	if (m_state == ENVELOPE_RELEASE)
	{
		if (m_release > 0)
		{
			m_ticks = m_release;
			m_decr = 1;
			m_coeff = -m_value / (float) m_ticks;
		}
		else
		{
			Reset();
		}
	}
}
//////////////////////////////////////////////////////////////////
//
//	Start
//
//////////////////////////////////////////////////////////////////
void Envelope::Start()
{
	m_state = ENVELOPE_IDLE;
	m_ticks = 0;
	Clip(0);
}
//////////////////////////////////////////////////////////////////
//
//	Stop
//
//////////////////////////////////////////////////////////////////
void Envelope::Stop()
{
	if ((m_state != ENVELOPE_IDLE) && (m_state != ENVELOPE_RELEASE))
	{
		if (m_release > 0)
		{
			m_state = ENVELOPE_RELEASE;
			m_ticks = m_release;
			m_decr = 1;
			m_coeff = -m_value / (float) m_ticks;
		}
		else
		{
			Reset();
		}
	}
}
//////////////////////////////////////////////////////////////////
//
//	Clip
//
//////////////////////////////////////////////////////////////////
int Envelope::Clip(int nsamples)
{
	while (m_ticks <= 0)
	{
		switch (m_state)
		{
		case ENVELOPE_IDLE :
			m_state = ENVELOPE_DELAY;
			if (m_delay > 0)
			{
				m_ticks = m_delay;
				m_decr = 1;
				m_coeff = 0.0f;
			}
			break;
		case ENVELOPE_DELAY :
			m_state = ENVELOPE_ATTACK;
			if (m_attack > 0)
			{
				m_ticks = m_attack;
				m_decr = 1;
				m_coeff = (1.0f - m_value) / (float) m_ticks;
			}
			break;
		case ENVELOPE_ATTACK :
			m_state = ENVELOPE_DECAY;
			if (m_decay > 0)
			{
				m_ticks = m_decay;
				m_decr = 1;
				m_value = 1.0f;
				m_coeff = (m_sustain - 1.0f) / (float) m_ticks;
			}
			break;
		case ENVELOPE_DECAY :
			m_state = ENVELOPE_SUSTAIN;
			if (m_sustain > 0.0f)
			{
				m_value = m_sustain;
				m_coeff = 0.0f;
				if (m_length == -1)
				{
					m_ticks = ENVELOPE_MAXTICKS;
					m_decr = 0;
				}
				else
				{
					m_ticks = m_length;
					m_decr = 1;
				}
			}
			break;
		case ENVELOPE_SUSTAIN :
			m_state = ENVELOPE_RELEASE;
			if (m_release > 0)
			{
				m_ticks = m_release;
				m_decr = 1;
				m_coeff = -m_value / (float) m_ticks;
			}
			break;
		case ENVELOPE_RELEASE :
			Reset();
			break;
		}
	}
	return (nsamples < m_ticks ? nsamples : m_ticks);
}
//////////////////////////////////////////////////////////////////
//
//	Skip
//
//////////////////////////////////////////////////////////////////
void Envelope::Skip(int nsamples)
{
	int amt;
	do
	{
		amt = Clip(nsamples);
		nsamples -= amt;
		m_ticks -= m_decr * amt;
		m_value += m_coeff * (float) amt;
	}
	while (nsamples);
}
//////////////////////////////////////////////////////////////////
//
//	Fill
//
//////////////////////////////////////////////////////////////////
void Envelope::Fill(float *pout, int nsamples)
{
	int amt;
	--pout;
	do
	{
		amt = Clip(nsamples);
		nsamples -= amt;
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
//	Add
//
//////////////////////////////////////////////////////////////////
void Envelope::Add(float *pout, float *pin, int nsamples)
{
	int amt;
	--pin;
	--pout;
	do
	{
		amt = Clip(nsamples);
		nsamples -= amt;
		do
		{
			*++pout = Next() + *++pin;
		}
		while (--amt);
	}
	while (nsamples);
}
