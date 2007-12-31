// This file is intended as a template for the TortoiseSVN tool SubWCRev
// Set up your project so it runs a command similar to this as pre-build event:
//"C:\Program Files\TortoiseSVN\bin\SubWCRev.exe" "$(SolutionDir)../aegisub" "$(SolutionDir)../aegisub/build/svn-revision-base.h" "$(SolutionDir)../aegisub/build/svn-revision.h"
// Of course make sure the relative paths etc. match your system

// Do not add or commit the resulting file to SVN.

#define BUILD_SVN_REVISION $WCREV$
#define BUILD_SVN_DATE "$WCDATE$"
#define BUILD_SVN_LOCALMODS $WCMODS?true:false$
