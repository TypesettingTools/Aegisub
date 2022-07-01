; This file declares everything related to installable translations of Aegisub

[Files]
; Aegisub localization
#ifdef ENABLE_AEG_TRANSLATIONS
Source: {#BUILD_ROOT}\po\ar\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\ar;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\bg\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\bg;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\ca\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\ca;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\cs\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\cs;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\da\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\da;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\de\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\de;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\el\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\el;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\es\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\es;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\eu\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\eu;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\fa\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\fa;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\fr_FR\LC_MESSAGES\aegisub.mo;       DestDir: {app}\locale\fr_FR; DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\gl\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\gl;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\hu\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\hu;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\id\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\id;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\it\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\it;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\ja\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\ja;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\ko\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\ko;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\nl\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\nl;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\pl\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\pl;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\pt_BR\LC_MESSAGES\aegisub.mo;       DestDir: {app}\locale\pt_BR; DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\pt_PT\LC_MESSAGES\aegisub.mo;       DestDir: {app}\locale\pt_PT; DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\ru\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\ru;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\sr_RS\LC_MESSAGES\aegisub.mo;       DestDir: {app}\locale\sr_RS; DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\sr_RS@latin\LC_MESSAGES\aegisub.mo; DestDir: {app}\locale\sr_RS@latin; DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\uk_UA\LC_MESSAGES\aegisub.mo;       DestDir: {app}\locale\uk_UA; DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\vi\LC_MESSAGES\aegisub.mo;          DestDir: {app}\locale\vi;    DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\zh_CN\LC_MESSAGES\aegisub.mo;       DestDir: {app}\locale\zh_CN; DestName: aegisub.mo; Flags: ignoreversion; Components: translations
Source: {#BUILD_ROOT}\po\zh_TW\LC_MESSAGES\aegisub.mo;       DestDir: {app}\locale\zh_TW; DestName: aegisub.mo; Flags: ignoreversion; Components: translations
#endif
; END ENABLE_TRANSLATIONS

;; TODO: rm those lines
;;  xref: [Update and review translations · Issue #132 · TypesettingTools/Aegisub](https://github.com/TypesettingTools/Aegisub/issues/132)
#ifdef ENABLE_WX_TRANSLATIONS
; wxWidgets localization (commented out ones are out of date; some don't have wxstd.mo)
Source: src\mo\wxstd-ar.mo;      DestDir: {app}\locale\ar;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
; Missing wxstd for Bulgarian
Source: src\mo\wxstd-ca.mo;      DestDir: {app}\locale\ca;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-cs.mo;      DestDir: {app}\locale\cs;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-da.mo;      DestDir: {app}\locale\da;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-de.mo;      DestDir: {app}\locale\de;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-el.mo;      DestDir: {app}\locale\el;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-es.mo;      DestDir: {app}\locale\es;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-eu.mo;      DestDir: {app}\locale\eu;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
; Farsi wxstd missing
;Source: src\mo\wxstd-fa.mo;     DestDir: {app}\locale\fa;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-fi.mo;      DestDir: {app}\locale\fi;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-fr.mo;      DestDir: {app}\locale\fr_FR; DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-gl_ES.mo;   DestDir: {app}\locale\gl;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-hu.mo;      DestDir: {app}\locale\hu;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-id.mo;      DestDir: {app}\locale\id;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-it.mo;      DestDir: {app}\locale\it;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-ja.mo;      DestDir: {app}\locale\ja;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-ko_KR.mo;   DestDir: {app}\locale\ko;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-nl.mo;      DestDir: {app}\locale\nl;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-pl.mo;      DestDir: {app}\locale\pl;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-pt_BR.mo;   DestDir: {app}\locale\pt_BR; DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-pt.mo;      DestDir: {app}\locale\pt_PT; DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-ru.mo;      DestDir: {app}\locale\ru;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
; Missing wxstd for Serbian
Source: src\mo\wxstd-uk_UA.mo;   DestDir: {app}\locale\uk_UA; DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-vi.mo;      DestDir: {app}\locale\vi;    DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-zh_CN.mo;   DestDir: {app}\locale\zh_CN; DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
Source: src\mo\wxstd-zh_TW.mo;   DestDir: {app}\locale\zh_TW; DestName: wxstd.mo;   Flags: ignoreversion; Components: translations
#endif 
; END ENABLE_WX_TRANSLATIONS
