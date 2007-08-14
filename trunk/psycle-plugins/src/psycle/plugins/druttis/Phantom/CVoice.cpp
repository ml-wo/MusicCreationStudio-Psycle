//============================================================================
//
//				CVoice.cpp
//				----------
//				druttis@darkface.pp.se
//
//============================================================================
#include <packageneric/pre-compiled.private.hpp>
#include "CVoice.h"

double coeff[5][11] = {
	{
		8.11044e-06, 8.943665402, -36.83889529, 92.01697887, -154.337906, 181.6233289,
		-151.8651235, 89.09614114, -35.10298511, 8.388101016, -0.923313471
	},
	{
		4.36215e-06, 8.890438318, -36.55179099, 91.05750846, -154.422234, 179.1170248,
		-149.6496211, 87.78352223, -34.60687431, 8.282228154, -0.914150747
	},
	{
		3.33819e-06, 8.893102966, -36.49532826, 90.96543286, -154.4545478, 179.4835618,
		-150.315433, 88.43409371, -34.98612086, 8.407803364, -0.932568035
	},
	{
		1.13572e-06, 8.994734087, -37.2084849, 93.22900521, -156.6929844, 184.596544,
		-154.3755513, 90.49663749, -35.58964535, 8.478996281, -0.929252233
	},
	{
		4.09431e-07, 8.997322763, -37.20218544, 93.11385476, -156.2530937, 183.7080141,
		-153.2631681, 89.59539726, -35.12454591, 8.338655623, -0.910251753
	}
};

//============================================================================
//				Constructor
//============================================================================
CVoice::CVoice()
{
}
//============================================================================
//				Destructor
//============================================================================
CVoice::~CVoice()
{
}
//============================================================================
//				Stop
//============================================================================
void CVoice::Stop()
{
	vca.Kill();
	vcf.Kill();
}
//============================================================================
//				NoteOff
//============================================================================
void CVoice::NoteOff()
{
	vca.Stop();
	vcf.Stop();
}
//============================================================================
//				NoteOn
//============================================================================
void CVoice::NoteOn(int note, int volume)
{
	//
	velocity = (float) volume / 255.0f;
	//
	float nnote;
	for (int i = 0; i < 6; i++) {
		nnote = (float) note + globals->osc_semi[i] + globals->osc_fine[i];
		osc_phase[i] = globals->osc_phase[i];
		osc_increment[i] = CDsp::GetFreq(nnote, WAVESIZE, globals->samplingrate);
	}
	//
	vca.SetAttack(globals->vca_attack);
	vca.SetDecay(globals->vca_decay);
	vca.SetSustain(globals->vca_sustain);
	vca.SetRelease(globals->vca_release);
	vca.Begin();
	//
	vcf.SetAttack(globals->vcf_attack);
	vcf.SetDecay(globals->vcf_decay);
	vcf.SetSustain(globals->vcf_sustain);
	vcf.SetRelease(globals->vcf_release);
	vcf.Begin();
	//
	for (int i = 0; i < 6; i++)
	{
		memory[i][0] = memory[i][1] = memory[i][2] = memory[i][3] = memory[i][4] = memory[i][5] = memory[i][6] = memory[i][7] = memory[i][8] = memory[i][9] = 0.0;
	}
	//
	filter_phase = 0.0f;
	filter.buf0 = filter.buf1 = 0.0f;
}
