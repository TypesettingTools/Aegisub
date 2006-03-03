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

sum:
 			          
Integral from -sqrt(t LN(10) to sqrt(t LN(10) of f(x) dx
1.715955662·sqrt(t)
*/

FexFilter_Gauss::FexFilter_Gauss( double sigma )
{
	Sigma = sigma;
	TwoSigmaSq = 2*sigma*sigma;
	Width = sqrt( TwoSigmaSq*log(10) );
	Normalize = 1.0 / (1.715955662 * sqrt( TwoSigmaSq ));
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
0.7062351183·t^1.5
*/

FexFilter_GaussDerivation::FexFilter_GaussDerivation( double sigma )
{
	Sigma = sigma;
	TwoSigmaSq = 2*sigma*sigma;
	Width = sqrt( TwoSigmaSq*log(10) );
	Normalize = 1.0 / (0.7062351183 * pow( TwoSigmaSq, 1.5 ));
}

FexFilter_GaussDerivation::~FexFilter_GaussDerivation()
{
}

double FexFilter_GaussDerivation::Filter( double t )
{
	return -t * exp( -t*t / TwoSigmaSq ) * Normalize;	
}
