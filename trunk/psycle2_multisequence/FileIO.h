#ifndef _FILEIO_H
#define _FILEIO_H

#if defined(_WINAMP_PLUGIN_)
	#include <stdio.h>
#endif // defined(_WINAMP_PLUGIN_)

typedef struct TYPEULONGINV {
  unsigned char hihi;
  unsigned char hilo;
  unsigned char lohi;
  unsigned char lolo;
} ULONGINV;


typedef struct
{
	ULONG _id;
	ULONG _size; // This one should be ULONGINV (it is, at least, in the files I([JAZ]) have tested)
}
RiffChunkHeader;

class RiffFile
{
public:
	RiffChunkHeader _header;

	virtual bool Open(char* psFileName);
	virtual bool Create(char* psFileName, bool overwrite);
	virtual BOOL Close(void);
	virtual bool Read(void* pData, ULONG numBytes);
	virtual bool Write(void* pData, ULONG numBytes);
	virtual bool Expect(void* pData, ULONG numBytes);
	virtual long Seek(long offset);
	virtual long Skip(long numBytes);
	virtual bool Eof(void);
	virtual long FileSize(void);
	virtual bool ReadString(char* pData, ULONG maxBytes);
	virtual long GetPos(void);
	
	virtual FILE* GetFile(void) { return NULL; };

	static ULONG FourCC(char *psName);
	char szName[MAX_PATH];

protected:
	HANDLE _handle;
	bool _modified;
};

class OldPsyFile : public RiffFile
{
public:
	virtual bool Open(char* psFileName);
	virtual bool Create(char* psFileName, bool overwrite);
	virtual BOOL Close(void);
	virtual BOOL Error();
	virtual bool Read(void* pData, ULONG numBytes);
	virtual bool Write(void* pData, ULONG numBytes);
	virtual bool Expect(void* pData, ULONG numBytes);
	virtual long Seek(long offset);
	virtual long Skip(long numBytes);
	virtual bool Eof(void);
	virtual long FileSize(void);
	virtual long GetPos(void);
	

	virtual FILE* GetFile(void) { return _file; };

protected:
	FILE* _file;
};

#endif
