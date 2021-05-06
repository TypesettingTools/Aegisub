; This file declares everything related to installable translations of Aegisub

[Files]
; localization (commented out ones are out of date; some don't have wxstd.mo)
#ifdef ENABLE_TRANSLATIONS
Source: {#BUILD_ROOT}\po\ar.gmo;          DestDir: {app}\locale\ar;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-ar.mo;      DestDir: {app}\locale\ar;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\bg.gmo;          DestDir: {app}\locale\bg;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
; Missing wxstd for Bulgarian
Source: {#BUILD_ROOT}\po\ca.gmo;          DestDir: {app}\locale\ca;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-ca.mo;      DestDir: {app}\locale\ca;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\cs.gmo;          DestDir: {app}\locale\cs;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-cs.mo;      DestDir: {app}\locale\cs;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\da.gmo;          DestDir: {app}\locale\da;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-da.mo;      DestDir: {app}\locale\da;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\de.gmo;          DestDir: {app}\locale\de;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-de.mo;      DestDir: {app}\locale\de;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\el.gmo;          DestDir: {app}\locale\el;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-el.mo;      DestDir: {app}\locale\el;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\es.gmo;          DestDir: {app}\locale\es;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-es.mo;      DestDir: {app}\locale\es;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\eu.gmo;          DestDir: {app}\locale\eu;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-eu.mo;      DestDir: {app}\locale\eu;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\fa.gmo;          DestDir: {app}\locale\fa;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
; Farsi wxstd missing
;Source: src\mo\wxstd-fa.mo;     DestDir: {app}\locale\fa;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\fi.gmo;          DestDir: {app}\locale\fi;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-fi.mo;      DestDir: {app}\locale\fi;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\fr_FR.gmo;       DestDir: {app}\locale\fr_FR; DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-fr.mo;      DestDir: {app}\locale\fr_FR; DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\gl.gmo;          DestDir: {app}\locale\gl;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-gl_ES.mo;   DestDir: {app}\locale\gl;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\hu.gmo;          DestDir: {app}\locale\hu;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-hu.mo;      DestDir: {app}\locale\hu;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\id.gmo;          DestDir: {app}\locale\id;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-id.mo;      DestDir: {app}\locale\id;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\it.gmo;          DestDir: {app}\locale\it;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-it.mo;      DestDir: {app}\locale\it;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\ja.gmo;          DestDir: {app}\locale\ja;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-ja.mo;      DestDir: {app}\locale\ja;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\ko.gmo;          DestDir: {app}\locale\ko;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-ko_KR.mo;   DestDir: {app}\locale\ko;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\nl.gmo;          DestDir: {app}\locale\nl;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-nl.mo;      DestDir: {app}\locale\nl;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\pl.gmo;          DestDir: {app}\locale\pl;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-pl.mo;      DestDir: {app}\locale\pl;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\pt_BR.gmo;       DestDir: {app}\locale\pt_BR; DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-pt_BR.mo;   DestDir: {app}\locale\pt_BR; DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\pt_PT.gmo;       DestDir: {app}\locale\pt_PT; DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-pt.mo;      DestDir: {app}\locale\pt_PT; DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\ru.gmo;          DestDir: {app}\locale\ru;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-ru.mo;      DestDir: {app}\locale\ru;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\sr_RS.gmo;       DestDir: {app}\locale\sr_RS; DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\sr_RS@latin.gmo; DestDir: {app}\locale\sr_RS@latin; DestName: aegisub.mo; Flags: ignoreversion; Components: translations
; Missing wxstd for Serbian
Source: {#BUILD_ROOT}\po\uk_UA.gmo;       DestDir: {app}\locale\uk_UA; DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-uk_UA.mo;   DestDir: {app}\locale\uk_UA; DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\vi.gmo;          DestDir: {app}\locale\vi;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-vi.mo;      DestDir: {app}\locale\vi;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\zh_CN.gmo;       DestDir: {app}\locale\zh_CN; DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-zh_CN.mo;   DestDir: {app}\locale\zh_CN; DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\zh_TW.gmo;       DestDir: {app}\locale\zh_TW; DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-zh_TW.mo;   DestDir: {app}\locale\zh_TW; DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
#endif
