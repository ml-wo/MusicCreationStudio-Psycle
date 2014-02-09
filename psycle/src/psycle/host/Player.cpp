///\file
///\brief implementation file for psycle::host::Player.

#include <psycle/host/detail/project.private.hpp>
#include "Player.hpp"
#include "Song.hpp"
#include "Machine.hpp"
#include "Configuration.hpp"

//For the patch in compute_plan()
#include "internal_machines.hpp"

#if !defined WINAMP_PLUGIN
	#include "MidiInput.hpp"
	#include "InputHandler.hpp"
#endif //!defined WINAMP_PLUGIN

#include "cpu_time_clock.hpp"
#include <universalis/os/thread_name.hpp>
#include <universalis/os/sched.hpp>
#include <universalis/os/aligned_alloc.hpp>
#include <psycle/helpers/value_mapper.hpp>
#include <seib-vsthost/CVSTHost.Seib.hpp> // Included to interact directly with the host.
#include <psycle/helpers/math.hpp>
namespace psycle
{
	namespace host
	{
		using namespace seib::vst;
		using namespace psycle::helpers::math;
		namespace {
			static thread_local bool this_thread_suspended_ = false;
		}
		Player::Player()
		{
			_playing = false;
			_playBlock = false;
			_recording = false;
			_clipboardrecording = false;
			_isWorking = false;
			_samplesRemaining=0;
			sampleCount=0;
			_lineCounter=0;
			_lineStop=-1;
			_loopSong=true;
			_patternjump=-1;
			_linejump=-1;
			_loop_count=0;
			_loop_line=0;
			_playPosition=0;
			measure_cpu_usage_=false;
			m_SampleRate=44100;
			m_extraTicks=0;
			SetBPM(125,4);
			for(int i=0;i<MAX_TRACKS;i++) {
				prevMachines[i]=255;
				prevInstrument[i]=255;
				playTrack[i]=false;
			}
			universalis::os::aligned_memory_alloc(16, _pBuffer, MAX_SAMPLES_WORKFN);
		}

		Player::~Player()
		{
			//This should be called explicitely on application/plugin close.
			//It is here just as a safe measure, but was causing a deadlock when used in in_psycle and xmplay.
			stop_threads();
			if(_recording) _outputWaveFile.Close();
			universalis::os::aligned_memory_dealloc(_pBuffer);
		}

void Player::start_threads(int thread_count) {
	loggers::trace()("psycle: core: player: starting scheduler threads", UNIVERSALIS__COMPILER__LOCATION);
	if(!threads_.empty()) {
		loggers::trace()("psycle: core: player: scheduler threads are already running", UNIVERSALIS__COMPILER__LOCATION);
		return;
	}
	if(thread_count <= 0) {
		thread_count = thread::hardware_concurrency();
	}

	// normally, we would compute the scheduling plan here,
	// but we haven't yet implemented signals that notifies when connections are changed or nodes added/removed.
	// so it's actually done every time in the processing loop
	//compute_plan();

	stop_requested_ = suspend_requested_ = false;
	processed_node_count_ = suspended_ = waiting_ = 0;

	if(loggers::information()) {
		std::ostringstream s;
		s << "psycle: core: player: using " << thread_count << " threads";
		loggers::information()(s.str());
	}

	if(thread_count < 2) return; // don't create any thread, will use a single-threaded, recursive processing

	try {
		// start the scheduling threads
		for(std::size_t i(0); i < thread_count; ++i)
			threads_.push_back(new thread(boost::bind(&Player::thread_function, this, i)));
	} catch(...) {
		{ scoped_lock lock(mutex_);
			stop_requested_ = true;
		}
		condition_.notify_all();
		for(threads_type::const_iterator i(threads_.begin()), e(threads_.end()); i != e; ++i) {
			(**i).join();
			delete *i;
		}
		threads_.clear();
		clear_plan();
		throw;
	}
}


		void Player::Start(int pos, int lineStart, int lineStop, bool initialize)
		{
			_lineStop = lineStop;
			Start(pos, lineStart, initialize);
		}

		void Player::Start(int pos, int line, bool initialize)
		{
			CExclusiveLock lock(&Global::song().semaphore, 2, true);
			if (initialize)
			{
				DoStop(); // This causes all machines to reset, and samplesperRow to init.
				bool recording = _recording;
				_recording = false;
				Work(256);
				_recording = recording;
				static_cast<Master*>(Global::song()._pMachine[MASTER_INDEX])->_clip = false;
			}
			_lineChanged = true;
			_lineCounter = line;
			_SPRChanged = false;
			_playPosition= pos;
			_playPattern = Global::song().playOrder[_playPosition];
			if(pos != 0 || line != 0) {
				int songLength = CalcOrSeek(Global::song(),pos,line);
				sampleCount=songLength*m_SampleRate;
			}
			else sampleCount=0;

			if (initialize)
			{
				_playTime = 0;
				_playTimem = 0;
			}
			_loop_count =0;
			_loop_line = 0;
			if (initialize)
			{
				SetBPM(Global::song().BeatsPerMin(),Global::song().LinesPerBeat());
				if (!_recording) {
					SampleRate(Global::configuration()._pOutputDriver->GetSamplesPerSec());
				}
				for(int i=0;i<MAX_TRACKS;i++) 
				{
					prevMachines[i] = 255;
					prevInstrument[i] = 255;
				}
				_playing = true;
			}
			CVSTHost::vstTimeInfo.flags |= kVstTransportPlaying;
			CVSTHost::vstTimeInfo.flags |= kVstTransportChanged;
			ExecuteLine();
			_samplesRemaining = SamplesPerRow();
		}

		void Player::Stop(void)
		{
			CExclusiveLock lock(&Global::song().semaphore, 2, true);
			DoStop();
		}
		void Player::DoStop(void)
		{
			if (_playing == true)
				_lineStop = -1;

			// Stop song enviroment
			_playing = false;
			_playBlock = false;			
			for(int i=0; i<MAX_MACHINES; i++)
			{
				if(Global::song()._pMachine[i])
				{
					Global::song()._pMachine[i]->Stop();
					for(int c = 0; c < MAX_TRACKS; c++) Global::song()._pMachine[i]->TriggerDelay[c]._cmd = 0;
				}
			}
			for(int i=0;i<MAX_TRACKS;i++) {
				playTrack[i]=false;
			}
			Global::song().wavprev.Stop();
			SetBPM(Global::song().BeatsPerMin(),Global::song().LinesPerBeat());
			if (!_recording) {
				SampleRate(Global::configuration()._pOutputDriver->GetSamplesPerSec());
			}
			CVSTHost::vstTimeInfo.flags &= ~kVstTransportPlaying;
			CVSTHost::vstTimeInfo.flags |= kVstTransportChanged;
		}
		void Player::SetSampleRate(const int sampleRate) {
			CExclusiveLock lock(&Global::song().semaphore, 2, true);
			SampleRate(sampleRate);
		}
		void Player::SampleRate(const int sampleRate)
		{
#if PSYCLE__CONFIGURATION__RMS_VUS
			helpers::dsp::numRMSSamples=sampleRate*0.05f;
#endif
			if(m_SampleRate != sampleRate)
			{
				m_SampleRate = sampleRate;
				RecalcSPR();
				CVSTHost::pHost->SetSampleRate(sampleRate);
				for(int i(0) ; i < MAX_MACHINES; ++i)
				{
					if(Global::song()._pMachine[i]) Global::song()._pMachine[i]->SetSampleRate(sampleRate);
				}
			}
		}
		void Player::SetBPM(int _bpm,int _lpb)
		{
			if ( _lpb != 0) { lpb=_lpb; m_extraTicks = 0;}
			if ( _bpm != 0) { bpm=_bpm; }
			RecalcSPR();
			CVSTHost::vstTimeInfo.tempo = bpm;
			CVSTHost::vstTimeInfo.flags |= kVstTransportChanged;
			CVSTHost::vstTimeInfo.flags |= kVstTempoValid;
		}

//This method should be called from an exclusively locked thread (since changing the plan in the middle
//of work can cause unexpected problems (this is especially true with the send_return mixer.
void Player::suspend_and_compute_plan() {
	// before we compute the plan, we need to suspend all the threads.
	{ scoped_lock lock(mutex_);
		suspend_requested_ = true;
	}
	condition_.notify_all();  // notify all threads they must suspend
	{ scoped_lock lock(mutex_);
		// wait until all threads are suspended
		while(suspended_ != threads_.size()) main_condition_.wait(lock);
		compute_plan();
		suspend_requested_ = false;
	}
	condition_.notify_all(); // notify all threads they can resume
}

void Player::compute_plan() {
	graph_size_ = 0;
	terminal_nodes_.clear();

	// iterate over all the nodes
	Song& song = Global::song();
	for(int m(0); m < MAX_MACHINES; ++m) if(song._pMachine[m]) {
		++graph_size_;
		Machine & n(*song._pMachine[m]);
		if(n._type == MACH_MIXER) {
			static_cast<Mixer*>(&n)->InitialWorkState();
		}
		// find the terminal nodes in the graph (nodes with no connected input ports)
		input_nodes_.clear(); n.sched_inputs(input_nodes_);
		if(input_nodes_.empty()) terminal_nodes_.push_back(&n);
	}

	// copy the initial processing queue
	nodes_queue_ = terminal_nodes_;
}

void Player::clear_plan() {
	nodes_queue_.clear();
	terminal_nodes_.clear();
}


		void Player::ExecuteLine(void)
		{
			// Global commands are executed first so that the values for BPM and alike
			// are up-to-date when "NotifyNewLine()" is called.
			ExecuteGlobalCommands();
			NotifyNewLine();
			ExecuteNotes();
			NotifyPostNewLine();
		}
		// Initial Loop. Read new line and Interpret the Global commands.
		void Player::ExecuteGlobalCommands(void)
		{
			Song& song = Global::song();
			_patternjump = -1;
			_linejump = -1;
			int mIndex = 0;
			unsigned char* const plineOffset = song._ptrackline(_playPattern,0,_lineCounter);

			for(int track=0; track<song.SONGTRACKS; track++)
			{
				PatternEntry* pEntry = reinterpret_cast<PatternEntry*>(plineOffset + track*EVENT_SIZE);
				// If This isn't a tweak (twk/tws/mcm) then do
				if(pEntry->_note < notecommands::tweak || pEntry->_note == notecommands::empty)
				{
					switch(pEntry->_cmd)
					{
					case PatternCmd::SET_TEMPO:
						if(pEntry->_parameter != 0)
						{	///\todo: implement the Tempo slide
							// SET_SONG_TEMPO=			20, // T0x Slide tempo down . T1x slide tempo up
							if (pEntry->_parameter < 0x20) {
							}
							else {
								SetBPM(pEntry->_parameter);
							}
						}
						break;
					case PatternCmd::EXTENDED:
						if(pEntry->_parameter != 0)
						{
							if ( (pEntry->_parameter&0xE0) == 0 ) // range from 0 to 1F for LinesPerBeat.
							{
								SetBPM(0,pEntry->_parameter);
							}
							else if ( (pEntry->_parameter&0xF0) == PatternCmd::SET_BYPASS )
							{
								mIndex = pEntry->_mach;
								if ( mIndex < MAX_MACHINES && song._pMachine[mIndex] && song._pMachine[mIndex]->_mode == MACHMODE_FX )
								{
									if ( pEntry->_parameter&0x0F )
										song._pMachine[mIndex]->Bypass(true);
									else
										song._pMachine[mIndex]->Bypass(false);
								}
							}

							else if ( (pEntry->_parameter&0xF0) == PatternCmd::SET_MUTE )
							{
								mIndex = pEntry->_mach;
								if ( mIndex < MAX_MACHINES && song._pMachine[mIndex] && song._pMachine[mIndex]->_mode != MACHMODE_MASTER )
								{
									if ( pEntry->_parameter&0x0F )
										song._pMachine[mIndex]->_mute = true;
									else
										song._pMachine[mIndex]->_mute = false;
								}
							}
							else if ( (pEntry->_parameter&0xF0) == PatternCmd::PATTERN_DELAY )
							{
								int memextra = m_extraTicks;
								m_extraTicks = 24/lpb * (pEntry->_parameter&0x0F);
								RecalcSPR();
								m_extraTicks = memextra;
								_SPRChanged=true;
							}
							else if ( (pEntry->_parameter&0xF0) == PatternCmd::MEMORY_FINE_PAT_DELAY )
							{
								m_extraTicks = pEntry->_parameter&0x0F;
								RecalcSPR();
							}
							else if ( (pEntry->_parameter&0xF0) == PatternCmd::FINE_PATTERN_DELAY)
							{
								int memextra = m_extraTicks;
								m_extraTicks = pEntry->_parameter&0x0F;
								RecalcSPR();
								m_extraTicks = memextra;
								_SPRChanged=true;
							}
							else if ( (pEntry->_parameter&0xF0) == PatternCmd::PATTERN_LOOP)
							{
								int value = pEntry->_parameter&0x0F;
								if (value == 0 )
								{
									_loop_line = _lineCounter;
								} else {
									if ( _loop_count == 0 )
									{ 
										_loop_count = value;
										_linejump = _loop_line;
									} else {
										if (--_loop_count) _linejump = _loop_line;
										else _loop_line = _lineCounter+1; //This prevents infinite loop in specific cases.
									}
								}
							}
						}
						break;
					case PatternCmd::JUMP_TO_ORDER:
						if ( pEntry->_parameter < song.playLength ) {
							_patternjump=pEntry->_parameter;
							_linejump=0;
						}
						break;
					case PatternCmd::BREAK_TO_LINE:
						if (_patternjump ==-1) {
							_patternjump=(_playPosition+1>=song.playLength)?0:_playPosition+1;
						}
						{
							int lines = song.patternLines[song.playOrder[_playPosition]];
							if ( pEntry->_parameter >= lines) {
								_linejump = lines-1;
							}
							else { _linejump= pEntry->_parameter; }
						}
						break;
					case PatternCmd::SET_VOLUME:
						if(pEntry->_mach == 255) {
							static_cast<Master*>(song._pMachine[MASTER_INDEX])->_outDry = pEntry->_parameter;
						}
						else {
							int mIndex = pEntry->_mach;
							if(mIndex < MAX_MACHINES && song._pMachine[mIndex]) {
								song._pMachine[mIndex]->SetDestWireVolume(pEntry->_inst, helpers::value_mapper::map_256_1(pEntry->_parameter));
							}
						}
						break;
					case  PatternCmd::SET_PANNING:
						mIndex = pEntry->_mach;
						if(mIndex < MAX_MACHINES && song._pMachine[mIndex]) {
							song._pMachine[mIndex]->SetPan(pEntry->_parameter>>1);
						}
						break;
					}
				}
				// Check For Tweak or MIDI CC
				else if(!song._trackMuted[track])
				{
					if(pEntry->_mach < MAX_MACHINES) prevMachines[track] = pEntry->_mach;

					int mac = prevMachines[track];
					if(mac < MAX_MACHINES && song._pMachine[mac]) {
						if(pEntry->_note == notecommands::midicc && (pEntry->_inst < MAX_TRACKS || pEntry->_inst == 0xFF))
						{
							int voice(pEntry->_inst);
							// make a copy of the pattern entry, because we're going to modify it.
							PatternEntry entry(*pEntry);
							// check for out of range voice values.
							if(voice < song.SONGTRACKS)
							{
								song._pMachine[mac]->Tick(voice, &entry);
							}
							else if(voice == 0xFF)
							{
								entry._inst = 0x00;
								// special voice value which means we want to send the same command to all voices
								for(int voice(0) ; voice < song.SONGTRACKS ; ++voice)
								{
									song._pMachine[mac]->Tick(voice, &entry);
								}
							}
						}
						else {
							song._pMachine[mac]->Tick(track, pEntry);
						}
					}
				}
			}
		}

			// Notify all machines that a new Line comes.
		void Player::NotifyNewLine(void)
		{
			Song& song = Global::song();
			for(int tc=0; tc<MAX_MACHINES; tc++)
			{
				if(song._pMachine[tc])
				{
					song._pMachine[tc]->NewLine();
					for(int c = 0; c < MAX_TRACKS; c++) song._pMachine[tc]->TriggerDelay[c]._cmd = 0;
				}
			}

		}

		/// Final Loop. Read new line for notes to send to the Machines
		void Player::ExecuteNotes(void)
		{
			Song& song = Global::song();
			unsigned char* const plineOffset = song._ptrackline(_playPattern,0,_lineCounter);


			for(int track=0; track<song.SONGTRACKS; track++)
			{
				PatternEntry* pEntry = reinterpret_cast<PatternEntry*>(plineOffset + track*EVENT_SIZE);
				if(( !song._trackMuted[track]) && 
					(pEntry->_note < notecommands::tweak || pEntry->_note == notecommands::empty)) // Is it not muted and is a note or command?
				{
					int mac = pEntry->_mach;
					if(mac < MAX_MACHINES) prevMachines[track] = mac;
					else mac = prevMachines[track];
					if( mac != 255 && (pEntry->_inst != 255 || pEntry->_note != 255 || pEntry->_cmd != 0 || pEntry->_parameter != 0) ) // is there a machine number and it is either a note or a command?
					{
						if(mac < MAX_MACHINES && song._pMachine[mac] != NULL) //looks like a valid machine index?
						{
							Machine *pMachine = song._pMachine[mac];
							//XMSampler maintains this, and also has special meanings when no instrument is set.
							int ins = pEntry->_inst;
							if(pMachine->_type != MACH_XMSAMPLER) {
								if(pEntry->_inst != 255) prevInstrument[track] = pEntry->_inst;
								else ins = prevInstrument[track];
							}
							if(pEntry->_note == notecommands::release
								 && pMachine->_type != MACH_SAMPLER && pMachine->_type != MACH_XMSAMPLER) {
								playTrack[track]=false;
							}
							else if(pEntry->_note <= notecommands::b9){
								playTrack[track]=true;
							}
							if(!pMachine->_mute) // Does this machine really exist and is not muted?
							{
								// make a copy of the pattern entry, because we're going to modify it.
								PatternEntry entry(*pEntry);
								entry._inst = ins;
								if(entry._cmd == PatternCmd::NOTE_DELAY)
								{
									// delay
									memcpy(&pMachine->TriggerDelay[track], &entry, sizeof(PatternEntry));
									pMachine->TriggerDelayCounter[track] = ((entry._parameter+1)*SamplesPerRow())/256;
								}
								else if(entry._cmd == PatternCmd::RETRIGGER)
								{
									// retrigger
									memcpy(&pMachine->TriggerDelay[track], &entry, sizeof(PatternEntry));
									pMachine->RetriggerRate[track] = (entry._parameter+1);
									pMachine->TriggerDelayCounter[track] = 0;
								}
								else if(entry._cmd == PatternCmd::RETR_CONT)
								{
									// retrigger continue
									memcpy(&pMachine->TriggerDelay[track], &entry, sizeof(PatternEntry));
									if(entry._parameter&0xf0) pMachine->RetriggerRate[track] = (entry._parameter&0xf0);
								}
								else if (entry._cmd == PatternCmd::ARPEGGIO)
								{
									// arpeggio
									//\todo : Add Memory.
									//\todo : This won't work... What about sampler's NNA's?
									if (entry._parameter)
									{
										memcpy(&pMachine->TriggerDelay[track], &entry, sizeof(PatternEntry));
										pMachine->ArpeggioCount[track] = 1;
									}
									pMachine->RetriggerRate[track] = SamplesPerRow()*lpb/24;
								}
								else 
								{
									pMachine->TriggerDelay[track]._cmd = 0;
									pMachine->Tick(track, &entry);
									pMachine->TriggerDelayCounter[track] = 0;
									pMachine->ArpeggioCount[track] = 0;
								}
							}
						}
					}
				}
			}
		}	

			// Notify all machines that a new Line comes.
		void Player::NotifyPostNewLine(void)
		{
			Song& song = Global::song();
			for(int tc=0; tc<MAX_MACHINES; tc++)
			{
				if(song._pMachine[tc])
				{
					song._pMachine[tc]->PostNewLine();
				}
			}
		}

		void Player::AdvancePosition()
		{
			Song& song = Global::song();
			if ( _patternjump!=-1 ) _playPosition= _patternjump;
			if ( _SPRChanged ) { RecalcSPR(); _SPRChanged = false; }
			if ( _linejump!=-1 ) _lineCounter=_linejump;
			else _lineCounter++;
			_playTime += 60 / float (bpm * lpb);
			if(_playTime>60)
			{
				_playTime-=60;
				_playTimem++;
			}
			if(_lineCounter >= song.patternLines[_playPattern] || _lineCounter==_lineStop)
			{
				_lineCounter = 0;
				if(!_playBlock)
					_playPosition++;
				else
				{
					_playPosition++;
					while(_playPosition< song.playLength && (!song.playOrderSel[_playPosition]))
						_playPosition++;
				}
			}
			if( _playPosition >= song.playLength)
			{	
				_playPosition = 0;
				if( _loopSong )
				{
					if(( _playBlock) && (song.playOrderSel[_playPosition] == false))
					{
						while((!song.playOrderSel[_playPosition]) && ( _playPosition< song.playLength)) _playPosition++;
					}
				}
				else 
				{
					_playing = false;
					_playBlock = false;
					_lineStop = -1;
					StopRecording();
				}
			}
			// this is outside the if, so that _patternjump works
			_playPattern = song.playOrder[_playPosition];
			_lineChanged = true;
		}

int Player::CalcOrSeek(Song& song, int seqPos, int patLine, int seektime_ms,bool allowLoop)
{
	float songLength = 0;
	float seektime = seektime_ms/1000.f;
	int bpm_calc = song.BeatsPerMin();
	int lpb_calc = song.LinesPerBeat();
	int extratick_calc = 0;
	float lineSeconds = (60.f/bpm_calc)*((1.f/lpb_calc) + (extratick_calc/24.f));
	int playPos=0;
	while(playPos <song.playLength)
	{
		int pattern = song.playOrder[playPos];
		unsigned char* const plineOffset = song._ppattern(pattern);
		int loopCount=0;
		int loopLine=0;
		int patternJump = -1;
		int playLine=0;
		int l = 0;
		while( playLine < song.patternLines[pattern])
		{
			int lineJump = -1;
			int resetLineSec = false;
			for(int track=0; track<song.SONGTRACKS; track++)
			{
				PatternEntry* pEntry = reinterpret_cast<PatternEntry*>(plineOffset+l+track*EVENT_SIZE);

				if(pEntry->_note < notecommands::tweak || pEntry->_note == notecommands::empty) // If This isn't a tweak (twk/tws/mcm) then do
				{
					switch(pEntry->_cmd)
					{
					case PatternCmd::SET_TEMPO:
						if(pEntry->_parameter != 0)
						{
							bpm_calc=pEntry->_parameter;
							lineSeconds = (60.f/bpm_calc)*((1.f/lpb_calc) + (extratick_calc/24.f));
						}
						break;
					case PatternCmd::EXTENDED:
						if(pEntry->_parameter != 0)
						{
							if ( (pEntry->_parameter&0xE0) == 0 ) // range from 0 to 1F for LinesPerBeat.
							{
								lpb_calc=pEntry->_parameter;
								extratick_calc=0;
								lineSeconds = (60.f/bpm_calc)*((1.f/lpb_calc) + (extratick_calc/24.f));
							}
							else if ( (pEntry->_parameter&0xF0) == PatternCmd::PATTERN_DELAY )
							{
								lineSeconds = (60.f/bpm_calc)*((1.f/lpb_calc) + (pEntry->_parameter&0x0F));
								resetLineSec=true;
							}
							else if ( (pEntry->_parameter&0xF0) == PatternCmd::MEMORY_FINE_PAT_DELAY)
							{
								extratick_calc=pEntry->_parameter&0x0F;
								lineSeconds = (60.f/bpm_calc)*((1.f/lpb_calc) + (extratick_calc/24.f));
							}
							else if ( (pEntry->_parameter&0xF0) == PatternCmd::FINE_PATTERN_DELAY)
							{
								lineSeconds = (60.f/bpm_calc)*((1.f/lpb_calc) + ((pEntry->_parameter&0x0F)/24.f));
								resetLineSec=true;
							}
							else if ( (pEntry->_parameter&0xF0) == PatternCmd::PATTERN_LOOP)
							{
								int value = pEntry->_parameter&0x0F;
								if (value == 0 )
								{
									loopLine = playLine;
								} else if ( loopCount == 0 ) {
									loopCount = value;
									lineJump = loopLine;
								} else {
									if (--loopCount) lineJump = loopLine;
									else loopLine = playLine+1; //This prevents infinite loop in specific cases.
								}
							}
						}
						break;
					case PatternCmd::JUMP_TO_ORDER:
						if ( pEntry->_parameter < song.playLength) {
							if (pEntry->_parameter <= playPos && !allowLoop){
								return round<int>(songLength*1000.0f);
							}
							patternJump=pEntry->_parameter;
							lineJump=0;
						}
						break;
					case PatternCmd::BREAK_TO_LINE:
						if (patternJump ==-1 ) 
						{
							if(playPos+1>=song.playLength) {
								if(allowLoop) {
									patternJump=0;
								}
								else {
									return round<int>(songLength*1000.0f);
								}
							}
							else patternJump=playPos+1;
						}
						else if (patternJump == playPos && pEntry->_parameter <= playLine && !allowLoop) {
							return round<int>(songLength*1000.0f);
						}
						//No need to check limits. That is done by the loop.
						lineJump= pEntry->_parameter;
						break;
					default:break;
					}
				}
			}
			songLength += lineSeconds;
			if ( seektime_ms > -1 && seektime <= songLength) {
				Start(playPos,playLine);
				return round<int>(songLength*1000.0f);
			}
			else if (seqPos >=0 && seqPos <= playPos &&
				(patLine == -1 || patLine <= playLine)) {
				return round<int>(songLength*1000.0f);
			}
			if ( resetLineSec ) { lineSeconds = (60.f/bpm_calc)*((1.f/lpb_calc) + (extratick_calc/24.f)); resetLineSec = false; }
			if ( patternJump!=-1 ) {
				playPos= patternJump;
				playLine=lineJump;
				l=playLine*MULTIPLY;
				break;
			}
			else if ( lineJump!=-1) {
				playLine=lineJump;
				l=playLine*MULTIPLY;
			}
			else {
				playLine++;
				l+=MULTIPLY;
			}
		}
		if ( patternJump==-1 ) {
			playPos++;
		}
	}
	return round<int>(songLength*1000.0f);
}

void Player::thread_function(std::size_t thread_number) {
	if(loggers::trace()) {
		scoped_lock lock(mutex_);
		std::ostringstream s;
		s << "psycle: core: player: scheduler thread #" << thread_number << " started";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}

	// set thread name
	universalis::os::thread_name thread_name;
	{
		std::ostringstream s;
		s << universalis::compiler::typenameof(*this) << " Engine thread #" << thread_number;
		thread_name.set(s.str());
	}

	// install cpu/os exception handler/translator
	universalis::cpu::exceptions::install_handler_in_thread();

	HANDLE hTask = NULL;
	{ // set thread priority and cpu affinity
		using universalis::os::exceptions::operation_not_permitted;
		using universalis::os::sched::thread;
		thread t;

		// set thread priority
		try {
			t.become_realtime();
			// Ask MMCSS to temporarily boost the thread priority
			// to reduce glitches while the low-latency stream plays.
			if(Is_Vista_or_Later()) 
			{
				DWORD taskIndex = 0;
				hTask = Global::pAvSetMmThreadCharacteristics(TEXT("Pro Audio"), &taskIndex);
			}
		} catch(const operation_not_permitted& e) {
			if(loggers::warning()) {
				std::ostringstream s; s << "no permission to set thread scheduling policy and priority to realtime: " << e.what();
				loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}

		// set thread cpu affinity
		try {
			thread::affinity_mask_type const af(t.affinity_mask());
			if(af.active_count()) {
				unsigned int rotated = 0, cpu_index = 0;
				while(!af(cpu_index) || rotated++ != thread_number) cpu_index = (cpu_index + 1) % af.size();
				thread::affinity_mask_type new_af; new_af(cpu_index, true); t.affinity_mask(new_af);
			}
		} catch(const operation_not_permitted& e) {
			if(loggers::warning()) {
				std::ostringstream s; s << "no permission to set thread cpu affinity: " << e.what();
				loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	}

	try {
		process_loop(thread_number);
	} catch(const std::exception & e) {
		if(loggers::exception()) {
			std::ostringstream s;
			s << "caught exception in scheduler thread";
			s << "exception: " << universalis::compiler::typenameof(e) << ": " << e.what();
			loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		throw;
	} catch(...) {
		if(loggers::exception()) {
			std::ostringstream s;
			s << "caught exception in scheduler thread";
			s << "exception: " << universalis::compiler::exceptions::ellipsis_desc();
			loggers::exception()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		throw;
	}
	if (hTask != NULL) { Global::pAvRevertMmThreadCharacteristics(hTask); }

	if(loggers::trace()) {
		scoped_lock lock(mutex_);
		std::ostringstream s;
		s << "psycle: core: player: scheduler thread #" << thread_number << " terminated";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
}

void Player::process_loop(std::size_t thread_number) throw(std::exception) {
	while(true) {
		Machine * node_;
		{ scoped_lock lock(mutex_);
			while( !nodes_queue_.size() && !suspend_requested_ && !stop_requested_ )
			{	
				waiting_++;
				condition_.wait(lock);
				waiting_--;
			}

			if(stop_requested_) break;

			if(suspend_requested_) {
				if(!this_thread_suspended_) {
					this_thread_suspended_ = true;
					++suspended_;
					main_condition_.notify_all();
				}
				continue;
			}
			if(this_thread_suspended_) {
				this_thread_suspended_ = false;
				--suspended_;
			}

			// There are nodes waiting in the queue. We pop the first one.
			node_ = nodes_queue_.front();
			nodes_queue_.pop_front();
		}
		Machine & node(*node_);

		bool const done(node.sched_process(samples_to_process_, measure_cpu_usage_));

		int notify(0);
		bool all_threads_waiting(false);
		{ scoped_lock lock(mutex_);
			node.sched_processed_ = done;
			//If not done, it means it needs to be reprocessed somewhere.
			if (!done) ++graph_size_;

			// check whether all nodes have been processed
			if(++processed_node_count_ == graph_size_) notify = -1; // wake up the main processing loop
			else {
				// check whether successors of the node we processed are now ready.
				// iterate over all the outputs of the node we processed
				output_nodes_.clear(); node.sched_outputs(output_nodes_);

				for(Machine::sched_deps::const_iterator i(output_nodes_.begin()), e(output_nodes_.end()); i != e; ++i) {
					Machine & output_node(*const_cast<Machine*>(*i));
					bool output_node_ready(true);
					// iterate over all the inputs connected to our output
					input_nodes_.clear(); output_node.sched_inputs(input_nodes_);
					for(Machine::sched_deps::const_iterator i(input_nodes_.begin()), e(input_nodes_.end()); i != e; ++i) {
						const Machine & input_node(**i);
						if(&input_node == &node) continue;
						if(!input_node.sched_processed_) {
							output_node_ready = false;
							break;
						}
					}
					if(output_node_ready) {
						// All the dependencies of the node have been processed.
						// We add the node to the processing queue.
						nodes_queue_.push_back(&output_node);
						++notify;
					}
				}
			}
			all_threads_waiting=waiting_+1 == threads_.size();
		}
		switch(notify) {
			case -1: main_condition_.notify_all(); break; // wake up the main processing loop
			case 0: //no successor ready. Maybe another thread is working.
				if (all_threads_waiting && nodes_queue_.empty()) { //This should not happen, but when it does, it causes a deadlock. So at least, prevent the application hang.
					char buf[256];
					sprintf(buf,"Invalid condition found: no successor. Preventing application hang. w:%d,q:%d,t:%d,g:%d:p:%d",
						waiting_,nodes_queue_.size(),threads_.size(),graph_size_,processed_node_count_);
					if(loggers::warning()) loggers::warning()(buf, UNIVERSALIS__COMPILER__LOCATION);
					//::MessageBox(NULL,buf, "bla",MB_OK);
					processed_node_count_ = graph_size_;
					main_condition_.notify_all();// wake up the main processing loop
				}
				break;
			case 1: break; // If there's only one successor ready, we don't notify since it can be processed in the same thread.
			case 2: condition_.notify_one(); break; // notify one thread that we added nodes to the queue
			default: condition_.notify_all(); // notify all threads that we added nodes to the queue
		}
	}
}

void Player::stop_threads() {
	if(loggers::trace()) loggers::trace()("terminating and joining scheduler threads ...", UNIVERSALIS__COMPILER__LOCATION);
	if(threads_.empty()) {
		if(loggers::trace()) loggers::trace()("scheduler threads were not running", UNIVERSALIS__COMPILER__LOCATION);
		return;
	}
	{ scoped_lock lock(mutex_);
		stop_requested_ = true;
	}
	condition_.notify_all();
	for(threads_type::const_iterator i(threads_.begin()), e(threads_.end()); i != e; ++i) {
		(**i).join();
		delete *i;
	}
	if(loggers::trace()) loggers::trace()("scheduler threads joined", UNIVERSALIS__COMPILER__LOCATION);
	threads_.clear();
	clear_plan();
}
		bool Player::trackPlaying(int track)
		{
			Song& song = Global::song();
			for(int track=0; track < song.SONGTRACKS; track++) {
				if(playTrack[track] && prevMachines[track] < MAX_MACHINES 
						&& song._pMachine[prevMachines[track]]) {
					Machine& mac = *song._pMachine[prevMachines[track]];
					playTrack[track] = mac.playsTrack(track);
				}
				else {
					playTrack[track] = false;
				}
			}
			return playTrack[track];
		}

		CExclusiveLock Player::GetLockObject() {
			return CExclusiveLock(&Global::song().semaphore, 2, FALSE);
		}
		float * Player::Work(void* context, int numSamples)
		{
			CSingleLock crit(&Global::song().semaphore, FALSE);
			Player* pThis = (Player*)context;
			//Avoid possible deadlocks
			if(crit.Lock(100)) {
				try {
					pThis->Work(numSamples);
				}
				catch(const std::exception& e) {
						loggers::exception()(e.what());
#ifndef NDEBUG
					throw e;
#endif
				}
				catch(...) {
#ifndef NDEBUG
					throw;
#endif
				}
			}
			else {
				dsp::Clear(pThis->_pBuffer,numSamples*2);
			}
			return pThis->_pBuffer;
		}
		float * Player::Work(int numSamples)
		{
			int amount;
			Song& song = Global::song();
			Master::_pMasterSamples = _pBuffer;
			cpu_time_clock::time_point const t0(cpu_time_clock::now());
			sampleOffset = 0;
			do
			{
				// Song play
				if((_samplesRemaining <=0))
				{
					//this double "if" is meant to prevent new line to play if song looping is disabled.
					if(_playing) 
					{
						// Advance position in the sequencer
						AdvancePosition();
					}
					if (_playing)
					{
						ExecuteLine();
					}
					else
					{
						NotifyNewLine();
						NotifyPostNewLine();
					}
					_samplesRemaining = SamplesPerRow();
				}
				if(numSamples > STREAM_SIZE) amount = STREAM_SIZE; else amount = numSamples;
				// Tick handler function
				if(amount >= _samplesRemaining) amount = _samplesRemaining;
				// Processing plant
				if(amount > 0)
				{
					// Reset all machines
					for(int c=0; c<MAX_MACHINES; c++)
					{
						//Note: This should be scheduled if possible too. Also note that it increments
						// the routing_accumulator, which is is divided by numthreads in the infodlg.
						if(song._pMachine[c]) song._pMachine[c]->PreWork(amount, true, measure_cpu_usage_);
					}

					//\todo: Sampler::DoPreviews( amount );
					song.DoPreviews( amount );

					CVSTHost::vstTimeInfo.samplePos = sampleCount;

#if !defined WINAMP_PLUGIN
					// Inject Midi input data
					PsycleGlobal::midi().InjectMIDI( amount );
#endif //!defined WINAMP_PLUGIN
					if(threads_.empty()){ // single-threaded, recursive processing
						song._pMachine[MASTER_INDEX]->recursive_process(amount, measure_cpu_usage_);
					} else { // multi-threaded scheduling
						// we push all the terminal nodes to the processing queue
						{ scoped_lock lock(mutex_);
							compute_plan(); // it's overkill, but we haven't yet implemented signals that notifies when connections are changed or nodes added/removed.
							processed_node_count_ = 0;
							samples_to_process_ = amount;
						}
						condition_.notify_all(); // notify all threads that we added nodes to the queue
						// wait until all nodes have been processed
						{ scoped_lock lock(mutex_);
							while(processed_node_count_ != graph_size_) main_condition_.wait(lock);
						}
					}
					sampleCount += amount;

					if(_recording)
					{
						float* pL(((Master*)song._pMachine[MASTER_INDEX])->getLeft());
						float* pR(((Master*)song._pMachine[MASTER_INDEX])->getRight());
						if(_dodither)
						{
							dither.Process(pL, amount);
							dither.Process(pR, amount);
						}
						int i;
						if ( _clipboardrecording)
						{
							switch(m_recording_chans)
							{
							case mono_mix: // mono mix
								for(i=0; i<amount; i++)
								{
									if (!ClipboardWriteMono(((*pL++)+(*pR++))*0.5f)) StopRecording(false);
								}
								break;
							case mono_left: // mono L
								for(i=0; i<amount; i++)
								{
									if (!ClipboardWriteMono(*pL++)) StopRecording(false);
								}
								break;
							case mono_right: // mono R
								for(i=0; i<amount; i++)
								{
									if (!ClipboardWriteMono(*pR++)) StopRecording(false);
								}
								break;
							default: // stereo
								for(i=0; i<amount; i++)
								{
									if (!ClipboardWriteStereo(*pL++,*pR++)) StopRecording(false);
								}
								break;
							}
						}
						else switch(m_recording_chans)
						{
						case mono_mix: // mono mix
							for(i=0; i<amount; i++)
							{
								//argh! dithering both channels and then mixing.. we'll have to sum the arrays before-hand, and then dither.
								if(_outputWaveFile.WriteMonoSample(((*pL++)+(*pR++))*0.5f) != DDC_SUCCESS) StopRecording(false);
							}
							break;
						case mono_left: // mono L
							for(i=0; i<amount; i++)
							{
								if(_outputWaveFile.WriteMonoSample((*pL++)) != DDC_SUCCESS) StopRecording(false);
							}
							break;
						case mono_right: // mono R
							for(i=0; i<amount; i++)
							{
								if(_outputWaveFile.WriteMonoSample((*pR++)) != DDC_SUCCESS) StopRecording(false);
							}
							break;
						default: // stereo
							for(i=0; i<amount; i++)
							{
								if(_outputWaveFile.WriteStereoSample((*pL++),(*pR++)) != DDC_SUCCESS) StopRecording(false);
							}
							break;
						}
					}
					Master::_pMasterSamples += amount * 2;
					numSamples -= amount;
				}
				 _samplesRemaining -= amount;
				 sampleOffset += amount;
				 CVSTHost::vstTimeInfo.flags &= ~kVstTransportChanged;
			} while(numSamples>0);

			cpu_time_clock::time_point const t1(cpu_time_clock::now());
			song.accumulate_processing_time(t1 - t0);
			return _pBuffer;
		}
		bool Player::ClipboardWriteMono(float sample)
		{
			// right now the implementation does not support these two being different
/*			if (Global::configuration()._pOutputDriver->GetSampleValidBits() !=
				Global::configuration()._pOutputDriver->GetSampleBits()) {
					return false;
			}
*/
			//total length.
			int *length = reinterpret_cast<int*>((*pClipboardmem)[0]);
			//position in this buffer.
			int pos = *length%1000000;

			int d(0);
			if(sample > 32767.0f) sample = 32767.0f;
			else if(sample < -32768.0f) sample = -32768.0f;
			switch( m_recording_depth)
			{
			case 8:
				d = int(sample/256.0f);
				d += 128;
				(*pClipboardmem)[clipbufferindex][pos++]=static_cast<char>(d&0xFF);
				*length+=1;
				break;
			case 16:
				d = static_cast<int>(sample);
				(*pClipboardmem)[clipbufferindex][pos++]=static_cast<char>(d&0xFF);
				(*pClipboardmem)[clipbufferindex][pos++]=*(reinterpret_cast<char*>(&d)+1);
				*length+=2;
				break;
			case 24:
				d = int(sample * 256.0f);
				if ( pos+3 < 1000000 )
				{
					(*pClipboardmem)[clipbufferindex][pos++]=static_cast<char>(d&0xFF);
					(*pClipboardmem)[clipbufferindex][pos++]=*(reinterpret_cast<char*>(&d)+1);
					(*pClipboardmem)[clipbufferindex][pos++]=*(reinterpret_cast<char*>(&d)+2);
					*length+=3;
				}
				else { pos+=3; } //Delay operation after buffer creation.
				break;
			case 32:
				d = int(sample * 65536.0f);
				(*pClipboardmem)[clipbufferindex][pos]=static_cast<char>(d&0xFF);
				(*pClipboardmem)[clipbufferindex][pos+1]=*(reinterpret_cast<char*>(&d)+1);
				(*pClipboardmem)[clipbufferindex][pos+2]=*(reinterpret_cast<char*>(&d)+2);
				(*pClipboardmem)[clipbufferindex][pos+3]=*(reinterpret_cast<char*>(&d)+3);
				*length+=4;
				break;
			default:
				break;
			}

			//if reached buffer end.
			if ( pos >= 1000000)
			{
				clipbufferindex++;
				char *newbuf = new char[1000000];
				if (!newbuf) return false;
				pClipboardmem->push_back(newbuf);
				// bitdepth == 24 is the only "odd" value, since it uses 3 chars each, nondivisible by 1000000
				if ( m_recording_depth == 24)
				{
					clipbufferindex--;
					pos-=3;
					(*pClipboardmem)[clipbufferindex][pos]=static_cast<char>(d&0xFF);
					if ( ++pos = 1000000) { pos = 0; clipbufferindex++; }
					(*pClipboardmem)[clipbufferindex][pos+1]=*(reinterpret_cast<char*>(&d)+1);
					if ( ++pos = 1000000) { pos = 0; clipbufferindex++; }
					(*pClipboardmem)[clipbufferindex][pos+2]=*(reinterpret_cast<char*>(&d)+2);
					if ( ++pos = 1000000) { pos = 0;  clipbufferindex++; }
					*length+=3;
				}
			}
			return true;
		}
		bool Player::ClipboardWriteStereo(float left, float right)
		{
			if (!ClipboardWriteMono(left)) return false;
			return ClipboardWriteMono(right);
		}

		void Player::StartRecording(std::string psFilename, int bitdepth, int samplerate, channel_mode channelmode, bool isFloat, bool dodither, int ditherpdf, int noiseshape, std::vector<char*> *clipboardmem)
		{
			if(!_recording)
			{
				Stop();

				if(samplerate > 0) SampleRate(samplerate);

				if(bitdepth>0)	m_recording_depth = bitdepth;
				else {
					m_recording_depth = Global::configuration()._pOutputDriver->GetSampleValidBits();
					if(Global::configuration()._pOutputDriver->GetSampleBits() == 32) isFloat = true;
				}

				_dodither=dodither;
				if(dodither)
				{
					dither.SetBitDepth(m_recording_depth);
					dither.SetPdf((helpers::dsp::Dither::Pdf::type)ditherpdf);
					dither.SetNoiseShaping((helpers::dsp::Dither::NoiseShape::type)noiseshape);
				}

				int channels;
				if(channelmode != no_mode && channelmode != stereo) { channels = 1; }
				else { channels = 2; }
				m_recording_chans = channelmode;

				if (!psFilename.empty())
				{
					if(_outputWaveFile.OpenForWrite(psFilename.c_str(), m_SampleRate, m_recording_depth, channels, isFloat) == DDC_SUCCESS)
						_recording = true;
					else
					{
						StopRecording(false);
					}
				}
				else
				{
					char *newbuf = new char[1000000];
					if ( newbuf)
					{
						pClipboardmem = clipboardmem;
						pClipboardmem->push_back(newbuf);
						_clipboardrecording = true;
						clipbufferindex = 1;
						_recording = true;
					}
					else {
						StopRecording(false);
					}
				}
			}
		}

		void Player::StopRecording(bool bOk)
		{
			if(_recording || !bOk)
			{
				SampleRate(Global::configuration()._pOutputDriver->GetSamplesPerSec());
				if (!_clipboardrecording)
					_outputWaveFile.Close();
				_recording = false;
				_clipboardrecording =false;
				if(!bOk)
				{
					MessageBox(0, "Wav recording failed.", "ERROR", MB_OK);
				}
			}
		}
	}
}
