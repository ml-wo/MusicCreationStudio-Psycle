#ifndef _GLOBAL_H
#define _GLOBAL_H

class Song;
class Player;
class Configuration;

#if defined(_WINAMP_PLUGIN_)

	bool FindFileinDir(char *dllname,CString &path);
	
	class Global
	{
	public:
		Global();
		~Global();

		static Song* _pSong;
		static Player* pPlayer;
		static Configuration* pConfig;
		static int _lbc;
	};

#else

	class Resampler;
	class InputHandler;

	class Global
	{
	public:
		Global();
		~Global();

		static unsigned int _cpuHz;
		static Song* _pSong;
		static Player* pPlayer;
		static int _lbc;
		static Configuration* pConfig;
		static Resampler* pResampler;
		static InputHandler* pInputHandler;
	};
#endif // _WINAMP_PLUGIN_

#endif