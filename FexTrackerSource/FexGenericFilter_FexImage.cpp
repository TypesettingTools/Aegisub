// This file is part of FexGenericFilter and (C) 2006 by Hajo Krabbenhöft  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.

#include "stdafx.h"
#include "FexSystem.h"
#include "ext\imdebug.h"


void FexImage_Filter( FexImage* in, FexFilter* filter, FexImage* out )
{
#define FilterWidth (filter->Width)
#define FilterWeight(t) (filter->Filter(t))

#include "FexGenericFilter_FexImageApply.h"

#undef FilterWidth
#undef FilterWeight
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


void FexImage_Rescale( FexImage* in, FexImage* out )
{
#define FilterWidth (3)
#define FilterWeight(t) (RescaleFilter(t))

#include "FexGenericFilter_FexImageApply.h"

#undef FilterWidth
#undef FilterWeight
}
