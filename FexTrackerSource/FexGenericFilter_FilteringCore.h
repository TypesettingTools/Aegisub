// This file is part of FexGenericFilter and (C) 2006 by Hajo Krabbenhöft  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.

{
	FexFilterContribution* Contrib = new FexFilterContribution[ ImageOutSX ];
	double XScale = (double)ImageOutSX / (double) ImageInSX;

//calculate contributions

	if( XScale < 1.0 )
	{
		#define CONTRIB_XSCALE
		#include "FexGenericFilter_Contribution.h"
		#undef CONTRIB_XSCALE	
	}
	else
	{
		#include "FexGenericFilter_Contribution.h"
	}
	

	#include "FexGenericFilter_StaticFor.h"
//apply filter

	for( int y=0;y<ImageInSY;++y)
	{
		for( int x=0;x<ImageOutSX;++x )
		{
			FexFilterContribution* cc = &Contrib[x];
			
			double Sum[ImagePlanes];
			#define DOFOR(i)  Sum[i] = 0.0;
			STATIC_FOR
			#undef DOFOR	
			
			for( int lx=cc->xMin;lx<=cc->xMax;++lx )
			{
				#define DOFOR(i)  Sum[i] += ((double)ImageIn(lx,y,i))*cc->Weight[lx-cc->xMin];
				STATIC_FOR
				#undef DOFOR					
			}
			
			#define DOFOR(i)  {if(Sum[i]<PixelMin)Sum[i]=PixelMin;if(Sum[i]>PixelMax)Sum[i]=PixelMax;}
			STATIC_FOR
			#undef DOFOR	
			
			#define DOFOR(i)  ImageOut(x,y,i) = (PixelType) Sum[i];
			STATIC_FOR
			#undef DOFOR	
		}
	}

	for( int x=0;x<ImageOutSX;++x )
		delete []Contrib[x].Weight;
	delete []Contrib;
}

