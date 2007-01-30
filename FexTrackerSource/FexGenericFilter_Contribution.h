// This file is part of FexGenericFilter and (C) 2006 by Hajo Krabbenhöft  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.

{
	double width = FilterWidth;
#ifdef CONTRIB_XSCALE
    width /= XScale;
    double fscale = 1.0 / XScale;
#endif

    for(int i=0;i<ImageOutSX;++i)
    {
        double center = (double) i / XScale;
        FexFilterContribution* cc = &Contrib[i];
        int left = (int) ceil(center - width);
        int right = (int) (floor(center + width) + 0.1);

        cc->xMin = left;
        cc->xMax = right;
        if( cc->xMin < 0 ) cc->xMin = 0;
        if( cc->xMax > ImageInSX-1 ) cc->xMax = ImageInSX - 1;

        int len = cc->xMax-cc->xMin+1;
        cc->Weight = new double[ len ];
        memset( cc->Weight, 0x00, sizeof(double)*len );
        
        for(int j=left;j<=right;++j) {
            double weight = center - (double) j;
#ifdef CONTRIB_XSCALE
            weight = FilterWeight(weight / fscale) / fscale;
#else
            weight = FilterWeight(weight);
#endif
			int n;
            if(j < 0) n=0;
            else if(j >= ImageInSX) n = ImageInSX - 1;
            else n = j;
            cc->Weight[n-cc->xMin] += weight;
        }
    }
}

