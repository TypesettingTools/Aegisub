--[[
 Copyright (c) 2005, Niels Martin Hansen
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   * Neither the name of the Aegisub Group nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 ]]

-- Aegisub Automation include file
-- This file is meant as a support file for the various karaoke skeleton
-- scripts, to avoid including unneeded code

include("utils.lua")

-- karaskel
-- This is a gloabl table defining various parameters, controlling what the
-- skeleton will do. Not all parameters are defined in this base.
karaskel = {
	-- Does this script need positioning information?
	engage_positioning = false,
	-- Syllable precalc parameters
	precalc_start_progress = 0, -- progress precalc starts at
	precalc_end_progress = 50, -- and where it ends at
	-- Does this script use furigana information? (has no effect without positioning)
	engage_furigana = false,
	-- Furigana parameters
	furigana_scale = 0.4, -- relative size of furigana
	-- Default effect-name for inline effects (read manual)
	inline_fx_default = "regular",
	-- Set this to the name of the style used for out-of-line effect specifications, if any (read manual on this!)
	ool_fx_style = false,
	-- Show tracing messages?
	engage_trace = false
}

function karaskel.warning(s)
	aegisub.output_debug("WARNING! " .. s)
end

function karaskel.trace(s)
	if karaskel.engage_trace then
		aegisub.output_debug(s)
	else
		-- A little optimisation
		karaskel.trace = function() end
	end
end

-- precalc_syllable_data
-- Adds various extra fields to the line and syllable tables
-- See the implementation and/or Aegisub help file for more information
function karaskel.precalc_syllable_data(meta, styles, lines)
	karaskel.trace("precalc_syllable_data")
	aegisub.set_status("Preparing syllable-data")
	for i = 0, lines.n-1 do
		karaskel.trace("precalc_syllable_data:2:"..i)
		aegisub.report_progress(karaskel.precalc_start_progress + i/lines.n*(karaskel.precalc_end_progress-karaskel.precalc_start_progress))
		local line, style = lines[i]
		-- Index number of the line
		line.i = i
		-- Linked list-style access
		line.prev = lines[i-1]
		line.next = lines[i+1]
		karaskel.trace("precalc_syllable_data:3:")
		if line.kind == "dialogue" or line.kind == "comment" then
			karaskel.trace("precalc_syllable_data:4:")
			local style = styles[line.style]
			if not style then
				-- ok, so the named style does not exist... well there MUST be at least ONE style
				-- pick the first one
				style = styles[0]
				karaskel.warning(string.format("You have a line using a style named \"%s\", but that style does not exist! Using the first defined style (\"%s\") instead.", line.style, style.name))
			end
			if karaskel.engage_furigana then
				karaskel.split_furigana_data(line)
				line.text_stripped = ""
				for k = 0, line.karaoke.n-1 do
					line.text_stripped = line.text_stripped .. line.karaoke[k].text
				end
			end
			if karaskel.engage_positioning then
				-- Line dimensions
				line.width, line.height, line.ascent, line.extlead = aegisub.text_extents(style, line.text_stripped)
				karaskel.trace("precalc_syllable_data:5:")
				-- Line position
				line.centerleft = math.floor((meta.res_x - line.width) / 2)
				line.centerright = meta.res_x - line.centerleft
			end
			-- Line duration, in miliseconds
			line.duration = (line.end_time - line.start_time) * 10
			-- Style reference
			line.styleref = style
			karaskel.trace("precalc_syllable_data:6:")
			-- Process the syllables
			local curx, curtime = 0, 0
			local inline_fx = karaskel.inline_fx_default
			for j = 0, line.karaoke.n-1 do
				karaskel.trace("precalc_syllable_data:7::"..j)
				local syl = line.karaoke[j]
				-- Syllable index
				syl.i = j
				-- Check for inline-effect
				karaskel.trace("testing for inline_fx in: " .. syl.text)
				local a, b, new_inline_fx = string.find(syl.text, "{%\\?%-(.*)}")
				if new_inline_fx then
					karaskel.trace("caught new inline_fx: " .. new_inline_fx)
					inline_fx = new_inline_fx
				end
				syl.inline_fx = inline_fx
				-- Do positioning calculations, if applicable
				if karaskel.engage_positioning then
					-- Syllable dimensions
					syl.width, syl.height, syl.ascent, syl.extlead = aegisub.text_extents(style, syl.text_stripped)
					karaskel.trace("precalc_syllable_data:8::")
					-- Syllable positioning
					syl.left = curx
					syl.center = math.floor(curx + syl.width/2)
					syl.right = curx + syl.width
					curx = syl.right
					if syl.furigana then
						karaskel.calc_furigana_sizes(line, syl)
					end
				end
				-- Start and end times in miliseconds
				syl.start_time = curtime
				syl.end_time = curtime + syl.duration*10
				curtime = syl.end_time
			end
		end
	end
end

-- Rebuild the entire karaoke data list, splitting out and adding furigana data
-- This also does joining of syllables with "#" as text
-- (Joining of furigana syllables with # as text seems useless for now, so it's not done.)
function karaskel.split_furigana_data(line)
	if line.kind ~= "dialogue" then
		karaskel.trace("skipping line, not dialogue")
		return
	end
	karaskel.trace("split_furigana_data:1")
	line.furigana = {n=0}
	local newkara = {n=0}
	local curtime = 0
	for i = 0, line.karaoke.n-1 do
		karaskel.trace("split_furigana_data:2:"..i)
		local syl = line.karaoke[i]
		local a, b, maintext, furitext = string.find(syl.text_stripped, "^(.*)|(.*)$")
		if not maintext then
			maintext = syl.text_stripped
		end
		karaskel.trace("split_furigana_data:3:"..i..":"..maintext)
		local highlight = { duration = syl.duration, start_time = curtime*10, end_time = curtime*10+syl.duration*10 }
		curtime = curtime + syl.duration
		if maintext == "#" then
			karaskel.trace("split_furigana_data:4:"..i..":a")
			-- repeat character
			newkara[newkara.n-1].duration = newkara[newkara.n-1].duration + syl.duration
			syl = newkara[newkara.n-1]
		else
			karaskel.trace("split_furigana_data:4:"..i..":b")
			syl.furigana = {n=0, text=""} -- essentially a list of syllables in the syllable :o
			syl.highlights = {n=0}
			syl.text_stripped = maintext
			newkara[newkara.n] = syl
			newkara.n = newkara.n + 1
		end
		karaskel.trace("split_furigana_data:5:"..i)
		syl.highlights[syl.highlights.n] = highlight
		syl.highlights.n = syl.highlights.n + 1
		if a then
			karaskel.trace("split_furigana_data:6:"..i)
			local furi = { text = furitext, duration = highlight.duration, start_time = highlight.start_time, end_time = highlight.end_time }
			syl.furigana[syl.furigana.n] = furi
			syl.furigana.n = syl.furigana.n + 1
			syl.furigana.text = syl.furigana.text .. furitext
		end
	end
	line.karaoke = newkara
end

function karaskel.calc_furigana_sizes(line, syl)
	-- assume the sizes for the main syllable itself has been calculated already
	local ls = line.styleref
	local style = { -- only copy what's needed for text_extents
		fontname = ls.fontname,
		fontsize = ls.fontsize * karaskel.furigana_scale,
		bold = ls.bold,
		italic = ls.italic,
		underline = ls.underline,
		strikeout = ls.strikeout,
		scale_x = ls.scale_x,
		scale_y = ls.scale_y,
		spacing = ls.spacing,
		encoding = ls.encoding
	}
	syl.furigana.width, syl.furigana.height = aegisub.text_extents(style, syl.furigana.text)
	syl.furigana.scale = syl.width / syl.furigana.width * 100
	syl.furigana.fontsize = style.fontsize
	if syl.furigana.scale > 100 then
		syl.furigana.scale = 100
	else
		syl.furigana.width = syl.width
	end
	style.scale_x = style.scale_x * syl.furigana.scale / 100
	local left = syl.left + (syl.width - syl.furigana.width)/2
	for i = 0, syl.furigana.n-1 do
		local f = syl.furigana[i]
		f.width, f.height = aegisub.text_extents(style, f.text)
		f.left = left
		f.center = f.left + f.width/2
		f.right = f.left + f.width
		left = f.right
	end
end

-- Everything else is done in the process_lines function
function karaskel.process_lines(meta, styles, lines, config)
	karaskel.trace("new skeleton")
	karaskel.trace("skel_process_lines")
	-- Do a little pre-calculation for each line and syllable
	karaskel.precalc_syllable_data(meta, styles, lines)
	karaskel.trace("skel_process_lines:2")
	-- A var for the new output
	local result = {n=0}
	local ool_fx = {}
	aegisub.set_status("Running main-processing")
	-- Now do the usual processing
	for i = 0, lines.n-1 do
		karaskel.trace("skel_process_lines:3:"..i)
		aegisub.report_progress(50+i/lines.n*50)
		if (lines[i].kind == "dialogue" or lines[i].kind == "comment") and lines[i].style == karaskel.ool_fx_style then
			local fx = {}
			fx.head, fx.tail = string.headtail(lines[i])
			fx.line = lines[i]
			fx.start_time, fx.end_time = fx.line.start_time, fx.line.end_time
			ool_fx[fx.head] = fx
		else
			-- Get replacement lines
			lines[i].ool_fx = ool_fx
			repl = do_line(meta, styles, config, lines[i])
			karaskel.trace("skel_process_lines:4:"..i)
			-- Append to result table
			for j = 1, repl.n do
				table.insert(result, repl[j])
			end
			ool_fx = {}
		end
	end
	-- Done, return the stuff
	return result
end
process_lines = karaskel.process_lines
