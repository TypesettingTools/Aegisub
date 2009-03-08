/*
Dummy library created for the Aegisub 2 installer.
Exports a single function only for the purpose of testing
whether the DLL can load on the system, ie. whether
the required runtime libraries are installed.

There are no restrictions on the use of this library
for any purposes whatsoever, and likewise are
no warranties given for it whatsoever.
*/

#include <stdlib.h>

extern "C" __declspec(dllexport) void __stdcall TestFunction()
{
	void *x = malloc(16);
	free(x);
}
