--[[ 
"Clean Tags" -- An Auto4 LUA script for cleaning up ASS subtitle lines of badly-formed override 
blocks and redundant/duplicate tags
* Designed to work for Aegisub 2.0 and above (only pre-release version was available at the time of 
writing) @ http://www.malakith.net/aegiwiki
* Requires cleantags.lua to be available in automation's include folder
* The changes performed on your subtitles are guaranteed to be undo-able provided that Aegisub's undo 
mechanism works. Even so, I am not resposible if it damages your subtitles permanently, so please 
back up your subtitle file before applying the cleaning up

Copyright (c) 2007 ai-chan (Aegisub's forum member and registered nick holder of Rizon irc network)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and 
associated documentation files (the "Software"), to deal in the Software without restriction, 
including without limitation the rights to use, copy, modify, merge, publish, distribute, 
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial 
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT 
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES 
OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
]]

script_name = "Clean Tags"
script_description = "Clean subtitle lines by re-arranging ASS tags and override blocks within the lines"
script_author = "ai-chan"
script_version = "1.150"
script_modified = "12 September 2007"

include("cleantags.lua")

function cleantags_subs(subtitles)
	local linescleaned = 0
	for i = 1, #subtitles do
		aegisub.progress.set(i * 100 / #subtitles)
		if subtitles[i].class == "dialogue" and subtitles[i].text ~= "" then
			ntext = cleantags(subtitles[i].text)
			local nline = subtitles[i]
			nline.text = ntext
			subtitles[i] = nline -- I don't understand why we need these steps to incorporate new text
			linescleaned = linescleaned + 1
			aegisub.progress.task(linescleaned .. " lines cleaned")
		end
	end
end

function cleantags_macro(subtitles, selected_lines, active_line)
	cleantags_subs(subtitles)
	aegisub.set_undo_point(script_name)
end

function cleantags_filter(subtitles, config)
	cleantags_subs(subtitles)
end

aegisub.register_macro(script_name, script_description, cleantags_macro)
aegisub.register_filter(script_name, script_description, 0, cleantags_filter)
