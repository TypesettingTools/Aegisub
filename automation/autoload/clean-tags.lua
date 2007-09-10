script_name = "Clean Tags"
script_description = "Clean subtitle lines by re-arranging ASS tags and override blocks within the lines"
script_author = "ai-chan"
script_version = "1.000"
script_modified = "10 September 2007"

ktag = "\\[kK][fo]?%d+"

function cleantags(subtitles)
	local linescleaned = 0
	for i = 1, #subtitles do
		aegisub.progress.set(i * 100 / #subtitles)
		if subtitles[i].class == "dialogue" and subtitles[i].text then
			local l = subtitles[i]
			local t = l.text
		
			--[[ Combine adjacent override override blocks into one ]]
			function combineadjacentnotks(block1, block2)
				if string.find(block1, ktag) and string.find(block2, ktag) then
					-- if both adjacent override blocks have \k , let them be
					return "{" .. block1 .. "}" .. string.char(1) .. "{" .. block2 .. "}" -- char(1) prevents infinite loop
				else
					-- either one or both override blocks don't have \k , so combine them into one override block
					return "{" .. block1 .. block2 .. "}"
				end
			end
			repeat
				if aegisub.progress.is_cancelled() then return end
				t, replaced = string.gsub(t,"{(.-)}{(.-)}", combineadjacentnotks)
			until replaced == 0
			t = string.gsub(t, string.char(1), "") -- removes all char(1) we inserted

			--[[ Move first \k tag in override blocks to the front ]]
			t = string.gsub(t, "{([^{}]-)(" .. ktag .. ")(.-)}", "{%2%1%3}") 

			--[[ For some reasons if one override block has more than one \k tag, push those to behind the first \k tag (which has been pushed to front already) ]]
			repeat
				if aegisub.progress.is_cancelled() then return end
				t, replaced = string.gsub(t, "{([^{}]-)(" .. ktag .. ")(\\[^kK][^}]-)(" .. ktag .. ")(.-)}", "{%1%2%4%3%5}")
			until replaced == 0
						
			--[[ Move to the front all tags that affect the whole line (i.e. not affected by their positions in the line) ]]
			local linetags = ""
			function first(pattern)
				local p_s, _, p_tag = string.find(t, pattern)
				if p_s then
					t = string.gsub(t, pattern, "")
					linetags = linetags .. p_tag				
				end
			end
			function firstoftwo(pattern1, pattern2)
				local p1_s, _, p1_tag = string.find(t, pattern1)
				local p2_s, _, p2_tag = string.find(t, pattern2)
				t = string.gsub(t, pattern1, "")
				t = string.gsub(t, pattern2, "")
				if p1_s and (not p2_s or p1_s < p2_s) then
					linetags = linetags .. p1_tag
				elseif p2_s then
					linetags = linetags .. p2_tag				
				end
			end
			-- \an or \a
			first("(\\an?%d+)")
			-- \org
			first("(\\org%([^,%)]*,[^,%)]*%))")
			-- \move and \pos (the first one wins)
			firstoftwo("(\\move%([^,%)]*,[^,%)]*,[^,%)]*,[^,%)]*%))", "(\\pos%([^,%)]*,[^,%)]*%))")
			-- \fade and \fad (the first one wins)
			firstoftwo("(\\fade%([^,%)]*,[^,%)]*,[^,%)]*,[^,%)]*,[^,%)]*,[^,%)]*,[^,%)]*%))", "(\\fad%([^,%)]*,[^,%)]*%))")
			-- integrate
			if string.len(linetags) > 0 then
				if string.sub(t, 1, 1) == "{" then
					t = "{" .. linetags .. string.sub(t, 2)
				else
					t = "{" .. linetags .. "}" .. t
				end
			end

			--[[ Remove any spaces within parenteses within override blocks ]]
			repeat
				if aegisub.progress.is_cancelled() then return end
				t, replaced2 = string.gsub(t, "({[^}]*%([^%s%)}]*)%s+(.*%)[^}]*})", "%1%2")
			until replaced2 == 0
			
			--[[ Remove all empty override blocks ==> {} ]]
			t = string.gsub(t, "{%s*}", "")

			--[[ Finally, update subtitle line with changes ]]
			l.text = t
			subtitles[i] = l
			linescleaned = linescleaned + 1
			aegisub.progress.task(linescleaned .. " lines cleaned")
		end
	end

end

function cleantags_macro(subtitles, selected_lines, active_line)
	cleantags(subtitles)
	aegisub.set_undo_point(script_name)
end

function cleantags_filter(subtitles, config)
	cleantags(subtitles)
end

aegisub.register_macro(script_name, script_description, cleantags_macro)
aegisub.register_filter(script_name, script_description, 0, cleantags_filter)
