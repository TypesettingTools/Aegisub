(*
This AppleScript customizes the appearance of the 
disk image in which Inkscape is bundled on MacOS X

 Author:
	Jean-Olivier Irisson <jo.irisson@gmail.com>

 Copyright 2006
 Licensed under GNU General Public License
*)


tell application "Finder"
	tell disk "@PKG_DMG_STRING@"
		open
		tell container window
			set current view to icon view
			set toolbar visible to false
			set statusbar visible to false
			set the bounds to {308, 397, 725, 731}
		end tell
		close
		set opts to the icon view options of container window
		tell opts
			set icon size to 90
			set arrangement to not arranged
		end tell
		set background picture of opts to file ".background:background.png"
		set position of application file "@PKG_BUNDLE_STRING@" to {133, 55}
		set position of alias file "Applications" to {133, 241}
		update without registering applications
		tell container window
			set the bounds to {308, 397, 725, 731}
			set the bounds to {307, 397, 725, 731}
		end tell
		update without registering applications
	end tell
	
	--give the finder some time to write the .DS_Store file
	delay 7
end tell
