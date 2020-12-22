#include "fragment_dictbase.iss"

[Setup]
OutputBaseFilename=Aegisub-3.0-dict-{#LANGCODE}
VersionInfoDescription=Aegisub 3.0 {#LANGNAME} dictionary

[Files]
Source: src\dictionaries\{#LANGCODE}.aff; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/{#LANGCODE}
Source: src\dictionaries\{#LANGCODE}.dic; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: dic/{#LANGCODE}
#ifndef NOTHES
Source: src\dictionaries\th_{#LANGCODE}.dat; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/{#LANGCODE}
Source: src\dictionaries\th_{#LANGCODE}.idx; DestDir: {app}\dictionaries; Flags: ignoreversion; Components: th/{#LANGCODE}
#endif

[Components]
Name: dic;             Description: Spell checker; Types: full
Name: dic/{#LANGCODE}; Description: {#LANGNAME} dictionary; Types: full
#ifndef NOTHES
Name: th;              Description: Thesaurus; Types: full
Name: th/{#LANGCODE};  Description: {#LANGNAME} thesaurus; Types: full
#endif
