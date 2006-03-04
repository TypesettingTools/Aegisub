// This file is part of FexGenericFilter and (C) 2006 by Hajo Krabbenhöft  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.

#if ImagePlanes==1	
#define STATIC_FOR	{DOFOR(0)}
#elif ImagePlanes==3
#define STATIC_FOR	{DOFOR(0) DOFOR(1) DOFOR(2)}
#elif ImagePlanes==4
#define STATIC_FOR	{DOFOR(0) DOFOR(1) DOFOR(2) DOFOR(3)}
#else
#define STATIC_FOR	{for( int dofori=0;dofori<ImagePlanes;++doforti ){DOFOR(i)}}
#endif

