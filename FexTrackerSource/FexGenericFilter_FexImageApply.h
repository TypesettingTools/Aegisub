{
#define PixelType BYTE
#define	PixelMin 0
#define	PixelMax 255.9
#define ImagePlanes 	4

	FexImage* tmp = new FexImage(out->sx,in->sy);
			
	//do filtering + scaling in x-dir
#define	ImageInSX		(in->sx)
#define	ImageInSY		(in->sy)
#define ImageIn(x,y,p) 	(in->data[ (y*in->sx+x)*4 + p ])
#define	ImageOutSX		(tmp->sx)
#define ImageOut(x,y,p) (tmp->data[ (y*tmp->sx+x)*4 + p ])

#include "FexGenericFilter_FilteringCore.h"

#undef ImageInSX
#undef ImageInSY
#undef ImageIn
#undef ImageOutSX
#undef ImageOut

	//do filtering + scaling in y-dir by using transposed image

#define	ImageInSX		(tmp->sy)
#define	ImageInSY		(tmp->sx)
#define ImageIn(y,x,p) 	(tmp->data[ (y*tmp->sx+x)*4 + p ])
#define	ImageOutSX		(out->sy)
#define ImageOut(y,x,p) (out->data[ (y*out->sx+x)*4 + p ])

#include "FexGenericFilter_FilteringCore.h"

#undef ImageInSX
#undef ImageInSY
#undef ImageIn
#undef ImageOutSX
#undef ImageOut
	
	delete tmp;

#undef PixelType
#undef PixelMin
#undef PixelMax
#undef ImagePlanes
}