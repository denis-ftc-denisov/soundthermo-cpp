#ifndef FELIX_THE_CAT_FAST_FOURIER_TRANSFORM_HEADER_FILE_INCLUDED_JYUWRGHJBVJHVBWJKFGKJJKFHBJSJKGHJKGH
#define FELIX_THE_CAT_FAST_FOURIER_TRANSFORM_HEADER_FILE_INCLUDED_JYUWRGHJBVJHVBWJKFGKJJKFHBJSJKGHJKGH

#include <complex>

using namespace std;

#define Complex complex<long double>

/* Defines Fast Fourier Transform */ 

void FFT(Complex *a, int n, bool inv = false);

/*
  	IMPLEMENTATION
*/

const long double PI = 3.1415926535897932384626433832795;

int Rev(int i, int l)
{
	int res = 0;
	int j;
	for (j = 0; j < l; j++)
	{
		res <<= 1;
		if (i & (1 << j)) res++;
	}
	return res;
}

void FFT(Complex *a, int n, bool inv)
{
	int i, j, k;
	int logn = 0;
	while ((1 << logn) < n) logn++;
	for (i = 0; i < n; i++)
	{
		j = Rev(i, logn);
		if (j > i)
		{
			Complex t = a[i];
			a[i] = a[j];
			a[j] = t;
		}
	}
	Complex u, v;
	for (i = 1; i <= logn; i++)
	{
		long double ang = 2 * PI / (1 << i);
		if (inv) ang = -ang;
		Complex w(cos(ang), sin(ang));
		for (j = 0; j < n; j += (1 << i))
		{
			Complex wk(1, 0);
			for (k = j; k < j + (1 << (i - 1)); k++)
			{
				u = a[k];
				v = a[k + (1 << (i - 1))] * wk;
				a[k] = u + v;
				a[k + (1 << (i - 1))] = u - v;
				wk *= w;
			}
			wk *= w;
		}
	}
	if (inv)
	{
		for (i = 0; i < n; i++) a[i] /= n;
	}
}

#endif
