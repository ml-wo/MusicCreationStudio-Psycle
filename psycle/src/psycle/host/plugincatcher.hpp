#pragma once

#include <psycle/host/detail/project.hpp>
#include <psycle/host/Global.hpp>
#include <psycle/host/plugininfo.hpp>
#include <psycle/host/machineloader.hpp>
#include <universalis/stdlib/cstdint.hpp>
#include <iostream>
#include <typeinfo>
#include <map>
namespace psycle
{
	namespace host
	{

		const int MAX_BROWSER_PLUGINS = 2048;
		class CProgressDialog;
		class PluginCatcher;

		class LoadPluginInfoParams
		{
		public:
			LoadPluginInfoParams():theEvent(FALSE,TRUE){}
			CEvent theEvent;
			bool verify;
			PluginCatcher* theCatcher;
		};

		class PluginCatcher : public MachineLoader
		{
		public:
			PluginCatcher();
			virtual ~PluginCatcher();
			/*override*/ bool lookupDllName(const std::string& name, std::string & result, MachineType tye,std::int32_t& shellIdx);
			/*override*/ bool TestFilename(const std::string & name,const std::int32_t shellIdx);
			/*override*/ void LoadPluginInfo(bool verify=true);
			/*override*/ void ReScan(bool regenerate=true);
			/*override*/ bool IsLoaded(){ return _numPlugins>0; }
			bool SaveCacheFile();
			void DestroyPluginInfo();
		protected:
			static DWORD ProcessLoadPlugInfo(void* newMacDlg);
			std::string preprocessName(const std::string& dllName);
			void learnDllName(const std::string & fullpath, MachineType type);
			void FindPlugins(int & currentPlugsCount, int & currentBadPlugsCount, std::vector<std::string> const & list, MachineType type, std::ostream & out, CProgressDialog * pProgress = 0);
			bool LoadCacheFile(int & currentPlugsCount, int & currentBadPlugsCount, bool verify);
			

			std::map<std::string,std::string> NativeNames;
			std::map<std::string,std::string> VstNames;
			std::map<std::string,std::string> LuaNames;

		public:
		///\todo: private:
			int _numPlugins;
			PluginInfo* _pPlugsInfo[MAX_BROWSER_PLUGINS];
		};

	}
}
