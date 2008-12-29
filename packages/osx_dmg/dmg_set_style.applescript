(*
This AppleScript customizes the appearance of the 
disk image in which Inkscape is bundled on MacOS X

 Author:
	Jean-Olivier Irisson <jo.irisson@gmail.com>

 Copyright 2006
 Licensed under GNU General Public License
*)


tell application "Finder"
	tell disk "@PKG_NAME_VOLUME@"
		open
		tell container window
			set current view to icon view
			set toolbar visible to false
			set statusbar visible to false
			set the bounds to {250, 250, 700, 600}
		end tell
		close
		set opts to the icon view options of container window
		tell opts
			set icon size to 90
			set arrangement to not arranged
		end tell
		set background picture of opts to file ".background:background.png"
		set position of application file "@PKG_DIR@" to {133, 55}
		set position of alias file "Applications" to {133, 250}
		update without registering applications
		tell container window
			set the bounds to {250, 250, 700, 600}
			set the bounds to {249, 250, 700, 600}
		end tell
		update without registering applications
	end tell
	
	--give the finder some time to write the .DS_Store file
	delay 7
end tell
