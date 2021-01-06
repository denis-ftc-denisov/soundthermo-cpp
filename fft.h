#ifndef FELIX_THE_CAT_FAST_FOURIER_TRANSFORM_HEADER_FILE_INCLUDED_JYUWRGHJBVJHVBWJKFGKJJKFHBJSJKGHJKGH
#define FELIX_THE_CAT_FAST_FOURIER_TRANSFORM_HEADER_FILE_INCLUDED_JYUWRGHJBVJHVBWJKFGKJJKFHBJSJKGHJKGH

#include <complex>

using namespace std;

#define Complex complex<long double>

/* Defines Fast Fourier Transform */ 

void FFT(Complex *a, int n, bool inv = false);

#endif
