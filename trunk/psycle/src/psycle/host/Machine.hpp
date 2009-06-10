///\file
///\brief interface file for psycle::host::Machine
#pragma once
#include "Global.hpp"
#include "SongStructs.hpp"
#include "Dsp.hpp"
#include "Helpers.hpp"
#include "FileIO.hpp"
#include "exceptions.hpp"

namespace psycle { namespace host {

		class Machine; // forward declaration
		class RiffFile; // forward declaration


		/// Class storing the parameter description of Internal Machines.
		class CIntMachParam			
		{
		public:
			/// Short name
			const char * name;		
			/// >= 0
			int minValue;
			/// <= 65535
			int maxValue;
		};

		/// Base class for "Machines", the audio producing elements.
		class Machine
		{
			///\name crash handling
			///\{
				public:
					/// This function should be called when an exception was thrown from the machine.
					/// This will mark the machine as crashed, i.e. crashed() will return true,
					/// and it will be disabled.
					///\param e the exception that occured, converted to a std::exception if needed.
					void crashed(std::exception const & e) throw();

				public:
					/// Tells wether this machine has crashed.
					bool const inline & crashed() const throw() { return crashed_; }
				private:
					bool                crashed_;
			///\}

#if 0 // v1.9
			///\name cpu cost measurement ... for the time spent in the machine's processing function
			///\{
				public:
					void             inline work_cpu_cost(cpu::cycles_type const & value)       throw() { work_cpu_cost_ = value; }
					cpu::cycles_type inline work_cpu_cost(                              ) const throw() { return work_cpu_cost_; }
				private:
					cpu::cycles_type        work_cpu_cost_;
			///\}
			///\name cpu cost measurement ... for the time spent routing audio
			///\{
				public:
					void             inline wire_cpu_cost(cpu::cycles_type const & value)       throw() { wire_cpu_cost_ = value; }
					cpu::cycles_type inline wire_cpu_cost(                              ) const throw() { return wire_cpu_cost_; }
				private:
					cpu::cycles_type        wire_cpu_cost_;
			///\}
#else
			public:///\todo private
				cpu::cycles_type _cpuCost;
				cpu::cycles_type _wireCost;
#endif

#if 0 // v1.9
			///\name each machine has a type attribute so that we can make yummy switch statements
			///\{
				public:
					///\see enum MachineClass which defined somewhere outside
					typedef MachineClass class_type;
					Machine::class_type inline subclass() const throw() { return _subclass; }
				public:///\todo private:
					class_type _subclass;
			///\}

			///\name each machine has a mode attribute so that we can make yummy switch statements
			///\{
				public:
					///\see enum MachineMode which is defined somewhere outside
					typedef MachineMode mode_type;
					mode_type inline mode() const throw() { return _mode; }
				public:///\todo private:
					mode_type _mode;
			///\}

			///\name machine's numeric identifier used in the patterns and gui display
			///\{
				public:
					/// legacy
					///\todo should be unsigned but some functions return negative values to signal errors instead of throwing an exception
					//PSYCLE__STRONG_TYPEDEF(int, id_type);
					typedef int id_type;
					id_type id() const throw() { return _macIndex; }
				public:///\todo private:
					/// it's actually used as an array index, but that shouldn't be part of the interface
					id_type _macIndex;
			///\}
#else
			public:///\todo private
				int _macIndex;
				int id() const  { return _macIndex; }
				MachineType _type;
				MachineMode _mode;
#endif

			///\name the life cycle of a mahine
			///\{
				public:
					virtual void Init();
					virtual void PreWork(int numSamples,bool clear=true);
					virtual void Work(int numSamples);
					virtual void WorkNoMix(int numSamples);
					virtual void Tick() {}
					virtual void Tick(int track, PatternEvent * pData) {}
					virtual void Stop() {}
			///\}

			///\name (de)serialization
			///\{
				public:
					virtual bool Load(RiffFile * pFile);
					static Machine * LoadFileChunk(RiffFile* pFile, int index, int version,bool fullopen=true);
					virtual bool LoadSpecificChunk(RiffFile* pFile, int version);
					virtual void SaveDllNameAndIndex(RiffFile * pFile,int index);
					virtual void SaveFileChunk(RiffFile * pFile);
					virtual void SaveSpecificChunk(RiffFile * pFile);
					virtual void PostLoad(){};
			///\}

			///\name connections ... wires
			///\{
				public:
					// Set or replace output wire
					virtual void InsertOutputWireIndex(Song* pSong,int wireIndex,int dstmac);
					// Set or replace input wire
					virtual void InsertInputWireIndex(Song* pSong,int wireIndex,int srcmac,float wiremultiplier,float initialvol=1.0f);
					virtual void ExchangeInputWires(int first,int second);
					virtual void ExchangeOutputWires(int first,int second);
					virtual void NotifyNewSendtoMixer(Song* pSong,int callerMac,int senderMac);
					virtual void ClearMixerSendFlag(Song* pSong);
					virtual void DeleteOutputWireIndex(Song* pSong,int wireIndex);
					virtual void DeleteInputWireIndex(Song* pSong,int wireIndex);
					virtual void DeleteWires(Song *pSong);
					virtual int FindInputWire(int macIndex);
					virtual int FindOutputWire(int macIndex);
					virtual int GetFreeInputWire(int slottype=0);
					virtual int GetFreeOutputWire(int slottype=0);
					virtual int GetInputSlotTypes() { return 1; }
					virtual int GetOutputSlotTypes() { return 1; }
					virtual float GetAudioRange() { return 1.0f; }

			///\}

			///\name amplification of the signal in connections/wires
			///\{
				public:
					virtual void GetWireVolume(int wireIndex, float &value) { value = GetWireVolume(wireIndex); }
					virtual float GetWireVolume(int wireIndex) { return _inputConVol[wireIndex] * _wireMultiplier[wireIndex]; }
					virtual void SetWireVolume(int wireIndex,float value) { _inputConVol[wireIndex] = value / _wireMultiplier[wireIndex]; }
					virtual bool GetDestWireVolume(Song* pSong,int srcIndex, int WireIndex,float &value);
					virtual bool SetDestWireVolume(Song* pSong,int srcIndex, int WireIndex,float value);
			///\}

			///\name general information
			///\{
				public:
					///\todo: update this to std::string.
					virtual void SetEditName(std::string const & newname) { std::strncpy(_editName,newname.c_str(),32); }
					const std::string GetEditName() { return std::string(_editName); }
				public:///\todo private:
					///\todo this was a std::string in v1.9
					char _editName[32];

				public:
					virtual char * GetName() = 0;
					virtual const char * const GetDllName() const throw() { return ""; }
					virtual int GetPluginCategory() { return 0; }
					virtual bool IsShellMaster() { return false; }
					virtual int GetShellIdx() { return 0; }
			///\}

			///\name parameters
			///\{
				public:
					virtual int GetNumCols() { return _nCols; }
					virtual int GetNumParams() { return _numPars; }
					virtual void GetParamName(int numparam, char * name) { name[0]='\0'; }
					virtual void GetParamRange(int numparam, int &minval, int &maxval) {minval=0; maxval=0; }
					virtual void GetParamValue(int numparam, char * parval) { parval[0]='\0'; }
					virtual int GetParamValue(int numparam) { return 0; }
					virtual bool SetParameter(int numparam, int value) { return false; }
				public:///\todo private:
					int _numPars;
					int _nCols;
			///\}

			///\name gui stuff
			///\{
				public:
					virtual int  GetPosX() { return _x; }
					virtual void SetPosX(int x) {_x = x;}
					virtual int  GetPosY() { return _y; }
					virtual void SetPosY(int y) {_y = y;}
				public:///\todo private:
					int _x;
					int _y;
			///\}

			///\name states
			///\{
				public:
					virtual bool Bypass() { return _bypass; }
					virtual void Bypass(bool e) { _bypass = e; }
				public:///\todo private:
					bool _bypass;

				public:
					virtual bool Standby() { return _standby; }
					virtual void Standby(bool e) { _standby = e; }
				public:///\todo private:
					bool _standby;

				public:
					bool Mute() { return _mute; }
					void Mute(bool e) { _mute = e; }
				public:///\todo private:
					bool _mute;
			///\}

			///\name panning
			///\{
				public:
					///\todo int GetPan() { return _panning; }
					///\todo 3 dimensional?
					virtual void SetPan(int newpan);
					int Pan() const { return _panning; }
				public:///\todo private:
					/// numerical value of panning
					int _panning;							
					float lVol() const { return _lVol; }
					float rVol() const { return _rVol; }
					/// left chan volume
					float _lVol;							
					/// right chan volume
					float _rVol;							
			///\}
				
		public:
			Machine();
			Machine(Machine* mac);
			Machine(MachineType msubclass, MachineMode mode, int id);
			virtual ~Machine() throw();
			virtual void SetSampleRate(int sr)
			{
#if PSYCLE__CONFIGURATION__RMS_VUS
				rms.count=0;
				rms.AccumLeft=0.;
				rms.AccumRight=0.;
				rms.previousLeft=0.;
				rms.previousRight=0.;
#endif
			}

			bool IsGenerator() const {
				return (_mode == MACHMODE_GENERATOR);
			}

		protected:
			void UpdateVuAndStanbyFlag(int numSamples);

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//\todo below are unencapsulated data members

		public://\todo private:

			///\name gui stuff
			///\{
				/// The topleft point of a square where the wire triangle is centered when drawn. (Used to detect when to open the wire dialog)
				///\todo hardcoded limits and wastes with MAX_CONNECTIONS
				CPoint _connectionPoint[MAX_CONNECTIONS];
			///\}

			///\name signal measurements (and also gui stuff)
			///\{
				/// output peak level for DSP
				float _volumeCounter;					
				/// output peak level for display
#if PSYCLE__CONFIGURATION__RMS_VUS
				helpers::dsp::RMSData rms;
#endif
				int _volumeDisplay;	
				/// output peak level for display
				int _volumeMaxDisplay;
				/// output peak level for display
				int _volumeMaxCounterLife;
				///\todo doc
				int _scopePrevNumSamples;
				///\todo doc
				int	_scopeBufferIndex;
				///\todo doc
				float *_pScopeBufferL;
				///\todo doc
				float *_pScopeBufferR;
			///\}

			///\name misc
			///\{
				/// machine has started its work call, and is waiting for inputs to finish generating sound.
				bool _waitingForSound;
				/// machine has finished working, and samples are ready in the buffers until next work call.
				bool _worked;
				/// this machine is used by a send/return mixer. (Some things cannot be done on these machines)
				bool _isMixerSend;
				/// left data
				float *_pSamplesL;
				/// right data
				float *_pSamplesR;						
			///\}

			///\name various player-related states
			///\todo hardcoded limits and wastes with MAX_TRACKS
			///\{
				///\todo doc
				PatternEvent TriggerDelay[MAX_TRACKS];
				///\todo doc
				int TriggerDelayCounter[MAX_TRACKS];
				///\todo doc
				int RetriggerRate[MAX_TRACKS];
				///\todo doc
				int ArpeggioCount[MAX_TRACKS];
				///\todo doc
				bool TWSActive;
				///\todo doc
				int TWSInst[MAX_TWS];
				///\todo doc
				int TWSSamples;
				///\todo doc
				float TWSDelta[MAX_TWS];
				///\todo doc
				float TWSCurrent[MAX_TWS];
				///\todo doc
				float TWSDestination[MAX_TWS];
			///\}

			///\name input ports
			///\{
				/// number of Incoming connections
				int _numInputs;							
				/// Incoming connections Machine number
				int _inputMachines[MAX_CONNECTIONS];	
				/// Incoming connections activated
				bool _inputCon[MAX_CONNECTIONS];		
				/// Incoming connections Machine vol
				float _inputConVol[MAX_CONNECTIONS];	
				/// Value to multiply _inputConVol[] with to have a 0.0...1.0 range
				// The reason of the _wireMultiplier variable is because VSTs output wave data
				// in the range -1.0 to +1.0, while natives and internals output at -32768.0 to +32768.0
				// Initially (when the format was made), Psycle did convert this in the "Work" function,
				// but since it already needs to multiply the output by inputConVol, I decided to remove
				// that extra conversion and use directly the volume to do so.
				float _wireMultiplier[MAX_CONNECTIONS];	
			///\}

			///\name output ports
			///\{
				/// number of Outgoing connections
				int _numOutputs;						
				/// Outgoing connections Machine number
				int _outputMachines[MAX_CONNECTIONS];	
				/// Outgoing connections activated
				bool _connection[MAX_CONNECTIONS];      
			///\}
		};

		/// master machine.
		class Master : public Machine
		{
		public:
			Master();
			Master(int index);
			virtual void Init(void);
			virtual void Work(int numSamples);
			virtual float GetAudioRange(){ return 32768.0f; }
			virtual char* GetName(void) { return _psName; }
			virtual bool Load(RiffFile * pFile);
			virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
			virtual void SaveSpecificChunk(RiffFile * pFile);

			int _outDry;
			bool vuupdated;
			bool _clip;
			bool decreaseOnClip;
			int peaktime;
			float currentpeak;
			float _lMax;
			float _rMax;
			static float* _pMasterSamples;
			/// this is for the VstHost
			double sampleCount;
		protected:
			static char* _psName;
		};
}}