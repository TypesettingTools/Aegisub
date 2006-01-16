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
-- The purpose of this include file is to provide a default skeleton for Automation scripts,
-- where you, as a scripter, can just override the functions you need to override.

-- The following functions can be overridden:
--   do_syllable - called for each syllable in a line
--   do_line_decide - called to decide whether to process a line or not
--   do_line_start - called at the beginning of each line
--   do_line_end - called at the end of each line
--   do_line - Process an entire line
-- By default, these functions are all assigned to the following default functions
-- Your functions should obviously use the same parameter lists etc.
-- Note that if you override do_line, none of the other functions will be called!


-- All the functions mentioned here are, however, wrapped in some code that pre-calculates some
-- extra data for the lines and syllables.

-- The following fields are added to lines:
--   i - The index of the line in the file, 0 being the first
--   width - Rendered width of the line in pixels (using the line style, overrides are not taken into account)
--   height - Rendered height of the line in pixels (note that rotation isn't taken into account either)
--   ascent - Height of the ascenders, in pixels (notice that this looks like the output of text_extents?)
--   extlead - External leading of the font, in pixels (well, it is!)
--   centerleft - Pixel position of the left edge of the line, when horisontally centered on screen
--   centerright - Pixel pisition of the right edge of the line, when horisontally centered on screen
--   duration - Duration of the line in miliseconds
--   styleref - The actual style table for the style this line has

-- The following fields are added to syllables:
--   i - The index of the syllable in the line, 0 being the first (and usually empty) one
--   width, height, ascent, extlead - Same as for lines
--   left - Left pixel position of the syllable, relative to the start of the line
--   center - Center pixel position of the syllable, also relative to the start of the line
--   right - Right pixel position of the syllable, relative again
--   start_time - Start time of the syllable, in miliseconds, relative to the start of the line
--   end_time - End time of the syllable, similar to start_time


-- Return a replacement text for a syllable
function default_do_syllable(meta, styles, config, line, syl)
	return syl.text
end

-- Decide whether or not to process a line
function default_do_line_decide(meta, styles, config, line)
	return line.kind == "dialogue"
end

-- Return a text to prefix the line
function default_do_line_start(meta, styles, config, line)
	return ""
end

-- Return a text to suffix the line
function default_do_line_end(meta, styles, config, line)
	return ""
end

-- Process an entire line (which has pre-calculated extra data in it already)
-- Return a table of replacement lines
function default_do_line(meta, styles, config, line)
	-- Check if the line should be processed at all
	if not do_line_decide(meta, styles, config, line) then
		return {n=0}
	end
	-- Create a new local var for the line replacement text, set it to line prefix
	-- This is to make sure the actual line text isn't replaced before the line has been completely processed
	local newtext = do_line_start(meta, styles, config, line)
	-- Loop over the syllables
	for i = 0, line.karaoke.n-1 do
		-- Append the replacement for each syllable onto the line
		newtext = newtext .. do_syllable(meta, styles, config, line, line.karaoke[i])
	end
	-- Append line suffix
	newtext = newtext .. do_line_end(meta, styles, config, line)
	-- Now replace the line text
	line.text = newtext
	-- And return a table with one entry
	return {n=1; [1]=line}
end

-- Now assign all the default functions to the names that are actually called
do_syllable = default_do_syllable
do_line_decide = default_do_line_decide
do_line_start = default_do_line_start
do_line_end = default_do_line_end
do_line = default_do_line

precalc_start_progress = 0
precalc_end_progress = 50
function precalc_syllable_data(meta, styles, lines)
	aegisub.set_status("Preparing syllable-data")
	for i = 0, lines.n-1 do
		aegisub.report_progress(precalc_start_progress + i/lines.n*(precalc_end_progress-precalc_start_progress))
		local line, style = lines[i]
		-- Index number of the line
		line.i = i
		if line.kind == "dialogue" or line.kind == "comment" then
			local style = styles[line.style]
			-- Line dimensions
			line.width, line.height, line.ascent, line.extlead = aegisub.text_extents(style, line.text_stripped)
			-- Line position
			line.centerleft = math.floor((meta.res_x - line.width) / 2)
			line.centerright = meta.res_x - line.centerleft
			-- Line duration, in miliseconds
			line.duration = (line.end_time - line.start_time) * 10
			-- Style reference
			line.styleref = style
			-- Process the syllables
			local curx, curtime = 0, 0
			for j = 0, line.karaoke.n-1 do
				local syl = line.karaoke[j]
				-- Syllable index
				syl.i = j
				-- Syllable dimensions
				syl.width, syl.height, syl.ascent, syl.extlead = aegisub.text_extents(style, syl.text_stripped)
				-- Syllable positioning
				syl.left = curx
				syl.center = math.floor(curx + syl.width/2)
				syl.right = curx + syl.width
				curx = syl.right
				-- Start and end times in miliseconds
				syl.start_time = curtime
				syl.end_time = curtime + syl.duration*10
				curtime = syl.end_time
			end
		end
	end
end

-- Everything else is done in the process_lines function
function skel_process_lines(meta, styles, lines, config)
	-- Do a little pre-calculation for each line and syllable
	precalc_syllable_data(meta, styles, lines)
	-- A var for the new output
	local result = {n=0}
	aegisub.set_status("Running main-processing")
	-- Now do the usual processing
	for i = 0, lines.n-1 do
		aegisub.report_progress(50+i/lines.n*50)
		if do_line_decide(meta, styles, config, lines[i]) then
			-- Get replacement lines
			repl = do_line(meta, styles, config, lines[i])
			-- Append to result table
			for j = 1, repl.n do
				table.insert(result, repl[j])
			end
		end
	end
	-- Done, return the stuff
	return result
end
process_lines = skel_process_lines
