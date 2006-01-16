-- Aegisub Automation demonstration script
-- Original written by Niels Martin Hansen
-- Given into the public domain

name = "Karaoke skeleton demo"
description = "This script demonstrates the use of the karaskel.lua include file, to avoid writing almost identical code for every karaoke effect script."

version, kind, configuration = 3, 'basic_ass', {}

include("karaskel.lua")

function do_syllable(meta, styles, config, line, syl)
	if syl.i == 0 then
		return syl.text
	else
		return string.format("{\\r\\k%d\\t(%d,%d,\\1c&H%s&)}%s", syl.duration, syl.start_time, syl.end_time, line.styleref.color2, syl.text)
	end
end
