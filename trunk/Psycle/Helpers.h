#ifndef HELPERS_H
#define HELPERS_H

int _httoi(const TCHAR *value);

inline float fast_log2(float f) 
{ 
//  assert( f > 0. ); 
//  assert( sizeof(f) == sizeof(int) ); 
//  assert( sizeof(f) == 4 ); 
  int i = (*(int *)&f); 
  return (((i&0x7f800000)>>23)-0x7f)+(i&0x007fffff)/(float)0x800000; 
} 

inline int f2i(float flt) 
{ 
  int i; 
  static const double half = 0.5f; 
  _asm 
  { 
	 fld flt 
	 fsub half 
	 fistp i 
  } 
  return i;
}



#endif
