/***************************************************************************
  *   Copyright (C) 2006 by Stefan   *
  *   natti@linux   *
  *                                                                         *
  *   This program is free software; you can redistribute it and/or modify  *
  *   it under the terms of the GNU General Public License as published by  *
  *   the Free Software Foundation; either version 2 of the License, or     *
  *   (at your option) any later version.                                   *
  *                                                                         *
  *   This program is distributed in the hope that it will be useful,       *
  *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
  *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
  *   GNU General Public License for more details.                          *
  *                                                                         *
  *   You should have received a copy of the GNU General Public License     *
  *   along with this program; if not, write to the                         *
  *   Free Software Foundation, Inc.,                                       *
  *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
  ***************************************************************************/

#include "configuration.h"
#include "song.h"
#include "global.h"
#ifdef __unix__
#include "alsaout.h"
#include "jackout.h"
#include "gstreamerout.h"
#include "esoundout.h"
//#include "microsoft_direct_sound_out.h"
//#include "netaudioout.h"
#include "wavefileout.h"
#endif
//#include "netaudioout.h"
#include "defaultbitmaps.h"
#include <ngrs/napp.h>
#include <ngrs/nconfig.h>
#include <ngrs/nfile.h>
#include <ngrs/nkeyevent.h>
#include <sys/stat.h>
#include <cstdlib>
#include <stdexcept>
#include <sstream>

#if defined XPSYCLE__CONFIGURATION
#include <xpsycle/install_paths.h>
#endif

namespace psycle {
		namespace host {

Configuration::Configuration()
{
  #if !defined XPSYCLE__CONFIGURATION
  std::cout << "xpsycle: warning: built without configuration" << std::endl;
  #endif
  setSkinDefaults();
  defaultPatLines = 64;
  loadConfig();
  bitmaps_ = new DefaultBitmaps(this);
  _RecordTweaks = false;
  _RecordUnarmed = true;
}


Configuration::~Configuration()
{
  delete bitmaps_;
}

void Configuration::setSkinDefaults( )
{
  _centerCursor = false;
  enableSound = 0;
  device_name = "default";

  _linenumbers       = true;
  _linenumbersHex    = false;
  _linenumbersCursor = true;
  pv_timesig = 4;

  pattern_font_x = 9;
  pattern_font_y = 11;

  vu1.setHCOLORREF(0x0080FF80);
  vu2.setHCOLORREF(0x00403731);
  vu3.setHCOLORREF(0x00262bd7);
  
  machineGUITopColor.setHCOLORREF(0x00D2C2BD);
  machineGUIFontTopColor.setHCOLORREF(0x00000000);
  machineGUIBottomColor.setHCOLORREF(0x009C796D);
  machineGUIFontBottomColor.setHCOLORREF(0x00FFFFFF);

  machineGUIHTopColor.setHCOLORREF(0x00BC94A9);//highlighted param colours
  machineGUIHFontTopColor.setHCOLORREF(0x00000000);
  machineGUIHBottomColor.setHCOLORREF(0x008B5A72);
  machineGUIHFontBottomColor.setHCOLORREF(0x0044EEFF);

  machineGUITitleColor.setHCOLORREF(0x00000000);
  machineGUITitleFontColor.setHCOLORREF(0x00FFFFFF);

	{	
		AudioDriver* driver = 0;
		driver = new AudioDriver;
		_pSilentDriver = driver;
		driverMap_[ driver->info().name() ] = driver;


        #ifdef __unix__
		driver = new WaveFileOut();
		driverMap_[ driver->info().name() ] = driver;

		#if !defined XPSYCLE__NO_ALSA
			driver = new AlsaOut;
			std::cout << "registered:" <<  driver->info().name() << std::endl;
			driverMap_[ driver->info().name() ] = driver;
		#endif
		#if !defined XPSYCLE__NO_JACK
			driver = new JackOut;
			std::cout << "registered:" <<  driver->info().name() << std::endl;
			driverMap_[ driver->info().name() ] = driver;
		#endif
		#if !defined XPSYCLE__NO_GSTREAMER
			driver = new GStreamerOut;
			std::cout << "registered:" <<  driver->info().name() << std::endl;
			driverMap_[ driver->info().name() ] = driver;
		#endif
		#if !defined XPSYCLE__NO_ESOUND
			driver = new ESoundOut;
			std::cout << "registered:" <<  driver->info().name() << std::endl;
			driverMap_[ driver->info().name() ] = driver;
		#endif		
		
		#endif
/*		#if !defined XPSYCLE__NO_NETAUDIO
			driver = new NetAudioOut;
			std::cout << "registered:" <<  driver->info().name() << std::endl;
			driverMap_[ driver->info().name() ] = driver;
		#endif*/
		/*#if !defined XPSYCLE__NO_MICROSOFT_DIRECT_SOUND
			driver = new MicrosoftDirectSoundOut;
			std::cout << "registered:" <<  driver->info().name() << std::endl;
			driverMap_[ driver->info().name() ] = driver;
		#endif*/

	}

	setDriverByName("silent");
	enableSound = false;

  #if defined XPSYCLE__CONFIGURATION
	#include <xpsycle/install_paths.h>
	hlpPath = XPSYCLE__INSTALL_PATHS__DOC "/";
	iconPath = XPSYCLE__INSTALL_PATHS__PIXMAPS "/";
	pluginPath = XPSYCLE__INSTALL_PATHS__PLUGINS "/";
	prsPath = XPSYCLE__INSTALL_PATHS__PRESETS "/";
  #else
	// we don't have any information about the installation paths,
	// so, we can only assume everything is at a fixed place, like under the user home dir
	hlpPath = NFile::replaceTilde("~/xpsycle/doc/");
	iconPath = NFile::replaceTilde("~/xpsycle/pixmaps/");
	pluginPath = NFile::replaceTilde("~/xpsycle/plugins/");
	prsPath =  NFile::replaceTilde("~/xpsycle/prs/");
  #endif

  #if !defined NDEBUG
  std::cout
    << "xpsycle: configuration: initial defaults:\n"
    << "xpsycle: configuration: pixmap dir: " << iconPath << "\n"
    << "xpsycle: configuration: plugin dir: " << pluginPath << "\n"
    << "xpsycle: configuration: preset dir: " << prsPath << "\n"
    << "xpsycle: configuration: doc    dir: " << hlpPath << "\n";
  #endif

}

void Configuration::setDriverByName( const std::string & driverName )
{
	std::map< std::string, AudioDriver*>::iterator it = driverMap_.begin();

	if ( ( it = driverMap_.find( driverName ) ) != driverMap_.end() ) {
		// driver found
		_pOutputDriver = it->second;
	}
	else {
		// driver not found,  set silent default driver
		_pOutputDriver = _pSilentDriver;
	}
	std::cout << "audio driver set as: " << _pOutputDriver->info().name() << std::endl;		

}


void Configuration::loadConfig()
{
  NApp::config()->tagParse.connect(this,&Configuration::onConfigTagParse);

  // system-wide

  // environment
  // this is most useful for developpers.
  // you can test xpsycle directly from within the build dir,
  // pointing various paths to the source or build dir.
  char const * const xpsycleenv(std::getenv("XPSYCLE__CONFIGURATION"));
  std::string path= (xpsycleenv?xpsycleenv:"");
  if(path.length()!=0) {
    try {
      loadConfig(path);
    }
    catch(std::exception const & e)
    {
      std::cerr << "xpsycle: configuration: error: " << e.what() << std::endl;
    }
  } else {
	path=NFile::replaceTilde("~/.xpsycle.xml");
	if (path.length()!=0) {
  		try {
		      loadConfig(NFile::replaceTilde("~/.xpsycle.xml"));
		}
  		catch(std::exception const & e) {
		    	std::cerr << "xpsycle: configuration: error: " << e.what() << std::endl;
	  	}
	} else {

/*  	path=XPSYCLE__INSTALL_PATHS__CONFIGURATION "/xpsycle.xml";
	if (path.length()!=0){
		try {
		    loadConfig(path);
		 }
		  catch(std::exception const & e)
		  {
			std::cerr << "xpsycle: configuration: error: " << e.what() << std::endl;
		  }
		}*/
	}
   }
}

void Configuration::loadConfig(std::string const & path) throw(std::exception)
{
  #if !defined NDEBUG
    std::cout << "xpsycle: configuration: attempting to load file: " << path << std::endl;
  #endif

  // check whether the file is readable
  if(!NFile::fileIsReadable(path))
  {
    std::ostringstream s;
    s << "cannot read file: " << path;
    throw std::runtime_error(s.str());
  }

  // parse the file
  try
  {
    NApp::config()->loadXmlConfig(path, /* throw_allowed */ true);
  }
  catch(std::exception const & e)
  {
    std::ostringstream s;
    s <<
        "an error occured while parsing xml configuration file " << path << " ; "
        "the settings might have been only partially loaded ; "
        "message: " << e.what();
    throw std::runtime_error(s.str());
    } catch(...)
    {
      std::ostringstream s;
      s <<
        "an unidentified error occured while parsing xml configuration file " << path << " ; "
        "the settings might have been only partially loaded";
      throw std::runtime_error(s.str());
    }
    // the parser defaults to empty strings on missing values
  // so we make sure not to override previous settings with empty strings
  {
    std::string const s(NFile::replaceTilde(NApp::config()->findPath("icondir")));
    if(s.length()) iconPath = s;
  }

  {
    std::string const s(NFile::replaceTilde(NApp::config()->findPath("plugindir")));
    if(s.length()) pluginPath = s;
  }

  {
    std::string const s(NFile::replaceTilde(NApp::config()->findPath("prsdir")));
    if(s.length()) prsPath = s;
  }

  {
    std::string const s(NFile::replaceTilde(NApp::config()->findPath("hlpdir")));
    if(s.length()) hlpPath = s;
  }

  #if !defined NDEBUG
  std::cout
    << "xpsycle: configuration: after loading file: " << path << "\n"
    << "xpsycle: configuration: pixmap dir: " << iconPath << "\n"
    << "xpsycle: configuration: plugin dir: " << pluginPath << "\n"
    << "xpsycle: configuration: preset dir: " << prsPath << "\n"
    << "xpsycle: configuration: doc    dir: " << hlpPath << "\n";
  #endif

	doEnableSound = true;
}

void Configuration::onConfigTagParse(const std::string & tagName )
{
	if (tagName == "driver" && doEnableSound) {		
			setDriverByName(NApp::config()->getAttribValue("name"));
	} else
  if (tagName == "alsa") {
    device_name = NApp::config()->getAttribValue("device");
		std::map< std::string, AudioDriver*>::iterator it = driverMap_.begin();
		if ( ( it = driverMap_.find( "alsa" ) ) != driverMap_.end() ) {
			AudioDriverSettings settings = it->second->settings();
			settings.setDeviceName( device_name );
			it->second->setSettings( settings );
		}		
  } else
  if (tagName == "audio") {
      std::string enableStr = NApp::config()->getAttribValue("enable");
      int enable = 0;
      if (enableStr != "") enable = str<int>(enableStr);
      enableSound = enable;
      if (enable == 0) {
				setDriverByName("silent");
				doEnableSound = false;
      } else doEnableSound = true;
  } else
  if (tagName == "key") {
  
      // the keycode id 
      std::string id         = NApp::config()->getAttribValue("id");

      // define special chars pressed in addition to this key function
      std::string modInput    = NApp::config()->getAttribValue("mod");
      int shift = nsNone;
      if (modInput == "ctrl")  {
        shift = nsCtrl;   
      } else
      if (modInput == "shift") {
        shift = nsShift;      
      }

      // the keycode      
      int keyCode = 0;
      std::string keyCodeStr = NApp::config()->getAttribValue("keycode");
      if (keyCodeStr!="") keyCode = str<int>(keyCodeStr);
        std::string keyCharStr = NApp::config()->getAttribValue("keychar");
        
      if (keyCharStr!="") {
        keyCode = keyCharStr[0];
        if (keyCharStr == "NK_Left") { keyCode = NK_Left; }
        if (keyCharStr == "NK_Right") { keyCode = NK_Right; }
        if (keyCharStr == "NK_Up") { keyCode = NK_Up; }
        if (keyCharStr == "NK_Down") { keyCode = NK_Down; }
        if (keyCharStr == "NK_Page_Up") { keyCode = NK_Page_Up; }
        if (keyCharStr == "NK_Page_Down") { keyCode = NK_Page_Down; }
        if (keyCharStr == "NK_Home") { keyCode = NK_Home; }
        if (keyCharStr == "NK_End") { keyCode = NK_End; }
        if (keyCharStr == "NK_Tab") { keyCode = NK_Tab; }
        #ifdef __unix__
        if (keyCharStr == "XK_ISO_Left_Tab") { keyCode = XK_ISO_Left_Tab; }
        #endif
        if (keyCharStr == "NK_Insert") { keyCode = NK_Insert; }
        if (keyCharStr == "NK_BackSpace") { keyCode = NK_BackSpace; }
        if (keyCharStr == "NK_Delete") { std::cout << NK_Delete << std::endl; keyCode = NK_Delete; }
        if (keyCharStr == "F1") { keyCode = NK_F1; }
        if (keyCharStr == "F2") { keyCode = NK_F2; }
        if (keyCharStr == "F3") { keyCode = NK_F3; }
        if (keyCharStr == "F4") { keyCode = NK_F4; }
        if (keyCharStr == "F5") { keyCode = NK_F5; }
        if (keyCharStr == "F6") { keyCode = NK_F6; }
        if (keyCharStr == "F7") { keyCode = NK_F7; }
        if (keyCharStr == "F8") { keyCode = NK_F8; }
        if (keyCharStr == "F9") { keyCode = NK_F9; }
      } 
      if (id == "add_new_machine") {
        inputHandler.changeKeyCode(cdefAddMachine,Key(shift,keyCode));
      } else
      if (id == "block_copy") {
        inputHandler.changeKeyCode(cdefBlockCopy,Key(shift,keyCode));
      } else
      if (id == "block_cut") {
        inputHandler.changeKeyCode(cdefBlockCut,Key(shift,keyCode));
      } else
      if (id == "block_delete") {
        inputHandler.changeKeyCode(cdefBlockDelete,Key(shift,keyCode));
      } else
      if (id == "block_double") {
        inputHandler.changeKeyCode(cdefBlockDouble,Key(shift,keyCode));
      } else
      if (id == "block_end") {
        inputHandler.changeKeyCode(cdefBlockEnd,Key(shift,keyCode));
      } else
      if (id == "block_halve") {
        inputHandler.changeKeyCode(cdefBlockHalve,Key(shift,keyCode));
      } else
      if (id == "block_interpolate") {
        inputHandler.changeKeyCode(cdefBlockInterpolate,Key(shift,keyCode));
      } else
      if (id == "block_mix") {
        inputHandler.changeKeyCode(cdefBlockMix,Key(shift,keyCode));
      } else
      if (id == "block_paste") {
        inputHandler.changeKeyCode(cdefBlockPaste,Key(shift,keyCode));
      } else
      if (id == "block_select_all") {
        inputHandler.changeKeyCode(cdefSelectAll,Key(shift,keyCode));
      } else
      if (id == "block_select_bar") {
        inputHandler.changeKeyCode(cdefSelectBar,Key(shift,keyCode));
      } else
      if (id == "block_select_column") {
        inputHandler.changeKeyCode(cdefSelectCol,Key(shift,keyCode));
      } else
      if (id == "block_select_up") {
        inputHandler.changeKeyCode(cdefSelectUp,Key(shift,keyCode));
      } else
      if (id == "block_select_down") {
        inputHandler.changeKeyCode(cdefSelectDn,Key(shift,keyCode));
      } else
      if (id == "block_select_left") {
        inputHandler.changeKeyCode(cdefSelectLeft,Key(shift,keyCode));
      } else
      if (id == "block_select_right") {
        inputHandler.changeKeyCode(cdefSelectRight,Key(shift,keyCode));
      } else
      if (id == "block_select_top") {
        inputHandler.changeKeyCode(cdefSelectTop,Key(shift,keyCode));
      } else
      if (id == "block_select_bottom") {
        inputHandler.changeKeyCode(cdefSelectBottom,Key(shift,keyCode));
      } else
      if (id == "block_set_instrument") {
        inputHandler.changeKeyCode(cdefBlockSetInstr,Key(shift,keyCode));
      } else
      if (id == "block_set_machine") {
        inputHandler.changeKeyCode(cdefBlockSetMachine,Key(shift,keyCode));
      } else
      if (id == "block_start") {
        inputHandler.changeKeyCode(cdefBlockStart,Key(shift,keyCode));
      } else
      if (id == "block_unmark") {
        inputHandler.changeKeyCode(cdefBlockUnMark,Key(shift,keyCode));
      } else
      if (id == "clear_row") {
        inputHandler.changeKeyCode(cdefRowClear,Key(shift,keyCode));
      } else
      if (id == "current_instrument+1") {
        inputHandler.changeKeyCode(cdefInstrInc,Key(shift,keyCode));
      } else
      if (id == "current_instrument-1") {
        inputHandler.changeKeyCode(cdefInstrDec,Key(shift,keyCode));
      } else
      if (id == "current_machine+1") {
        inputHandler.changeKeyCode(cdefMachineInc,Key(shift,keyCode));
      } else
      if (id == "current_machine-1") {
        inputHandler.changeKeyCode(cdefMachineDec,Key(shift,keyCode));
      } else
      if (id == "current_octave+1") {
        inputHandler.changeKeyCode(cdefOctaveUp,Key(shift,keyCode));
      } else
      if (id == "current_octave-1") {
        inputHandler.changeKeyCode(cdefOctaveDn,Key(shift,keyCode));
      } else
      if (id == "current_pattern+1") {
        inputHandler.changeKeyCode(cdefPatternInc,Key(shift,keyCode));
      } else
      if (id == "current_pattern-1") {
        inputHandler.changeKeyCode(cdefPatternDec,Key(shift,keyCode));
      } else
      if (id == "delete_row") {
        inputHandler.changeKeyCode(cdefRowDelete,Key(shift,keyCode));
      } else
      if (id == "edit_instrument") {
        inputHandler.changeKeyCode(cdefEditInstr,Key(shift,keyCode));
      } else
      if (id == "edit_redo") {
        inputHandler.changeKeyCode(cdefRedo,Key(shift,keyCode));
      } else
      if (id == "edit_undo") {
        inputHandler.changeKeyCode(cdefUndo,Key(shift,keyCode));
      } else
      if (id == "edit_toggle") {
        inputHandler.changeKeyCode(cdefEditToggle,Key(shift,keyCode));
      } else
      if (id.find("oct")!=std::string::npos) {
        if (id == "oct_C_0") {
            inputHandler.changeKeyCode(cdefKeyC_0,Key(shift,keyCode));
        } else
        if (id == "oct_CS0") {
            inputHandler.changeKeyCode(cdefKeyCS0,Key(shift,keyCode));
        } else
        if (id == "oct_D_0") {
            inputHandler.changeKeyCode(cdefKeyD_0,Key(shift,keyCode));
        } else
        if (id == "oct_DS0") {
            inputHandler.changeKeyCode(cdefKeyDS0,Key(shift,keyCode));
        } else
        if (id == "oct_E_0") {
            inputHandler.changeKeyCode(cdefKeyE_0,Key(shift,keyCode));
        } else
        if (id == "oct_F_0") {
            inputHandler.changeKeyCode(cdefKeyF_0,Key(shift,keyCode));
        } else
        if (id == "oct_FS0") {
            inputHandler.changeKeyCode(cdefKeyFS0,Key(shift,keyCode));
        } else
        if (id == "oct_G_0") {
            inputHandler.changeKeyCode(cdefKeyG_0,Key(shift,keyCode));
        } else
        if (id == "oct_GS0") {
            inputHandler.changeKeyCode(cdefKeyGS0,Key(shift,keyCode));
        } else
        if (id == "oct_A_0") {
            inputHandler.changeKeyCode(cdefKeyA_0,Key(shift,keyCode));
        } else
        if (id == "oct_AS0") {
            inputHandler.changeKeyCode(cdefKeyAS0,Key(shift,keyCode));
        } else
        if (id == "oct_B_0") {
            inputHandler.changeKeyCode(cdefKeyB_0,Key(shift,keyCode));
        }  else // and now the all again for octave 2
        if (id == "oct_C_1") {
            inputHandler.changeKeyCode(cdefKeyC_1,Key(shift,keyCode));
        } else
        if (id == "oct_CS1") {
            inputHandler.changeKeyCode(cdefKeyCS1,Key(shift,keyCode));
        } else
        if (id == "oct_D_1") {
            inputHandler.changeKeyCode(cdefKeyD_1,Key(shift,keyCode));
        } else
        if (id == "oct_DS1") {
            inputHandler.changeKeyCode(cdefKeyDS1,Key(shift,keyCode));
        } else
        if (id == "oct_E_1") {
            inputHandler.changeKeyCode(cdefKeyE_1,Key(shift,keyCode));
        } else
        if (id == "oct_F_1") {
            inputHandler.changeKeyCode(cdefKeyF_1,Key(shift,keyCode));
        } else
        if (id == "oct_FS1") {
            inputHandler.changeKeyCode(cdefKeyFS1,Key(shift,keyCode));
        } else
        if (id == "oct_G_1") {
            inputHandler.changeKeyCode(cdefKeyG_1,Key(shift,keyCode));
        } else
        if (id == "oct_GS1") {
            inputHandler.changeKeyCode(cdefKeyGS1,Key(shift,keyCode));
        } else
        if (id == "oct_A_1") {
            inputHandler.changeKeyCode(cdefKeyA_1,Key(shift,keyCode));
        } else
        if (id == "oct_AS1") {
            inputHandler.changeKeyCode(cdefKeyAS1,Key(shift,keyCode));
        } else
        if (id == "oct_B_1") {
            inputHandler.changeKeyCode(cdefKeyB_1,Key(shift,keyCode));
        } else // and now again for octave 2
        if (id == "oct_C_2") {
            inputHandler.changeKeyCode(cdefKeyC_2,Key(shift,keyCode));
        } else
        if (id == "oct_CS2") {
            inputHandler.changeKeyCode(cdefKeyCS2,Key(shift,keyCode));
        } else
        if (id == "oct_D_2") {
            inputHandler.changeKeyCode(cdefKeyD_2,Key(shift,keyCode));
        } else
        if (id == "oct_DS2") {
            inputHandler.changeKeyCode(cdefKeyDS2,Key(shift,keyCode));
        } else
        if (id == "oct_E_2") {
            inputHandler.changeKeyCode(cdefKeyE_2,Key(shift,keyCode));
        } else
        if (id == "oct_F_2") {
            inputHandler.changeKeyCode(cdefKeyF_2,Key(shift,keyCode));
        } else
        if (id == "oct_FS2") {
            inputHandler.changeKeyCode(cdefKeyFS2,Key(shift,keyCode));
        } else
        if (id == "oct_G_2") {
            inputHandler.changeKeyCode(cdefKeyG_2,Key(shift,keyCode));
        } else
        if (id == "oct_GS2") {
            inputHandler.changeKeyCode(cdefKeyGS2,Key(shift,keyCode));
        } else
        if (id == "oct_A_2") {
            inputHandler.changeKeyCode(cdefKeyA_2,Key(shift,keyCode));
        }
      } else
      if (id == "key_stop") {
        inputHandler.changeKeyCode(cdefKeyStop,Key(shift,keyCode));
      } else
      if (id == "key_stop_current") {
        inputHandler.changeKeyCode(cdefKeyStopAny,Key(shift,keyCode)); // not sure or key_stop ?
      }if (id == "machine_info") {
        inputHandler.changeKeyCode(cdefInfoMachine,Key(shift,keyCode));
      } else
      if (id == "maximise_pattern_view") {
        inputHandler.changeKeyCode(cdefMaxPattern,Key(shift,keyCode));
      } else
      if (id == "mcm_midi_cc") {
        inputHandler.changeKeyCode(cdefMIDICC,Key(shift,keyCode));
      } else
      if (id == "nav_bottom") {
        inputHandler.changeKeyCode(cdefNavBottom,Key(shift,keyCode));
      } else
      if (id == "nav_down") {
        inputHandler.changeKeyCode(cdefNavDn,Key(shift,keyCode));
      } else
      if (id == "nav_down_16") {
        inputHandler.changeKeyCode(cdefNavPageDn,Key(shift,keyCode));
      } else
      if (id == "nav_left") {
        inputHandler.changeKeyCode(cdefNavLeft,Key(shift,keyCode));
      } else
      if (id == "nav_right") {
        inputHandler.changeKeyCode(cdefNavRight,Key(shift,keyCode));
      } else
      if (id == "nav_top") {
        inputHandler.changeKeyCode(cdefNavTop,Key(shift,keyCode));
      } else
      if (id == "nav_up") {
        inputHandler.changeKeyCode(cdefNavUp,Key(shift,keyCode));
      } else
      if (id == "nav_up_16") {
        inputHandler.changeKeyCode(cdefNavPageUp,Key(shift,keyCode));
      } else
      if (id == "nav_firsttrack") {
        inputHandler.changeKeyCode(cdefNavFirstTrack,Key(shift,keyCode));
      } else
      if (id == "nav_lasttrack") {
        inputHandler.changeKeyCode(cdefNavLastTrack,Key(shift,keyCode));
      } else
      if (id == "next_column") {
        inputHandler.changeKeyCode(cdefColumnNext,Key(shift,keyCode));
      } else
      if (id == "pattern_copy") {
        inputHandler.changeKeyCode(cdefPatternCopy,Key(shift,keyCode));
      } else
      if (id == "pattern_cut") {
        inputHandler.changeKeyCode(cdefPatternCut,Key(shift,keyCode));
      } else
      if (id == "pattern_delete") {
        inputHandler.changeKeyCode(cdefPatternDelete,Key(shift,keyCode));
      } else
      if (id == "pattern_info") {
        inputHandler.changeKeyCode(cdefInfoPattern,Key(shift,keyCode));
      } else
      if (id == "pattern_mix_paste") {
        inputHandler.changeKeyCode(cdefPatternMixPaste,Key(shift,keyCode));
      } else
      if (id == "pattern_paste") {
        inputHandler.changeKeyCode(cdefPatternPaste,Key(shift,keyCode));
      } else
      if (id == "pattern_track_mute") {
        inputHandler.changeKeyCode(cdefPatternTrackMute,Key(shift,keyCode));
      } else
      if (id == "pattern_track_record") {
        inputHandler.changeKeyCode(cdefPatternTrackRecord,Key(shift,keyCode));
      } else
      if (id == "pattern_track_solo") {
        inputHandler.changeKeyCode(cdefPatternTrackSolo,Key(shift,keyCode));
      } else
      if (id == "patternstep_dec") {
        inputHandler.changeKeyCode(cdefPatternstepDec,Key(shift,keyCode));
      } else
      if (id == "patternstep_inc") {
        inputHandler.changeKeyCode(cdefPatternstepInc,Key(shift,keyCode));
      } else
      if (id == "play_current_note") {
        inputHandler.changeKeyCode(cdefPlayFromPos,Key(shift,keyCode));
      } else
      if (id == "play_current_row") {
        inputHandler.changeKeyCode(cdefPlayRowTrack,Key(shift,keyCode));
      } else
      if (id == "play_sel_pattern_looped") {
        inputHandler.changeKeyCode(cdefKeyStopAny,Key(shift,keyCode));
      } else
      if (id == "play_song_current") {
        inputHandler.changeKeyCode(cdefPlayFromPos,Key(shift,keyCode));
      } else
      if (id == "play_song_start") {
        inputHandler.changeKeyCode(cdefPlayStart,Key(shift,keyCode));
      } else
      if (id == "play_song_normal") {
        inputHandler.changeKeyCode(cdefPlaySong,Key(shift,keyCode));
      } else
      if (id == "position+1") {
        inputHandler.changeKeyCode(cdefSongPosInc,Key(shift,keyCode));
      } else
      if (id == "position-1") {
        inputHandler.changeKeyCode(cdefSongPosDec,Key(shift,keyCode));
      } else
      if (id == "prev_column") {
        inputHandler.changeKeyCode(cdefColumnPrev,Key(shift,keyCode));
      } else
      if (id == "row_skip+1") {
        //inputHandler.changeKeyCode(cdefKeyStopAny,Key(mod,keyCode));
      } else
      if (id == "row_skip-1") {
        //inputHandler.changeKeyCode(cdefKeyStopAny,Key(mod,keyCode));
      } else
      if (id == "screen_machines") {
        inputHandler.changeKeyCode(cdefEditMachine,Key(shift,keyCode));
      } else
      if (id == "screen_patterns") {
        inputHandler.changeKeyCode(cdefEditPattern,Key(shift,keyCode));
      } else
      if (id == "screen_sequencer") {
        inputHandler.changeKeyCode(cdefEditSequence,Key(shift,keyCode));
      } else
      if (id == "select_mac_in_cursor") {
        //inputHandler.changeKeyCode(cdefKeyStopAny,Key(mod,keyCode));
      } else
      if (id == "show_error_log") {
        //inputHandler.changeKeyCode(cdefKeyStopAny,Key(mod,keyCode));
      } else
      if (id == "stop_playback") {
        inputHandler.changeKeyCode(cdefPlayStop,Key(shift,keyCode));
      } else
      if (id == "toggle_edit_mode") {
        //inputHandler.changeKeyCode(cdefKeyStopAny,Key(mod,keyCode));
      } else
      if (id == "toggle_follow_song") {
        //inputHandler.changeKeyCode(cdefKeyStopAny,Key(mod,keyCode));
      } else
      if (id == "transpose_block+1") {
        inputHandler.changeKeyCode(cdefTransposeBlockInc,Key(shift,keyCode));
      } else
      if (id == "transpose_block+12") {
        inputHandler.changeKeyCode(cdefTransposeBlockInc12,Key(shift,keyCode));
      } else
      if (id == "transpose_block-1") {
        inputHandler.changeKeyCode(cdefTransposeBlockDec,Key(shift,keyCode));
      } else
      if (id == "transpose_block-12") {
        inputHandler.changeKeyCode(cdefTransposeBlockDec12,Key(shift,keyCode));
      } else
      if (id == "transpose_channel+1") {
        inputHandler.changeKeyCode(cdefTransposeChannelInc,Key(shift,keyCode));
      } else
      if (id == "transpose_channel+12") {
        inputHandler.changeKeyCode(cdefTransposeChannelInc12,Key(shift,keyCode));
      } else
      if (id == "transpose_channel-1") {
        inputHandler.changeKeyCode(cdefTransposeChannelDec,Key(shift,keyCode));
      } else
      if (id == "transpose_channel-12") {
        inputHandler.changeKeyCode(cdefTransposeChannelDec12,Key(shift,keyCode));
      } else
      if (id == "tweak_parameter") {
        inputHandler.changeKeyCode(cdefTweakM,Key(shift,keyCode));
      } else
      if (id == "tweak_smooth_paramater") {
        inputHandler.changeKeyCode(cdefTweakS,Key(shift,keyCode));
      }
  }
}

DefaultBitmaps & Configuration::icons( )
{
  return *bitmaps_;
}

}
}
