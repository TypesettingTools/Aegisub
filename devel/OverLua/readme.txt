OverLua is a plugin library that runs a Lua script to process video frames,
primarily intended to produce advanced overlay graphics. The target audience
is currently primarily programmers.

The name is intended as a pun on Overlay and Lua.

To help produce overlay graphics, OverLua provides an interface to the
cairo vector graphics library. A library of functions to do raster-based
graphics processing is also included.

Curerently the only known-working plugin interface to OverLua is the Avisynth
interface. A CSRI interface is also included but it's neither tested nor
actively maintained. Patches to make the CSRI interface work are most welcome.
Further interfaces would also be appreciated, eg. one for mencoder.

The most thorough documentation is still the source code but I'm working on
throwing together something more useful. See the other text files included
in the distribution.


Notes on the pre-built DLL file
-------------------------------

The included pre-built DLL is compiled with Microsoft Visual C++ 2005 SP1
and so will require the runtime library for that version of the compiler.
You can download the required runtime library at this location:

<http://www.microsoft.com/downloads/details.aspx?FamilyID=200b2fd9-ae1a-4a14-984d-389c36f85647&DisplayLang=en>

The DLL is no longer built with SSE2. Appanrently some people still use
CPU's without SSE2 support.

Finally, the DLL is built with OpenMP optimisations enabled, which means it
will take advantage of multi-core and other SMP systems if available. The
OpenMP optimisations are only in the raster image filtering and the
cairo surface/video frame interaction functions. If you do not use the raster
graphics operations much you won't see much gain from this SMP support either.


Licensing
---------

OverLua is licensed under version 2 of the GNU General Public License. This
means that you get no warranties of any kind.
See the file GPLv2.txt for details.
