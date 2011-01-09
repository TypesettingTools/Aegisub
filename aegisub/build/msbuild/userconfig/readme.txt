You can put MSBuild property files in this folder to override the default
build configuration for Aegisub's build system.

There is one configuration you almost guaranteed will want to override,
this is for the location of wxWidgets. Here is an example of a property
file that specifies a wxWidgets location:

-------------- mywx.props --------------
<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup>
	<!-- Set a helper property, to make the remaining property definitions shorter.
	     The WxBasePath property is not used by Aegisub's build system. -->
  	<WxBasePath>G:\Dev\wxWidgets-2.9\install-vc10</WxBasePath>
	<!-- Specify WxLibraryPath, the location where the link libraries for wxWidgets
	     are located. The location depends on the Platform variable.
	     These library paths are also used to deduce the location of the configuration
	     header file for the specific builds. -->
  	<WxLibraryPath Condition="'$(Platform)'=='Win32'">$(WxBasePath)\lib32</WxLibraryPath>
  	<WxLibraryPath Condition="'$(Platform)'=='x64'">$(WxBasePath)\lib64</WxLibraryPath>
	<!-- Specify where the wxWidgets include files are found. -->
  	<WxIncludePath>$(WxBasePath)\include</WxIncludePath>
  </PropertyGroup>
</Project>
----------------------------------------

You should not check files placed in this folder into source control.

Files must be named *.props to be used.


Other properties of interest:

Ft2IncludePath	- Path to Freetype2 include files
Ft2LibraryPath	- Path to Freetype2 lib files
Ft2LibraryName	- Name of Freetype2 library to link


These may be useful to override in some cases, but the defaults should be fine for most:

AegisubOutputBase	- Base directory for output files from compilation, defaults to solution file location
AegisubBinaryDir	- Where executables are placed, derived from AegisubOutputBase by default
AegisubLibraryDir	- Where lib files are placed, derived from AegisubOutputBase by default
AegisubObjectDir	- Where intermediary files are placed, derived from AegisubOutputBase by default
