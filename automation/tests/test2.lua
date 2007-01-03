-- Automation 4 test file
-- Create a Macro feature, that displays some text

script_name = "Automation 4 test 2"
script_description = "Some additional non-hello-world tests"
script_author = "Niels Martin Hansen"
script_version = "2"


function macro_test2(subtitles, selected_lines, active_line)
	aegisub.debug.out(subtitles.n .. " and " .. #subtitles .. " should be the same value")
	aegisub.debug.out(subtitles[selected_lines[1]].raw)
end

function dumper(subtitles, selected_lines, active_line)
	for i = 1, #subtitles do
		local l = subtitles[i]
		local s = l.raw .. "\n"
		s = s .. l.class .. "\n"
		if l.class == "comment" then
			s = s .. "text: " .. l.text .. "\n"
		elseif l.class == "info" then
			s = s .. string.format("key: %s\nvalue:%s\n", l.key, l.value)
		elseif l.class == "format" then
			-- skip
		elseif l.class == "style" then
			s = s .. string.format("name: %s\nfont: %s %d\ncolors: %s %s %s %s\n", l.name, l.fontname, l.fontsize, l.color1, l.color2, l.color3, l.color4)
		elseif l.class == "dialogue" then
			s = s .. string.format("layer: %d\nstyle: %s\ntext: %s\n", l.layer, l.style, l.text)
		end
		aegisub.debug.out(s)
	end
end

function inserttest(subtitles, selected_lines, active_line)
	local lid = selected_lines[1]
	subtitles[-lid] = subtitles[lid]
	subtitles[0] = subtitles[lid]
	local l = subtitles[lid]
	l.text = "A4 was here!"
	subtitles[lid] = l
	aegisub.set_undo_point("Insert Stuff")
end


aegisub.register_macro("File line count", "Count the number of lines in the ASS file", macro_test2, nil)

aegisub.register_macro("Dump", "Dumps info on every line in the file", dumper, nil)

aegisub.register_macro("Insert stuff", "Inserts some lines near the active line", inserttest, nil)
