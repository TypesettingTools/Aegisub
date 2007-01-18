-- Aegisub Automation demonstration script
-- Original written by Niels Martin Hansen
-- Given into the public domain

name = "Karaoke skeleton demo"
description = "This script demonstrates the use of the karaskel.lua include file, to avoid writing almost identical code for every karaoke effect script."
version, kind, configuration = 3, 'basic_ass', {}

-- Include the "magic" karaskel.lua file. It also includes utils.lua for you.
-- karaskel.lua defines the process_lines function, so you'll usually not have to write that yourself
include("karaskel.lua")

-- Instead, a do_syllable function is written. This is called by karaskel for every syllable, to get the text replacement for that syllable.
function do_syllable(meta, styles, config, line, syl)
	-- First check if it's the first syllable on the line; if it is don't bother doing anything special
	if syl.i == 0 then
		-- Remember you have to return the text of the syllable as well as any formatting tags you want
		return syl.text
	else
		-- For other syllables, apply a little effect
		return string.format("{\\r\\k%d\\t(%d,%d,\\1c&H%s&)}%s", syl.duration, syl.start_time, syl.end_time, line.styleref.color2, syl.text)
	end
end

-- Exercise for the reader: Rewrite this script in "classic style", ie. the same way as demo 6.
-- See how few lines you can get it down to, while still producing the same effect.
