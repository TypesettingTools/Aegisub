-- Aegisub Automation demonstration script
-- Original written by Niels Martin Hansen
-- Given into the public domain

-- Furigana is handled by the advanced karaskel
include("karaskel-adv.lua")
-- But it requires some additional functionality to be enabled as well. Do that here
karaskel.engage_furigana = true
-- Enabling trace isn't really required, but it's nice for debugging
karaskel.engage_trace = true

version = 3
kind = "basic_ass"
name = "Furigana demo"
description = "Demonstrates how to use the Furigana feature in karaskel-adv"

function do_syllable(meta, styles, config, line, syl)
	-- First prepare a table to add the resulting lines of this syllable to
	local result = {n=0}
	-- Actually, make a function that does some stuff for us
	-- Calling result.add will create another copy of the current line, add it to the result table, and return a reference to the new line
	-- The result from this function can just be modified, and the line in the result table changes along (since it's just a reference)
	function result.add()
		local l = copy_line(line)
		result.n = result.n + 1
		result[result.n] = l
		return l
	end
	-- Place the main text
	-- So far no magic, just a regular line placed
	local l = result.add()
	l.text = string.format("{\\an8\\pos(%d,%d)\\k%d\\kf%d}%s", line.centerleft+syl.center, line.height*1.5, syl.start_time/10, syl.duration, syl.text_stripped)
	l.layer = 5
	-- Now for some "magic" stuff, loop over the highlights array to add possibly multiple highlights to each syllable
	-- For example, kanji spanning multiple syllables can "flash" multiple times this way
	for i = 0, syl.highlights.n-1 do
		local hl = syl.highlights[i]
		l = result.add()
		l.start_time = l.start_time + hl.start_time/10
		l.end_time = l.start_time + 25
		l.text = string.format("{\\an8\\k10000\\k0\\move(%d,%d,%d,%d)\\t(\\1a&HFF&\\2a&HFF&\\3a&HFF&\\4a&HFF&)}%s", line.centerleft+syl.center, line.height*1.5, line.centerleft+syl.center, line.height*3, syl.text_stripped)
		-- Put each highlight in a layer for itself, because they might overlap. This creates a possibly slightly nicer effect
		l.layer = 10+i
	end
	-- Now for the real magic, ,loop over the furigana array
	-- This contains a lot of pre-calculated info about how each furigana syllable should be placed relatively to the beginning of the line
	-- With this info, it's really not much different from making a "main" effect. You just have to be aware that you're working with much smaller font sizes
	for i = 0, syl.furigana.n-1 do
		local furi = syl.furigana[i]
		l = result.add()
		-- Be sure to include the \fscx%f\fs%f tags here! Otherwise the text will be incorrectly scaled.
		-- The \fscx is also important, because long stretches of furigana might be squished together so it all fits over the main text
		l.text = string.format("{\\an2\\k%d\\k%d\\bord2\\fscx%.1f\\fs%.1f\\t(%d,%d,\\bord0)\\pos(%d,%d)}%s", furi.start_time/10, furi.duration, syl.furigana.scale, syl.furigana.fontsize, furi.start_time, furi.end_time, line.centerleft+furi.center, line.height*1.5, furi.text)
		l.layer = 3
	end
	-- The additional 'add' function in the result is ignored when it's read back by the caller
	return result
end

-- Trick: The script includes itself on apply, meaning it's automatically reloaded
function process_lines(meta, styles, lines, config)
	include("10-furigana.lua")
	-- Make sure to call the karaskel.process_lines function to do the work
	return karaskel.process_lines(meta, styles, lines, config)
end
