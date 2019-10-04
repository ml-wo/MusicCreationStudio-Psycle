// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(SEQUENCE_H)
#define SEQUENCE_H

#include "patterns.h"
#include <list.h>
#include <signal.h>

typedef struct {
	unsigned int pattern;
	beat_t offset;	
} SequenceEntry;

typedef struct {
	List* entries;
} SequenceTrack;

void sequencetrack_init(SequenceTrack*);

typedef List SequenceTrackNode;

typedef struct {
	Patterns* patterns;	
	SequenceTrackNode* tracknode;
	PatternNode* patternnode;		
} SequenceTrackIterator;

void sequencetrackiterator_inc(SequenceTrackIterator*);
PatternNode* sequencetrackiterator_patternnode(SequenceTrackIterator*);
SequenceEntry* sequencetrackiterator_entry(SequenceTrackIterator*);
PatternEntry* sequencetrackiterator_patternentry(SequenceTrackIterator*);
beat_t sequencetrackiterator_offset(SequenceTrackIterator*);

typedef List SequenceTracks;

typedef struct {
	SequenceTracks* track;
	SequenceTrackIterator trackposition;
} SequencePosition;

void sequenceposition_init(SequencePosition*);
SequenceEntry* sequenceposition_entry(SequencePosition*);

typedef struct {
	SequenceTracks* tracks;
	Patterns* patterns;
	SequencePosition editposition;
	Signal signal_editpositionchanged;
} Sequence;

void sequence_init(Sequence*, Patterns*);
void sequence_dispose(Sequence*);
SequenceEntry* sequence_insert(Sequence*, SequencePosition position, int pattern);
void sequence_remove(Sequence*, SequencePosition position);
unsigned int sequence_size(Sequence*, List* track);
SequencePosition sequence_at(Sequence*, unsigned int trackindex, unsigned int position);
SequenceTrackIterator sequence_begin(Sequence*, List* track, beat_t offset);
SequenceTrackIterator sequence_last(Sequence*, List* track);
void sequence_seteditposition(Sequence*, SequencePosition);
SequencePosition sequence_editposition(Sequence* self);
void sequence_clear(Sequence*);
List* sequence_appendtrack(Sequence*, SequenceTrack*);
List* sequence_removetrack(Sequence*, SequenceTracks*);
unsigned int sequence_sizetracks(Sequence*);
int sequence_patternused(Sequence*, unsigned int patternslot);
void sequence_setpatternslot(Sequence*, SequencePosition, unsigned int slot);
beat_t sequence_duration(Sequence*);


#endif

