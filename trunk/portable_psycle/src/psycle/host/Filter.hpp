///\file
///\brief interface file for psycle::host::Filter.
#pragma once
#include <cmath>
#include "Configuration.hpp"
namespace psycle
{
	namespace host
	{
		namespace dsp
			{
		#define TPI 6.28318530717958647692528676655901

		enum FilterType{
			F_LOWPASS12 = 0,
			F_HIGHPASS12 = 1,
			F_BANDPASS12 = 2,
			F_BANDREJECT12 = 3,
			F_NONE = 4
		};

		class FilterCoeff
		{
		public:
			float _coeffs[5][128][128][5];
			FilterCoeff() { _inited = false; };
			inline void Init(void)
			{
				if (!_inited)
				{
					_inited = true;
					for (int r=0; r<5; r++)
					{
						for (int f=0; f<128; f++)
						{
							for (int q=0; q<128; q++)
							{
								ComputeCoeffs(f, q, r);
								_coeffs[r][f][q][0] = (float)_coeff[0];
								_coeffs[r][f][q][1] = (float)_coeff[1];
								_coeffs[r][f][q][2] = (float)_coeff[2];
								_coeffs[r][f][q][3] = (float)_coeff[3];
								_coeffs[r][f][q][4] = (float)_coeff[4];
							}
						}
					}
				}
			};
		private:
			bool _inited;
			double _coeff[5];
			void ComputeCoeffs(int freq, int r, int t)
			{
				///\todo use unified function instead
				float omega = float(TPI*Cutoff(freq)/Global::pConfig->_pOutputDriver->_samplesPerSec);
				float sn = (float)sin(omega);
				float cs = (float)cos(omega);
				float alpha;
				if (t < 2) alpha = float(sn / Resonance( r *(freq+70)/(127.0f+70)));
				else alpha = float(sn * sinh( Bandwidth( r) * omega/sn));
				float a0, a1, a2, b0, b1, b2;
				switch (t)
				{
				case F_LOWPASS12:
					b0 =  (1 - cs)/2;
					b1 =   1 - cs;
					b2 =  (1 - cs)/2;
					a0 =   1 + alpha;
					a1 =  -2*cs;
					a2 =   1 - alpha;
					break;
				case F_HIGHPASS12:
					b0 =  (1 + cs)/2;
					b1 = -(1 + cs);
					b2 =  (1 + cs)/2;
					a0 =   1 + alpha;
					a1 =  -2*cs;
					a2 =   1 - alpha;
					break;
				case F_BANDPASS12:
					b0 =   alpha;
					b1 =   0;
					b2 =  -alpha;
					a0 =   1 + alpha;
					a1 =  -2*cs;
					a2 =   1 - alpha;
					break;
				case F_BANDREJECT12:
					b0 =   1;
					b1 =  -2*cs;
					b2 =   1;
					a0 =   1 + alpha;
					a1 =  -2*cs;
					a2 =   1 - alpha;
					break;
				default:
					b0 =  0;
					b1 =  0;
					b2 =  0;
					a0 =  0;
					a1 =  0;
					a2 =  0;
				}
				_coeff[0] = b0/a0;
				_coeff[1] = b1/a0;
				_coeff[2] = b2/a0;
				_coeff[3] = -a1/a0;
				_coeff[4] = -a2/a0;
			};

			static inline float Cutoff(int v)
			{
				return float(pow( (v+5)/(127.0+5), 1.7)*13000+30);
			};
			
			static inline float Resonance(float v)
			{
				return float(pow( v/127.0, 4)*150+0.1);
			};
			
			static inline float Bandwidth(int v)
			{
				return float(pow( v/127.0, 4)*4+0.1);
			};
		};

		/// filter.
		class Filter
		{
		public:
			FilterType _type;
			int _cutoff;
			int _q;

			Filter();

			void Init(void);
			void Update(void);
			inline float Work(float x)
			{
				float y = _coeff0*x + _coeff1*_x1 + _coeff2*_x2 + _coeff3*_y1 + _coeff4*_y2;
				_y2 = _y1;
				_y1 = y;
				_x2 = _x1;
				_x1 = x;
				return y;
			};
			inline void WorkStereo(float& l, float& r)
			{
				float y = _coeff0*l + _coeff1*_x1 + _coeff2*_x2 + _coeff3*_y1 + _coeff4*_y2;
				_y2 = _y1;
				_y1 = y;
				_x2 = _x1;
				_x1 = l;
				l = y;
				float b = _coeff0*r + _coeff1*_a1 + _coeff2*_a2 + _coeff3*_b1 + _coeff4*_b2;
				_b2 = _b1;
				_b1 = b;
				_a2 = _a1;
				_a1 = r;
				r = b;
			};
		protected:
			static FilterCoeff _coeffs;
			float _coeff0;
			float _coeff1;
			float _coeff2;
			float _coeff3;
			float _coeff4;
			float _x1, _x2, _y1, _y2;
			float _a1, _a2, _b1, _b2;
		};
		}
	}
}