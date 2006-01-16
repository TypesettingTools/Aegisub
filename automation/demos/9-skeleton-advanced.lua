-- Aegisub Automation demonstration script
-- Original written by Niels Martin Hansen
-- Given into the public domain

name = "Advanced skeleton demo"
description = "This script demonstrates using the karaskel-adv.lua include file to make rceation of per-syllable positioning effects easier."

version, kind, configuration = 3, 'basic_ass', {}

include("karaskel-adv.lua")

-- What kind of effect this makes:
-- Each syllable "jumps" up and down once during its duration
-- This is achieved using two \move operations, and as known, gabest's TextSub can only handle one \move per line
-- So we need two lines per syllable, split exactly in the middle of the duration of the syllable

function do_syllable(meta, styles, config, line, syl)
	-- Make two copies of the original line (having the right timings etc)
	local half1, half2 = copy_line(line), copy_line(line)
	-- Make the first half line end halfway into the duration of the syllable
	half1.end_time = half1.start_time + syl.start_time/10 + syl.duration/2
	-- And make the second half line start where the first one ends
	half2.start_time = half1.end_time
	-- Where to move the syllable to/from
	local fromx, fromy = line.centerleft+syl.center, line.height*2 + 20
	local tox, toy = fromx, fromy - 10
	-- Generate some text for the syllable
	half1.text = string.format("{\\an8\\move(%d,%d,%d,%d,%d,%d)}%s", fromx, fromy, tox, toy, syl.start_time, syl.start_time+syl.duration*5, syl.text_stripped)
	half2.text = string.format("{\\an8\\move(%d,%d,%d,%d,%d,%d)}%s", tox, toy, fromx, fromy, 0, syl.duration*5, syl.text_stripped)
	-- Things will look bad with overlapping borders and stuff unless
	-- we manually layer borders lower than text,
	-- and shadows lower than borders, so let's do that
	local half1b, half1s = copy_line(half1), copy_line(half1)
	half1b.text = "{\\1a&HFF&\\shad0}" .. half1b.text
	half1s.text = "{\\1a&HFF&\\bord0}" .. half1s.text
	half1.text = "{\\bord0\\shad0}" .. half1.text
	half1.layer = 2
	half1b.layer = 1
	half1s.layer = 0
	local half2b, half2s = copy_line(half2), copy_line(half2)
	half2b.text = "{\\1a&HFF&\\shad0}" .. half2b.text
	half2s.text = "{\\1a&HFF&\\bord0\\shad2}" .. half2s.text
	half2.text = "{\\bord0\\shad0}" .. half2.text
	half2.layer = 2
	half2b.layer = 1
	half2s.layer = 0
	-- Done, return the two new lines
	return {n=6, [1]=half1, [2]=half2b, [3]=half1s, [4]=half2, [5]=half2b, [6]=half2s}
end
