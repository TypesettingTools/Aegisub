-- Aegisub Automation demonstration script
-- Original written by Niels Martin Hansen
-- Given into the public domain

version = 3
kind = "basic_ass"
name = "Reading data demonstration"
description = "This is a demonstration of how to access the various data passed to an Automation script. It loops over the data structures provided, and dumps them to the debug console."
configuration = {}

function process_lines(meta, styles, lines, config)
	out = aegisub.output_debug
	
	out(string.format("Metadata: res_x=%d res_s=%d", meta.res_x, meta.res_y))
	
	numstyles = styles[-1]
	out("Number of styles: " .. numstyles)
	for i = 0, numstyles-1 do
		out(string.format("Style %d: name='%s' fontname='%s'", i, styles[i].name, styles[i].fontname))
	end
	
	out("Number of subtitle lines: " .. lines.n)
	for i = 0, lines.n-1 do
		aegisub.report_progress(i/lines.n*100)
		if lines[i].kind == "dialogue" then
			out(string.format("Line %d: dialogue start=%d end=%d style=%s", i, lines[i].start_time, lines[i].end_time, lines[i].style))
			out("   Text: " .. lines[i].text)
			out("   Stripped text: " .. lines[i].text_stripped)
			out("   Number of karaoke syllables: " .. lines[i].karaoke.n)
			for j = 0, lines[i].karaoke.n-1 do
				syl = lines[i].karaoke[j]
				extx, exty, extd, extl = aegisub.text_extents(styles[lines[i].style], syl.text_stripped)
				out(string.format("        Syllable %d: dur=%d kind=%s text='%s' text_stripped='%s' extx=%d exty=%d extd=%d extl=%d", j, syl.duration, syl.kind, syl.text, syl.text_stripped, extx, exty, extd, extl))
				--out(string.format("        Syllable %d: kind=%s", j, syl.kind))
			end
		else
			out(string.format("Line %d: %s", i, lines[i].kind))
		end
	end
	
	-- but really just do nothing
	return lines
end
