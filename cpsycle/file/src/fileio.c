// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "fileio.h"
#include <string.h> 
#include <stdio.h>
#include <assert.h>

uint32_t FourCC(char *psName)
{
	int32_t retbuf = 0x20202020;   // four spaces (padding)
	char *ps = ((char *)&retbuf);
	int i;
	
	// Remember, this is Intel format!
	// The first character goes in the LSB
	for (i = 0; i < 4 && psName[i]; ++i) {
		ps[i] = psName[i];
	}
	return retbuf;
}

int psyfile_open(PsyFile* self, const char* psFileName)
{
	strcpy(self->szName,psFileName);
	self->_file = fopen(psFileName, "rb");	
	return (self->_file != NULL);
}

int psyfile_create(PsyFile* self, const char* psFileName, int overwrite)
{	
	strcpy(self->szName,psFileName);
	self->_file = fopen(psFileName, "rb");	
	if (self->_file != NULL)
	{
		fclose(self->_file);
		if (!overwrite)
		{
			return 0;
		}
	}
	
	self->_file = fopen(psFileName, "wb");
	return (self->_file != NULL);
}

int psyfile_close(PsyFile* self)
{
	if ( self->_file != NULL ) {
		int b;
		fflush(self->_file);
		b = !ferror(self->_file);
		fclose(self->_file);
		self->_file = NULL;
		return b;
	}
	return 1;	
}
// read
int psyfile_read(PsyFile* self,
				   void* pData,
				   uint32_t numBytes)
{
	uint32_t bytesRead = fread(pData, sizeof(char), numBytes, self->_file);
	return (bytesRead == numBytes);	
}

int8_t psyfile_read_int8(PsyFile* self)
{
	int8_t temp;
	int err;

	err = psyfile_read(self, &temp, sizeof(temp));
	return temp;
}

uint8_t psyfile_read_uint8(PsyFile* self)
{
	uint8_t temp;
	int err;

	err = psyfile_read(self, &temp, sizeof(temp));
	return temp;
}

int16_t psyfile_read_int16(PsyFile* self)
{
	int16_t temp;
	int err;

	err = psyfile_read(self, &temp, sizeof(temp));
	return temp;	
}

uint16_t psyfile_read_uint16(PsyFile* self)
{
	uint16_t temp;
	int err;

	err = psyfile_read(self, &temp, sizeof(temp));				
	return temp;	
}

int32_t psyfile_read_int32(PsyFile* self)
{
	int32_t temp;
	int err;

	err = psyfile_read(self, &temp, sizeof(temp));
	return temp;	
}

uint32_t psyfile_read_uint32(PsyFile* self)
{
	uint32_t temp;
	int err;

	err = psyfile_read(self, &temp, sizeof(temp));
	return temp;
}

float psyfile_read_float(PsyFile* self)
{
	float temp;
	int err;

	assert(sizeof(float) != 32);
	err = psyfile_read(self, &temp, sizeof(temp));
	return temp;
}

int psyfile_write(PsyFile* self, const void* data, uint32_t numbytes)
{		
	uint32_t byteswritten;
	int status;

	fflush(self->_file);
	byteswritten = fwrite(data, sizeof(char), numbytes, self->_file);
	status = (byteswritten == numbytes) ? PSY_OK : PSY_ERRFILE;
	return status;
}

int psyfile_write_int8(PsyFile* self, int8_t value)
{
	return psyfile_write(self, &value, sizeof(int8_t));
}

int psyfile_write_uint8(PsyFile* self, uint8_t value)
{
	return psyfile_write(self, &value, sizeof(uint8_t));
}

int psyfile_write_int16(PsyFile* self, int16_t value)
{	
	return psyfile_write(self, &value, sizeof(int16_t));
}

int psyfile_write_uint16(PsyFile* self, uint16_t value)
{
	return psyfile_write(self, &value, sizeof(uint16_t));
}

int psyfile_write_int32(PsyFile* self, int32_t value)
{
	return psyfile_write(self, &value, sizeof(int32_t));
}

int psyfile_write_uint32(PsyFile* self, uint32_t value)
{	
	return psyfile_write(self, &value, sizeof(uint32_t));
}

int psyfile_write_float(PsyFile* self, float value)
{	
	return psyfile_write(self, &value, sizeof(float));
}

int psyfile_expect(PsyFile* self,
					 void* pData,
					 uint32_t numBytes)
{
	unsigned char c;
	
	while (numBytes-- != 0)
	{
		if (fread(&c, sizeof(c), 1, self->_file) != 1)
		{
			return 0;
		}
		if (c != *((char*)pData))
		{
			return 0;
		}
		pData = (char*)pData + 1;
	}
	return 1;	
}

uint32_t psyfile_seek(PsyFile* self,
				   uint32_t offset)
{
	if (fseek(self->_file, (long)offset, SEEK_SET) != 0)
	{
		return -1;
	}
	return ftell(self->_file);
	
}

uint32_t psyfile_skip(PsyFile* self,
				   uint32_t numBytes)
{
	if (fseek(self->_file, (long)numBytes, SEEK_CUR) != 0) return -1;
	return ftell(self->_file);
}

int psyfile_eof(PsyFile* self)
{
	return feof(self->_file);
	
}

uint32_t psyfile_getpos(PsyFile* self)
{
	return ftell(self->_file);
}


uint32_t psyfile_filesize(PsyFile* self)
{	
	int init = ftell(self->_file);
	int end;
	fseek(self->_file, 0,SEEK_END);
	end = ftell(self->_file);
	fseek(self->_file,init,SEEK_SET);
	return end;
	
}

int psyfile_readstring(PsyFile* self, char* pData, uint32_t maxBytes)
{
	if (maxBytes > 0)
	{		
		char c;
		uint32_t index;

		memset(pData, 0, maxBytes);
		for (index = 0; index < maxBytes; index++)
		{
			if (psyfile_read(self, &c, sizeof(c)))
			{
				pData[index] = c;
				if (c == 0)
				{
					return 1;
				}
			}
			else 
			{
				return 0;
			}
		}
		do
		{
			if (!psyfile_read(self, &c, sizeof(c)))
			{
				return 0;
			}
		} while (c != 0);
		return 1;
	}
	return 0;
	
}

FILE* psyfile_getfile(PsyFile* self)
{
	return self->_file;
}

int psyfile_writeheader(PsyFile* file, char* data, uint32_t version,
	uint32_t size, uint32_t* pos)
{	
	int status;

	if (status = psyfile_write(file, data, 4)) {
		return status;
	}
	if (status = psyfile_write_uint32(file, version)) {
		return status;
	}
	*pos = psyfile_getpos(file);
	if (status = psyfile_write_uint32(file, size)) {
		return status;
	}
	return status;
}

int psyfile_writestring(PsyFile* file, const char* str)
{
	return psyfile_write(file, str, (uint32_t)strlen(str) + 1);	
}

uint32_t psyfile_updatesize(PsyFile* file, uint32_t startpos)
{
	uint32_t pos2;
	uint32_t size;
	
	pos2 = psyfile_getpos(file); 
	size = (pos2 - startpos - 4);
	psyfile_seek(file, startpos);
	psyfile_write(file, &size, sizeof(size));
	psyfile_seek(file, pos2);
	return size;
}

int psyfile_readchunkbegin(PsyFile* self)
{
	--self->chunkcount;
	psyfile_read(self, &self->currchunk.version, sizeof(uint32_t));
	psyfile_read(self, &self->currchunk.size, sizeof(uint32_t));
	self->currchunk.begins = psyfile_getpos(self);
	return 1;
}

void psyfile_seekchunkend(PsyFile* self)
{
	psyfile_seek(self, self->currchunk.begins + self->currchunk.size);	
}
