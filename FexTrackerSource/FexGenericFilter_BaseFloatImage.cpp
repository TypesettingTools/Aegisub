// This file is part of FexGenericFilter and (C) 2006 by Hajo Krabbenhöft  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.

#include "StdAfx.h"
#include "FexGenericFilter_Include.h"


void BaseFloatImage_Filter( float* in, int inSx, int inSy, FexFilter* filter, float* out, int outSx, int outSy )
{
#define FilterWidth (filter->Width)
#define FilterWeight(t) (filter->Filter(t))

	float* tmp = new float[outSx*inSy];
#include "FexGenericFilter_BaseFloatImageApply.h"
	delete []tmp;

#undef FilterWidth
#undef FilterWeight
}

void BaseFloatImage_FilterSeperate( float* in, int inSx, int inSy, FexFilter* filterX, FexFilter* filterY, float* out, int outSx, int outSy )
{
	float* tmp = new float[outSx*inSy];

#define FilterWidth (filterX->Width)
#define FilterWeight(t) (filterX->Filter(t))
#define FILTER_NO_Y

#include "FexGenericFilter_BaseFloatImageApply.h"

#undef FilterWidth
#undef FilterWeight
#undef FILTER_NO_Y

#define FilterWidth (filterY->Width)
#define FilterWeight(t) (filterY->Filter(t))
#define FILTER_NO_X

#include "FexGenericFilter_BaseFloatImageApply.h"

#undef FilterWidth
#undef FilterWeight
#undef FILTER_NO_X

	delete []tmp;
}


void BaseFloatImage_GaussEdgeDetect( float* Img, int sizx, int sizy, float sigma, float* GradX, float* GradY )
{
	FexFilter_Gauss 			gauss (sigma);
	FexFilter_GaussDerivation 	gaussDeriv (sigma);

	BaseFloatImage_FilterSeperate( Img, sizx, sizy, &gaussDeriv, &gauss, GradX, sizx, sizy );
	BaseFloatImage_FilterSeperate( Img, sizx, sizy, &gauss, &gaussDeriv, GradY, sizx, sizy );
}

void BaseFloatImage_GaussSmooth( float* Img, int sizx, int sizy, float sigma, float* Out )
{
	FexFilter_Gauss 			gauss (sigma);
	BaseFloatImage_Filter( Img, sizx,sizy, &gauss, Out, sizx, sizy );
}

#include <math.h>
inline double sinc( double x )
{
	x *= 3.1415;
	if( x != 0 )
		return( sin(x) / x );
	return( 1.0 );
}
inline double RescaleFilter( double t )
{
	if( t < 0 )
		t = -t;
	if( t < 3.0 )
		return( sinc(t) * sinc(t/3.0) );
	return( 0.0 );
}

void BaseFloatImage_LanczosRescale( float* in, int inSx, int inSy, float* out, int outSx, int outSy )
{
#define FilterWidth (3)
#define FilterWeight(t) (RescaleFilter(t))

	float* tmp = new float[outSx*inSy];
#include "FexGenericFilter_BaseFloatImageApply.h"
	delete []tmp;

#undef FilterWidth
#undef FilterWeight
}

