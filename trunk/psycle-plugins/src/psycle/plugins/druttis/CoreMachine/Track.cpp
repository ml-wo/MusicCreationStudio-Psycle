// -*- mode:c++; indent-tabs-mode:t -*-
//////////////////////////////////////////////////////////////////////
//
//				Track.cpp
//
//				druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////

#include "Track.h"

//////////////////////////////////////////////////////////////////////
//
//				Constructor
//
//////////////////////////////////////////////////////////////////////

Track::Track()
{
}

//////////////////////////////////////////////////////////////////////
//
//				Destructor
//
//////////////////////////////////////////////////////////////////////

Track::~Track()
{
}

//////////////////////////////////////////////////////////////////////
//
//				Stop
//
//////////////////////////////////////////////////////////////////////

void Track::NoteStop()
{
	active = false;
}

//////////////////////////////////////////////////////////////////////
//
//				NoteOff
//
//////////////////////////////////////////////////////////////////////

void Track::NoteOff()
{
	active = false;
}

//////////////////////////////////////////////////////////////////////
//
//				NoteOn
//
//////////////////////////////////////////////////////////////////////

void Track::NoteOn(int note, float velocity)
{
	phase = 0.0f;
	incr = get_freq((float) note, 4096, globals->samplingrate);
	active = true;
}

//////////////////////////////////////////////////////////////////////
//
//				NoteCommand
//
//////////////////////////////////////////////////////////////////////

void Track::NoteCommand(int cmd, int val)
{
}
