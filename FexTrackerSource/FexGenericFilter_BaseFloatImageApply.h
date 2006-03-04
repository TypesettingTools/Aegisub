// This file is part of FexGenericFilter and (C) 2006 by Hajo Krabbenhöft  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.

{
#define PixelType float
#define	PixelMin 0
#define	PixelMax 255
#define ImagePlanes 	1

	//do filtering + scaling in x-dir
#define	ImageInSX		(inSx)
#define	ImageInSY		(inSy)
#define ImageIn(x,y,p) 	(in[ (y*inSx+x) ])
#define	ImageOutSX		(outSx)
#define ImageOut(x,y,p) (tmp[ (y*outSx+x) ])

#ifndef FILTER_NO_X
#include "FexGenericFilter_FilteringCore.h"
#endif

#undef ImageInSX
#undef ImageInSY
#undef ImageIn
#undef ImageOutSX
#undef ImageOut

	//do filtering + scaling in y-dir by using transposed image

#define	ImageInSX		(inSy)
#define	ImageInSY		(outSx)
#define ImageIn(y,x,p) 	(tmp[ (y*outSx+x) ])
#define	ImageOutSX		(outSy)
#define ImageOut(y,x,p) (out[ (y*outSx+x) ])

#ifndef FILTER_NO_Y
#include "FexGenericFilter_FilteringCore.h"
#endif

#undef ImageInSX
#undef ImageInSY
#undef ImageIn
#undef ImageOutSX
#undef ImageOut

#undef PixelType
#undef PixelMin
#undef PixelMax
#undef ImagePlanes
}