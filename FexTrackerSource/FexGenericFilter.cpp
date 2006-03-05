// This file is part of FexGenericFilter and (C) 2006 by Hajo Krabbenh鐪t  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.

#include <math.h>
#include "FexGenericFilter_Include.h"

FexFilter::FexFilter()
{
	Width = 3;
}
FexFilter::~FexFilter()
{
}
double FexFilter::Filter( double t )
{
	return t/Width;
}


/*
f(x) = e^(-x^2 / (2 s^2) )
f(x) = e^(-x^2 / t)

width:
SOLVE(0.1 = f(x), x, Real)
x = - sqrt(t LN(10) ? x = sqrt(t LN(10)
x = - 1.517427129新qrt t ? x = 1.517427129新qrt t
x = - sqrt(2*s*s* LN(10)) ? x = sqrt(2*s*s* LN(10))
x = - 2.145966026新 ? x = 2.145966026新

sum:
 			          
Integral from -sqrt(t LN(10) to sqrt(t LN(10) of f(x) dx
1.715955662新qrt(t)
2.426727768新
*/

FexFilter_Gauss::FexFilter_Gauss( double sigma )
{
	Sigma = sigma;
	TwoSigmaSq = 2*sigma*sigma;
	Width = 2.145966026 * sigma;
	Normalize = 1.0 / (2.426727768 * sigma);
	Normalize *= 1.0 + 0.1 / Width;  //its the 0.1 we left out
}
FexFilter_Gauss::~FexFilter_Gauss()
{
}

double FexFilter_Gauss::Filter( double t )
{
	return exp( -t*t / TwoSigmaSq ) * Normalize;
}




/*
f(x) = -x * e^(-x^2 / (2 s^2) )
f(x) = -x * e^(-x^2 / t)

width:
use the width of gauss since i'm clueless here

sum:
 			          
Integral from -sqrt(t LN(10) to sqrt(t LN(10) of -x*f(x) dx
0.7062351183暗^1.5
*/

FexFilter_GaussDerivation::FexFilter_GaussDerivation( double sigma )
{
	Sigma = sigma;
	TwoSigmaSq = 2*sigma*sigma;
	Width = 2.145966026 * sigma;
	Normalize = 1.0 / (0.7062351183 * pow( TwoSigmaSq, 1.5 ));
}

FexFilter_GaussDerivation::~FexFilter_GaussDerivation()
{
}

double FexFilter_GaussDerivation::Filter( double t )
{
	return -t * exp( -t*t / TwoSigmaSq ) * Normalize;	
}
