i18n = import('i18n')
# This is currently busted on OSX
# and incomplete on any platform.
# It misses translatable strings not directly found in either
# C++ source, desktop or appdata file. This affects strings
# of the Windows installer (iss), from Lua scripts and JSON files.
# Until a solution is found, POT updates should continue to use make_pot.sh.
i18n.gettext('aegisub',
	         args: [
	            '-k_', '-kSTR_MENU', '-kSTR_DISP', '-kSTR_HELP', '-kwxT',
	            '-kfmt_tl', '-kfmt_plural:2,3', '-kwxPLURAL:1,2',
	            '--sort-by-file'
	         ],
	         install_dir: localedir)
