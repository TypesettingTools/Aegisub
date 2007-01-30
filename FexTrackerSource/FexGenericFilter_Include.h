// This file is part of FexGenericFilter and (C) 2006 by Hajo Krabbenhöft  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.

class FexFilter {
public:
	FexFilter();
	~FexFilter();

	virtual double Filter( double t );

	double Width;
};

class FexFilter_Gauss : public FexFilter {
public:
	FexFilter_Gauss( double sigma );
	~FexFilter_Gauss();

	double Filter( double t );

	double Sigma;
	double TwoSigmaSq;
	double Normalize;
};

class FexFilter_GaussDerivation : public FexFilter {
public:
	FexFilter_GaussDerivation( double sigma );
	~FexFilter_GaussDerivation();

	double Filter( double t );

	double Sigma;
	double TwoSigmaSq;
	double Normalize;
};

struct FexFilterContribution {
	int xMin, xMax;
	double* Weight;
};

