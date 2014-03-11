You can put MSBuild property files in this folder to override the default
build configuration for Aegisub's build system.

A number of sample files are provided. These can be copied and used as
templates for common configuration. Importantly, configuring the location
of several library dependencies.
The copied sample files should be renamed to *.props, otherwise they will
not be found by the build system.

You should not check files placed in this folder into source control.

Files must be named *.props to be found by the build system.
