-- Automation 4 test file
-- Create a Macro feature, that displays some text

script_name = "Automation 4 test 5"
script_description = "Test include and text_extents"
script_author = "Niels Martin Hansen"
script_version = "1"

include("utils.lua")


function test5(subtitles, selected_lines, active_line)
	local styles = {}
	for i = 1, #subtitles do
		local l = subtitles[i]
		if l.class == "dialogue" then
			break
		end
		if l.class == "style" then
			styles[l.name] = l
		end
	end
	
	for i = #selected_lines, 1, -1 do
		local ri = selected_lines[i]
		local l = subtitles[ri]
		local k = aegisub.parse_karaoke_data(l)
		local left = 0
		for j = 1, #k do
			local nl = table.copy(l)
			l.text = string.format("{\\t(%d,%d,\\fscx50)\\pos(%d,20)}%s", k[j].start_time, k[j].end_time, left, k[j].text_stripped)
			left = left + (aegisub.text_extents(styles[l.style], k[j].text_stripped))
			subtitles.insert(ri+j, l)
		end
	end
	aegisub.set_undo_point("karaoke-like stuff")
end


aegisub.register_macro("More karaoke fun", "Makes some more karaoke-like stuff", test5, nil)
