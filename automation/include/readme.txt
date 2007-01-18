This directory contains standard include files for Automation.
By default, this directory is included in the Automation Include Path, which
means the files here are always available for use with the include() function.

See the Aegisub help and the individual files in this directory for more
information on their purpose and contents.


DO NOT MODIFY THE CONTENTS OF THIS DIRECTORY OR ADD YOUR OWN FILES,

any files added or modified might be deleted or reverted upon upgrade or
uninstall of Aegisub.

A better solution is to create your own include directory and put it in the
"Automation Include Path" in the Aegisub config.dat file. The path
specifications are separated by pipe characters ("|"). Using this method, you
can also override the default includes with your own, modified versions, by
placing the path to your own versions earlier in the path list.
