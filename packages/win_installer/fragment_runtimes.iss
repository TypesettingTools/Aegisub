; This file implements checking for and installing runtime libraries for Aegisub

[Files]
DestDir: {tmp}; Source: "{#DEPS_DIR}\VC_redist\VC_redist.x{#ARCH}.exe"; Flags: nocompression deleteafterinstall

[Run]
Filename: {tmp}\VC_redist.x{#ARCH}.exe; StatusMsg: {cm:InstallRuntime}; Parameters: "/install /quiet /norestart"
