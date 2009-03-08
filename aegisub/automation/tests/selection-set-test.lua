-- Automation 4 test file
-- Create a Macro feature, that displays some text

script_name = "Automation 4 set-selection test"
script_description = "Test setting the grid selection"
script_author = "Niels Martin Hansen"
script_version = "1"


function selecttest(subtitles, selected_lines, active_line)
	-- get line-id of first selected line
	local lid = selected_lines[1]
	-- insert a copy of line 'lid' before itself
	subtitles[-lid] = subtitles[lid]
	-- append a copy of the copy of the copied line
	subtitles[0] = subtitles[lid]
	-- grab the copied line
	local l = subtitles[lid]
	-- modify it
	l.text = "A4 was here!"
	-- and store it back
	subtitles[lid] = l
	-- select some new lines
	selected_lines = { lid-1, lid, lid+1 }
	-- and set undo point (never forget!)
	aegisub.set_undo_point("Insert+select Stuff")
	-- return the new selection
	return selected_lines
end


aegisub.register_macro("Insert+select stuff", "Inserts some lines near the active line and selects the new lines", selecttest, nil)
