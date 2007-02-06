-- Aegisub Automation demonstration script
-- Original written by Niels Martin Hansen
-- Given into the public domain

-- Define some required constants
-- version and kind are required to have the given values for the script to work.
-- configuration is not needed in this script, so it's just left as an empty table
version, kind, configuration = 3, "basic_ass", {}

-- Define the displayed name of the script
name = "Simple karaoke effect"
-- A longer description of the script
description = "A simple karaoke effect, with the source code heavily commented. Provided as a starting point for a useful effect."

-- The actual script function
function process_lines(meta, styles, lines, config)
	-- Create a local variable to store the output subtitles in
	local output = { n=0 }
	
	-- Start to loop over the lines, one by one
	-- The lines are numbered 0..n-1
	for i = 0, lines.n-1 do
		-- Show the user how far the script has got
		aegisub.report_progress(i/lines.n*100)
		-- First check if the line is even a dialogue line. If it's not, no need to process it.
		if lines[i].kind ~= "dialogue" then
			output.n = output.n + 1
			output[output.n] = lines[i]
		else
			-- This is a dialogue line, so process is
			-- Make a nicer name for the line we're processing
			newline = lines[i]
			-- Also show the line to the user
			aegisub.set_status(newline.text_stripped)
			
			-- The text of the new line will be build little by little
			-- Each line has 700 ms fadein, 300 ms fadeout,
			-- is positioned at the center of the screen (\an8)
			-- and the highlighting should be delayed by 1000 ms (100 cs)
			newline.text = string.format("{\\fad(700,300)\\pos(%d,30)\\k100}", meta.res_x/2)
			-- Make the line start 1000 ms (100 cs) earlier than original
			newline.start_time = newline.start_time - 100
			
			-- Now it's time to loop through the syllables one by one, processing them
			-- The first syllable is usually a "null" syllable, not containing real data, so that one should be skipped.
			-- This variable is used to keep track of when the last syllable ended
			-- It's initialised to 1000, since the start of the line was pushed 1000 ms back
			local cursylpos = 1000
			for j = 1, lines[i].karaoke.n-1 do
				local syl = lines[i].karaoke[j]
				-- Call another function to process the syllable
				newline.text = newline.text .. doSyllable(syl.text, cursylpos, cursylpos+syl.duration*10, syl.duration, syl.kind)
				-- Calculate the start time of the next syllable
				cursylpos = cursylpos + syl.duration*10
			end
			
			-- The entire line has been calculated
			-- Add it to the output
			output.n = output.n + 1
			output[output.n] = newline
		end
	end
	
	-- All lines processed, and output filled
	-- Just return it
	-- (This is important! If you don't return anything, the output file will be empty!)
	return output
end

-- This effect was originally written in the "Effector" program, which can be considered the first version of Automation.
-- This following function is almost verbatimly copied from that original script.
-- This is done in order to show how you can make sub-functions to make your script more readable.
-- The contents of this function could also just be pasted into the middle of the main loop in process_lines,
-- but that generally makes scripts harder to read.
function doSyllable(text, t_start, t_end, t_dur, ktype)
	-- Declare two local variables needed here
	-- (If they're not declared local, they will be global.)
	local a, b
	-- If it's a "long" syllable, let the effect be different
	if t_dur > 75 then
		a = t_start + 500
		b = t_end
	else
		a = t_start + 100
		b = t_start + 500
	end
	-- Return the replacement for the syllable, including some ASS tags for format it
	return string.format("{\\r\\t(%d,%d,\\1c&H808080&\\2c&H808080&)\\kf%d}%s", a, b, t_dur, text)
end
