#ifndef _PLUGIN_H
#define _PLUGIN_H

#include "Machine.h"
#include "Song.h"
#include "Configuration.h"
#include "MachineInterface.h"
#include "Player.h"
#include "NewMachine.h"

class PluginFxCallback : public CFxCallback
{
public:
	HWND hWnd;

	virtual void MessBox(char* ptxt,char *caption,unsigned int type)
	{
		MessageBox(hWnd,ptxt,caption,type);
	}
	virtual int GetTickLength(void)
	{
		return Global::_pSong->SamplesPerTick;
	}
	virtual int GetSamplingRate(void)
	{
#if defined(_WINAMP_PLUGIN_)
		return Global::pConfig->_samplesPerSec;
#else
		return Global::pConfig->_pOutputDriver->_samplesPerSec;
#endif // _WINAMP_PLUGIN_
	}
	virtual int GetBPM(void)
	{
		return Global::pPlayer->bpm;
	}
	virtual int GetTPB(void)
	{
		return Global::pPlayer->tpb;
	}
};

class Plugin : public Machine
{
public:

	Plugin();
	virtual ~Plugin();

	virtual void Init(void);
	virtual void Work(int numSamples);
	virtual void Stop(void);
	void Tick(void);
	virtual void Tick(int channel, PatternEntry* pEntry);
	virtual char* GetName(void) { return _psName; };
	virtual int GetNumParams(void) { return _pInfo->numParameters; }
	virtual void GetParamName(int numparam,char* name)
	{
		if ( numparam < _pInfo->numParameters )
			strcpy(name,_pInfo->Parameters[numparam]->Name);
		else strcpy(name,"Out of Range");

	}
	virtual void GetParamValue(int numparam,char* parval)
	{
		if ( numparam < _pInfo->numParameters )
		{
			if ( _pInterface->DescribeValue(parval,numparam,_pInterface->Vals[numparam]) == false )
			{
				sprintf(parval,"%i",_pInterface->Vals[numparam]);
			}
		}
		else strcpy(parval,"Out of Range");
	}
	virtual int GetParamValue(int numparam)
	{
		if ( numparam < _pInfo->numParameters )
			return _pInterface->Vals[numparam];
		else return -1;
	}
	virtual bool SetParameter(int numparam,int value)
	{
		if ( numparam < _pInfo->numParameters )
		{
			_pInterface->ParameterTweak(numparam,value);
			return true;
		}
		else return false;
	}
	virtual bool Load(RiffFile* pFile);
	virtual bool LoadSpecificFileChunk(RiffFile* pFile, int version)
	{
		UINT size;
		pFile->Read(&size,sizeof(size));
		if (size)
		{
			byte* pData = new byte[size];
			pFile->Read(pData, size); // Number of parameters
			_pInterface->PutData(pData); // Internal load
			delete pData;
		}
		return TRUE;
	};

#if !defined(_WINAMP_PLUGIN_)
	virtual bool Save(RiffFile* pFile);
	virtual void SaveSpecificChunk(RiffFile* pFile) 
	{
		UINT size = _pInterface->GetDataSize();
		pFile->Write(&size,sizeof(size));
		if (size)
		{
			byte* pData = new byte[size];
			_pInterface->GetData(pData); // Internal save
			pFile->Write(pData, size); // Number of parameters
			delete pData;
		}
	};
	virtual void SaveDllName(RiffFile* pFile) 
	{
		CString str = _psDllName;
		char str2[256];
		strcpy(str2,str.Mid(str.ReverseFind('\\')+1));
		pFile->Write(&str2,strlen(str2)+1);
	};

#endif // ndef _WINAMP_PLUGIN_

	bool Instance(char* psFileName);
	void Free(void);
//	bool Create(Plugin *plug);
	bool LoadDll(char* psFileName)
	{
		_strlwr(psFileName);
		char sPath2[_MAX_PATH];
		CString sPath;
#if defined(_WINAMP_PLUGIN_)
		sPath = Global::pConfig->GetPluginDir();

		if ( FindFileinDir(psFileName,sPath) )
		{
			strcpy(sPath2,sPath);
			return Instance(sPath2);
		}
#else
		if ( !CNewMachine::dllNames.Lookup(psFileName,sPath) ) 
		{
//			Check Compatibility Table.
//			Probably could be done with the dllNames lockup.
//
//			GetCompatible(psFileName,sPath2) // If no one found, it will return a null string.
			strcpy(sPath2,psFileName);
		}
		else 
		{ 
			strcpy(sPath2,sPath); 
		}

		if (!Instance(sPath2))
		{
			char sError[_MAX_PATH];
			sprintf(sError,"Missing or corrupted native Plug-in \"%s\"",psFileName);
			::MessageBox(NULL,sError, "Error", MB_OK);
			return FALSE;
		}
		else
		{
			return TRUE;
		}
#endif // _WINAMP_PLUGIN_	};
		return FALSE;
	};

	bool IsSynth(void) { return _isSynth; }
	char* GetDllName(void) { return _psDllName; }
	char* GetShortName(void) { return _psShortName; }
	char* GetAuthor(void) { return _psAuthor; }

	CMachineInfo* GetInfo(void) { return _pInfo; };
	CMachineInterface* GetInterface(void) { return _pInterface; };
	PluginFxCallback* GetCallback(void) { return &_callback; };


protected:
	HINSTANCE _dll;
	char _psShortName[16];
	char* _psAuthor;
	char* _psDllName;
	char* _psName;
	bool _isSynth;
	static PluginFxCallback _callback;
	CMachineInfo* _pInfo;
	CMachineInterface* _pInterface;

};


#endif