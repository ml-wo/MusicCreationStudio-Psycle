//////////////////////////////////////////////////////////////////////
//
//	Dsp.cpp
//
//	druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#include <packageneric/pre-compiled.private.hpp>
#include "Dsp.h"
#include "../blwtbl/blwtbl.h"
//////////////////////////////////////////////////////////////////////
//
//	Variables
//
//////////////////////////////////////////////////////////////////////
//
//	pow2 table
float pow2table[POW2TABLESIZE];
const float POW2TABLEFACT = 16384.0f;
//	fm & pm tables
float *pfmtable;
float *ppmtable;
//	Truncation for f2i
unsigned short cwTrunc = 0x1f72;
const double fimagic = 6755399441055744.0;
const double fihalf = 0.5;
double fitmp;
//////////////////////////////////////////////////////////////////////
//
//	InitializeDSP
//
//////////////////////////////////////////////////////////////////////
void __cdecl InitializeDSP()
{
	//
	//	Setup table for pow2
	for (int i = 0; i < POW2TABLESIZE; i++)
	{
		pow2table[i] = (float) pow(2.0, (double) i / (double) POW2TABLESIZE);
	}
	//
	//	Get amptable
	pfmtable = GetFMTable();
	ppmtable = GetPMTable();
}
//////////////////////////////////////////////////////////////////////
//
//	DestroyDSP
//
//////////////////////////////////////////////////////////////////////
void __cdecl DestroyDSP()
{
}
//////////////////////////////////////////////////////////////////////
//
//	Fill
//
//////////////////////////////////////////////////////////////////////
void __fastcall Fill(float *pbuf, float value, int nsamples)
{
	--pbuf;
	do
	{
		*++pbuf = value;
	}
	while (--nsamples);
}
//////////////////////////////////////////////////////////////////////
//
//	Copy
//
//////////////////////////////////////////////////////////////////////
void __fastcall Copy(float *pbuf1, float *pbuf2, int nsamples)
{
	--pbuf1;
	--pbuf2;
	do
	{
		*++pbuf1 = *++pbuf2;
	}
	while (--nsamples);
}
//////////////////////////////////////////////////////////////////////
//
//	Add
//
//////////////////////////////////////////////////////////////////////
void __fastcall Add(float *pbuf1, float *pbuf2, int nsamples)
{
	--pbuf1;
	--pbuf2;
	do
	{
		*++pbuf1 += *++pbuf2;
	}
	while (--nsamples);
}
//////////////////////////////////////////////////////////////////////
//
//	Sub
//
//////////////////////////////////////////////////////////////////////
void __fastcall Sub(float *pbuf1, float *pbuf2, int nsamples)
{
	--pbuf1;
	--pbuf2;
	do
	{
		*++pbuf1 -= *++pbuf2;
	}
	while (--nsamples);
}
//////////////////////////////////////////////////////////////////////
//
//	Mul
//
//////////////////////////////////////////////////////////////////////
void __fastcall Mul(float *pbuf1, float *pbuf2, int nsamples)
{
	--pbuf1;
	--pbuf2;
	do
	{
		*++pbuf1 *= *++pbuf2;
	}
	while (--nsamples);
}
