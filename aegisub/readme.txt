




           THIS FILE IS OUTDATED, IGNORE IT
           FOR INSTRUCTIONS:
           http://www.malakith.net/aegiwiki/index.php?title=Instructions






Aegisub Source Code Documentation


Some thoughts:

First, some of the code is pretty readable, some is decent, and some is patched up crap. Good luck. ;)
Second, if you want to code anything for Aegisub, you will need to agree to these terms:
  1. You will release the patch to the public domain or give its copyright to one of the developers. This is to stop a source file from being owned by too many people. (Exception: MAJOR changes might be accepted under BSD license under your name. Consult the developers)
  2. Make SURE it compiles and works fine before submitting to developers.
  3. Stick to the coding standards. That is, no GNU-style identing and crap.
Third, this is all available under the BSD license. According to GNU itself, BSD is GPL-compatible, meaning that you can link GPL code to BSD code. Keep in mind, though, that if a source file has mixed BSD and GPL content, it becomes ruled by GPL.


Some notes about the procedure:
All paths should be added to the global msvc settings or you might encounter some problems.
This guide assumes Visual Studio 2003 or 2005 is used, it might work in other versions but it
is guaranteed to fail with other compilers due to the avisynth dependency.
While you can compile most libraries yourself these instructions will assume you want to
download precompiled versions whenever possible. Also note that in  most other cases where
precompiled libraries are available they're unsuitable for use in aegisub.

Building instructions (dependencies):

1. Download and install wxWidgets 2.8.x (www.wxwidgets.org, 2.8.0 used when this guide was written).
Open build\msw\wx.sln, and set the active configuration to "Unicode Debug". Compile that. After that, set it to
"Unicode Release", and also compile that.

Include:
  include
  lib\vc_lib\mswu
Libraries:
  lib\vc_lib

2. Download and compile freetype2 (www.freetype.org). The easiest way to do this is to open the solution found in
builds\win32\visualc\freetype.sln and then batch build all configurations.

Include:
  include
Libraries:
  objs

3. Download and compile lua (www.lua.org). This easiest done by copying the contents of include, src/lib and src/lua
to src (overwriting makefiles makes no difference here). Create a new win32 c++ project, select static library,
blank project and uncheck precompiled headers. Add all h and c files in src to the project and set code generation
to multi threaded (debug) dll in the configurations, set the output names to lua50MT(d).lib in the lib directory.
Batch compile.

Include:
  include
Libraries:
  lib

4. Download and compile portaudio (www.portaudio.com). If you want to compile the directsound version
you need to have the directx9 sdk properly set up before continuing. This process is very similar to lua. First copy
the contents of pa_common to the pa_win_wmme and pa_win_ds dirs. Create a blank project and add all files in pa_win_ds
(for directshow) or in pa_win_wmme (for wmme audio output) to the project. Set the library name to PAStaticDSMT(d).lib
and the output path to lib. Set the code generation to multi threaded (debug) dll and batch compile.

Include:
  pa_common
Libraries:
  lib

5. OPTIONAL - You can safely skip this step if you don't intend to work on the aspell part of aegisub. It's disabled in
the release builds. Download precompiled libs for aspell (aspell.net/win32/) the current direct link is
http://ftp.gnu.org/gnu/aspell/w32/aspell-dev-0-50-3-3.zip.

Include:
  include
Libraries:
  lib
  
6. Install a Python interpreter if you don't have one already, you will NEED it to run a script during the
build process.
http://www.python.org/download

Building instructions (aegisub):

1. Create a new blank Win32 c++ project in msvc.

2. Copy setup0.h as setup.h, and edit any possible configurations you might want there. Make sure that you SVN ignore that
file (setup.h), as you are NOT supposed to commit it to the repository.

3. Add all h, cpp and rc files in the "core", "PRS" and "FexTrackerSource" folders to the project.

4. Open the project settings. Add UNICODE to the preprocessor defines, and set the code generation to multi threaded (debug) dll.

5. Set up the build-versioning stuff.

5.1. Open projects settings and select All Configurations. Go to Build Events, Pre-build Event.
Change the Command Line to the following two lines (click "..."):
  cd $(InputDir)\core\build
  c:\python24\python.exe make-svn-rev-header.py
You'll obviously want to change the path to the Python interpreter.

5.2 Create the file core/build/build-credit.h and add this line to it:
  #define BUILD_CREDIT "yournick"
Of course without indendation and replacing the yournick part.

6. Add the libraries to the linker input. If you compiled portaudio with wmme you have to remove dsound.lib and srmiids.lib
and replace PAStaticDS*.lib with the wmme version. Remove libaspell-15-dll.lib if you defined NO_SPELLCHECKER.

Link to these libraries for release:
freetype2110MT.lib libaspell-15-dll.lib wxzlib.lib wxpng.lib wxregexu.lib wxmsw28u_adv.lib wxmsw28u_core.lib wxbase28u.lib wxmsw28u_media.lib dsound.lib PAStaticDSMT.lib Vfw32.lib winmm.lib lua50MT.lib comctl32.lib rpcrt4.lib advapi32.lib wsock32.lib strmiids.lib

Link to these libraries for debug:
freetype2110MT_D.lib libaspell-15-dll.lib dsound.lib PAStaticDSMTd.lib Vfw32.lib lua50MTd.lib wxzlibd.lib wxpngd.lib wxregexud.lib wxbase28ud.lib wxmsw28ud_media.lib wxmsw28ud_core.lib wxmsw28ud_adv.lib comctl32.lib rpcrt4.lib winmm.lib advapi32.lib wsock32.lib strmiids.lib

6. Compile and wait.