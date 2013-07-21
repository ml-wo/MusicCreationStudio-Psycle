///\file
///\brief implementation file for psycle::host::CSkinDlg.

#include <psycle/host/detail/project.private.hpp>
#include "luaplugin.hpp"
#include "lua.hpp"
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <psycle/helpers/math.hpp>
#include "PsycleConfig.hpp"
#include "Configuration.hpp"
#include "PsycleGlobal.hpp"
#include <algorithm>
#include <psycle/host/Song.hpp>



namespace psycle { namespace host {
 	
		//////////////////////////////////////////////////////////////////////////
		// Lua

		LuaPlugin::LuaPlugin(lua_State* state, int index, bool full)
			: proxy_(this, state)
		{		
			_macIndex = index;
			_type = MACH_LUA;
			_mode = MACHMODE_FX;
			std::sprintf(_editName, "native plugin");		
			InitializeSamplesVector();
			try {
			  proxy_.call_run(samplesV);
			  if (full) {
			    proxy_.call_init();
			  }
			} catch(std::exception &e) {} //do nothing.
		}

		LuaPlugin::~LuaPlugin() {
			Free();
		}

		void LuaPlugin::Free() {
		  try {
			proxy_.free_state();
		} catch(std::exception &e) {} //do nothing.
		}

		void LuaPlugin::ReloadScript()
		{
			proxy_.reload();
		}

		int LuaPlugin::GenerateAudioInTicks(int /*startSample*/, int numSamples) throw(psycle::host::exception)
		{
			if (crashed()) {
				return numSamples;
			}
			if(_mode == MACHMODE_GENERATOR) {
				Standby(false);
			}
			if (!_mute) 
			{
				if ((_mode == MACHMODE_GENERATOR) || (!Bypass() && !Standby()))
				{										
					try {
						proxy_.call_work(numSamples);					
					}catch(std::exception &e) {} //do nothing.
					UpdateVuAndStanbyFlag(numSamples);
				}
			}
			else Standby(true);
			return numSamples;
		}

		bool LuaPlugin::LoadSpecificChunk(RiffFile* pFile, int version)
		{
			//TODO
			UINT size;
			pFile->Read(&size, sizeof size); // size of this part params to load
			pFile->Skip(size);
			return true;
		}
		void LuaPlugin::SaveSpecificChunk(RiffFile * pFile)
		{
			//TODO
		}

		bool LuaPlugin::SetParameter(int numparam, int value) {
			if (crashed()) {
				return false;
			}
			try {
			  int minval; int maxval;
			  proxy_.get_parameter_range(numparam, minval, maxval);
			  int quantization = (maxval-minval);
			  proxy_.call_parameter(numparam,float(value)/float(quantization));
			  return true;
			} catch(std::exception &e) {} //do nothing.
			return false;
		}

		void LuaPlugin::GetParamName(int numparam, char * parval) {
			if (crashed()) {
				std::strcpy(parval, "");
			}
			try {
			  if( numparam < GetNumParams() ) {
				std::string name = proxy_.get_parameter_name(numparam);
				std::strcpy(parval, name.c_str());
			  } else std::strcpy(parval, "Out of Range");
			}catch(std::exception &e) { std::strcpy(parval, ""); }
		}

		int LuaPlugin::GetParamValue(int numparam){
			if (crashed()) {
				return 0;
			}
			if(numparam < GetNumParams()) {
			  int minval; int maxval;			  
			  try {
				proxy_.get_parameter_range(numparam, minval, maxval);
			    int quantization = (maxval-minval);
				return proxy_.get_parameter_value(numparam)*quantization;
  			  } catch(std::exception &e) {} //do nothing.
			} else {
				// out of range
			}
			return 0;
		}

		bool LuaPlugin::DescribeValue(int numparam, char * psTxt){
			if (crashed()) {
				return false;
			}

				if(numparam >= 0 && numparam < GetNumParams()) {
					try {
					  std::string par_display = proxy_.get_parameter_display(numparam);
					  std::string par_label = proxy_.get_parameter_label(numparam);
					  std::sprintf(psTxt, "%s(%s)", par_display.c_str(), par_label.c_str());
					  return true;
					} catch(std::exception &e) {
                      std::string par_display("Out of range");
  				      std::sprintf(psTxt, "%s", par_display);
					  return true;
					} //do nothing.
				}
			return false;
		}

		void LuaPlugin::GetParamValue(int numparam, char * parval) {
			if (crashed()) {
				return;
			}
			if(numparam < GetNumParams()) {
				try {
					if(!DescribeValue(numparam, parval)) {
						std::sprintf(parval,"%.0f",GetParamValue(numparam) * 1); // 1 = Plugin::quantizationVal())
					}					
				}
				catch(const std::exception &e) {
#ifndef NDEBUG 
					throw e;
					return;
#else
					e;
					return;
#endif
				}
			}
			else std::strcpy(parval,"Out of Range");
		}

		void LuaPlugin::Tick(int track, PatternEntry * pData){
			if (crashed()) {
				return;
			}
			try {
		     proxy_.call_seqtick(track, pData->_note, pData->_inst, pData->_cmd, pData->_parameter);
			} catch(const std::exception &e) {}
		}

		void LuaPlugin::Stop(){
			if (crashed()) {
				return;
			}
			try { proxy_.call_stop(); } catch(const std::exception &e) {}
		}
	  
		
		

}   // namespace
}   // namespace