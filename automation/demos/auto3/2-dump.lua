-- Aegisub Automation demonstration script
-- Original written by Niels Martin Hansen
-- Given into the public domain

-- For an explanation of these, see the documentation, or demo 1
version = 3
kind = "basic_ass"
name = "Reading data demonstration"
description = "This is a demonstration of how to access the various data passed to an Automation script. It loops over the data structures provided, and dumps them to the debug console."
configuration = {}

include "karaskel-base.auto3"

function process_lines(meta, styles, lines, config)
	karaskel.parse_syllable_data(meta, styles, lines)
	
	-- This makes an alias for the aegisub.output_debug function. Remember that functions are first-class values in Lua.
	-- The 'local' keyword makes the 'out' variable local to this function
	local out = aegisub.output_debug
	
	-- Use the string.format function to form a string showing some metadata about the subtitles and output it
	out(string.format("Metadata: res_x=%d res_s=%d", meta.res_x, meta.res_y))
	
	-- The number of styles is stored in index -1, because there might be a style named "n"
	local numstyles = styles[-1]
	out("Number of styles: " .. numstyles)
	-- Loop over the styles with a for loop, printing some info about each style
	for i = 0, numstyles-1 do
		out(string.format("Style %d: name='%s' fontname='%s'", i, styles[i].name, styles[i].fontname))
	end
	
	-- Do the same, but for the actual subtitle lines
	out("Number of subtitle lines: " .. lines.n)
	for i = 0, lines.n-1 do
		-- Use the report_progress function to show the progress of the loop
		aegisub.report_progress(i/lines.n*100)
		-- Check if the line is a dialogue line, otherwise it won't have much interesting data
		-- Also, the script will crash if you try to access a field that doesn't exist
		if lines[i].kind == "dialogue" then
			-- Again, output some info about the line
			out(string.format("Line %d: dialogue start=%d end=%d style=%s", i, lines[i].start_time, lines[i].end_time, lines[i].style))
			out("   Text: " .. lines[i].text)
			out("   Stripped text: " .. lines[i].text_stripped)
			out("   Number of karaoke syllables: " .. lines[i].karaoke.n)
			-- Also loop over the karaoke syllables, if any, in the line.
			-- Note that there will actually always be at least one syllable, number zero, which is everything before the first \k tag
			for j = 0, lines[i].karaoke.n-1 do
				syl = lines[i].karaoke[j]
				-- Use the aegisub.text_extents function to calculate the rendered size of the syllable text
				-- Note that the styles[lines[i].style] construct can be dangerous, in case the line refers to a non-existant style
				extx, exty, extd, extl = aegisub.text_extents(styles[lines[i].style], syl.text_stripped)
				out(string.format("        Syllable %d: dur=%d kind=%s text='%s' text_stripped='%s' extx=%d exty=%d extd=%d extl=%d", j, syl.duration, syl.kind, syl.text, syl.text_stripped, extx, exty, extd, extl))
			end
		else
			-- For non-dialogue lines, output a lot less info
			out(string.format("Line %d: %s", i, lines[i].kind))
		end
	end
	out("Finished dumping")
	
	-- In the end, no modifications were done, so just return the original subtitle data
	return lines
end
