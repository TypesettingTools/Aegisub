// This file is part of FexTracker and (C) 2006 by Hajo Krabbenhöft  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.

// FexTrackingFeature.h: interface for the FexTrackingFeature class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FEXTRACKINGFEATURE_H__23B42013_9F11_467C_A0F6_F9E647D45CEB__INCLUDED_)
#define AFX_FEXTRACKINGFEATURE_H__23B42013_9F11_467C_A0F6_F9E647D45CEB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "tenlist.h"
class FexTrackingFeature  
{
public:
	FexTrackingFeature();
	~FexTrackingFeature();

	int				Eigenvalue;
	tenlist<vec2>	Pos;
 
	int				StartTime;

	float			Influence;
};

#endif // !defined(AFX_FEXTRACKINGFEATURE_H__23B42013_9F11_467C_A0F6_F9E647D45CEB__INCLUDED_)
