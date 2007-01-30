// This file is part of FexTracker and (C) 2006 by Hajo Krabbenhöft  (tentacle)
// All rights reserved but the aegisub project is allowed to use it.

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__7E36ECD6_3192_4C7E_88C9_742CCCFA5057__INCLUDED_)
#define AFX_STDAFX_H__7E36ECD6_3192_4C7E_88C9_742CCCFA5057__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// Insert your headers here
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#ifdef WIN32
#include <windows.h>
#endif
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <assert.h>

#include "FexTracker.h"
#include "FexImgPyramid.h"
#include "FexTrackingFeature.h"
#include "FexMovement.h"

#ifdef IMAGE_DEBUGGER
#include "ext/imdebug.h"
#else
#define imdebug(a,b,c,d) // 
#endif

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__7E36ECD6_3192_4C7E_88C9_742CCCFA5057__INCLUDED_)
