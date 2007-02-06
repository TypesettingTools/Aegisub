-- Aegisub Automation demonstration script
-- Original written by Niels Martin Hansen
-- Given into the public domain

include("utils.lua")

name = "Text placement demo"
description = "Demonstration of the text_extents function, to do per-syllable placement of text."
version, kind, configuration = 3, 'basic_ass', {}

function process_lines(meta, styles, lines, config)
	-- Prepare local variables
	local output = { n=0 }
	-- Loop through every line
	for i = 0, lines.n-1 do
		aegisub.report_progress(i/lines.n*100)
		-- Only process dialogue lines
		if lines[i].kind ~= "dialogue" then
			output.n = output.n + 1
			output[output.n] = lines[i]
		else
			-- This is just for making the code a bit easier to read
			local line = lines[i]
			-- Get the rendered size of the entire line. (Won't work if there's line breaks in it.)
			local totalx, totaly = aegisub.text_extents(styles[line.style], line.text_stripped)
			-- Calculate where the first syllable should be positioned, if the line is to appear centered on screen
			local curx, cury = (meta.res_x - totalx) / 2, meta.res_y / 2
			-- And more preparations for per-syllable placement
			local startx = curx
			local tstart, tend = 0, 0
			-- Now process each stllable
			for j = 1, line.karaoke.n-1 do
				-- A shortcut variable, and, most important: a copy of the original line
				local syl, syllin = line.karaoke[j], copy_line(line)
				-- Calculate the ending time of this syllable
				tend = tstart + syl.duration*10
				-- Get the rendered size of this syllable
				local extx, exty, extd, extl = aegisub.text_extents(styles[line.style], syl.text_stripped)
				-- Some debug stuff...
				aegisub.output_debug(string.format("text_extents returned: %d, %d, %d, %d", extx, exty, extd, extl));
				-- Replace the text of the copy of the line with this syllable, moving around
				syllin.text = string.format("{\\an4\\move(%d,%d,%d,%d,%d,%d)\\kf%d\\kf%d}%s", curx, cury, curx, cury-exty, tstart, tend, tstart/10, syl.duration, syl.text)
				-- Add the line to the output
				output.n = output.n + 1
				output[output.n] = syllin
				-- And prepare for next iteration
				curx = curx + extx
				tstart = tend
			end
			-- More debug stuff
			aegisub.output_debug(string.format("after syllable loop: totalx=%d curx-startx=%d", totalx, curx-startx))
		end
	end
	-- And remember to return something :)
	return output
end
