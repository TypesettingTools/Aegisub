-- Automation 4 test file
-- Create a Macro feature, that displays some text

script_name = "Automation 4 test 3"
script_description = "Test parse_karaoke_data"
script_author = "Niels Martin Hansen"
script_version = "1"


function copy_table(tab)
	local res = {}
	for key, val in pairs(tab) do
		res[key] = val
	end
	return res
end

function karatest(subtitles, selected_lines, active_line)
	for i = #selected_lines, 1, -1 do
		local ri = selected_lines[i]
		local l = subtitles[ri]
		local k = aegisub.parse_karaoke_data(l)
		for j = 1, #k do
			local nl = copy_table(l)
			l.text = string.format("{\\t(%d,%d,\\fscx50)}%s", k[j].start_time, k[j].end_time, k[j].text_stripped)
			subtitles.insert(ri+j, l)
		end
	end
	aegisub.set_undo_point("karaoke-like stuff")
end


aegisub.register_macro("Karaoke fun", "Makes karaoke-like stuff", karatest, nil)
