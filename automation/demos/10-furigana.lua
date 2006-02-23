-- Aegisub Automation demonstration script
-- Original written by Niels Martin Hansen
-- Given into the public domain

include("karaskel-adv.lua")
karaskel.engage_furigana = true
karaskel.engage_trace = true

version = 3
kind = "basic_ass"
name = "Furigana demo"
description = "Demonstrates how to use the Furigana feature in karaskel-adv"

function do_syllable(meta, styles, config, line, syl)
	local result = {n=0}
	function result.add()
		local l = copy_line(line)
		table.insert(result, l)
		return l
	end
	-- Place the main text
	local l = result.add()
	l.text = string.format("{\\an8\\pos(%d,%d)\\k%d\\kf%d}%s", line.centerleft+syl.center, line.height*1.5, syl.start_time/10, syl.duration, syl.text)
	l.layer = 5
	-- Perform the highlights
	for i = 0, syl.highlights.n-1 do
		local hl = syl.highlights[i]
		l = result.add()
		l.start_time = l.start_time + hl.start_time/10
		l.end_time = l.start_time + 25
		l.text = string.format("{\\an8\\k10000\\k0\\move(%d,%d,%d,%d)\\t(\\1a&HFF&\\2a&HFF&\\3a&HFF&\\4a&HFF&)}%s", line.centerleft+syl.center, line.height*1.5, line.centerleft+syl.center, line.height*3, syl.text_stripped)
		l.layer = 10+i
	end
	-- Place the furigana
	for i = 0, syl.furigana.n-1 do
		local furi = syl.furigana[i]
		l = result.add()
		l.text = string.format("{\\an2\\k%d\\k%d\\bord2\\fscx%.1f\\fs%.1f\\t(%d,%d,\\bord0)\\pos(%d,%d)}%s", furi.start_time/10, furi.duration, syl.furigana.scale, syl.furigana.fontsize, furi.start_time, furi.end_time, line.centerleft+furi.center, line.height*1.5, furi.text)
		l.layer = 3
	end
	return result
end
