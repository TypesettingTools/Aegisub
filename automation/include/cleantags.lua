--[[ 
"Clean Tags" -- An Auto4 LUA script for cleaning up ASS subtitle lines of badly-formed override 
blocks and redundant/duplicate tags
* Designed to work for Aegisub 2.0 and above
* include()'ed this file from any auto4 script to use the cleantags() function below
* Might change from time to time so look out for cleantags_version below

Copyright (c) 2007-2009 Muhammad Lukman Nasaruddin (aka ai-chan, Aegisub's forum member)

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

cleantags_version = "1.300"
cleantags_modified = "27 February 2009"

ktag = "\\[kK][fo]?%d+"

--[[ The main function that performs the cleaning up
Takes: text
Returns: cleaned up text
]]
function cleantags(text)
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
		text, replaced = string.gsub(text,"{(.-)}{(.-)}", combineadjacentnotks)
	until replaced == 0
	text = string.gsub(text, string.char(1), "") -- removes all char(1) we inserted

	--[[ Move first \k tag in override blocks to the front ]]
	text = string.gsub(text, "{([^{}]-)(" .. ktag .. ")(.-)}", "{%2%1%3}") 

	--[[ For some reasons if one override block has more than one \k tag, 
	push those to behind the first \k tag (which has been pushed to front already) ]]
	repeat
		if aegisub.progress.is_cancelled() then return end
		text, replaced = string.gsub(text, "{([^{}]-)(" .. ktag .. ")(\\[^kK][^}]-)(" .. ktag .. ")(.-)}", "{%1%2%4%3%5}")
	until replaced == 0
				
	--[[ Move to the front all tags that affect the whole line (i.e. not affected by their positions in the line) ]]
	local linetags = ""
	function first(pattern)
		local p_s, _, p_tag = string.find(text, pattern)
		if p_s then
			text = string.gsub(text, pattern, "")
			linetags = linetags .. p_tag				
		end
	end
	function firstoftwo(pattern1, pattern2)
		local p1_s, _, p1_tag = string.find(text, pattern1)
		local p2_s, _, p2_tag = string.find(text, pattern2)
		text = string.gsub(text, pattern1, "")
		text = string.gsub(text, pattern2, "")
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
		if string.sub(text, 1, 1) == "{" then
			text = "{" .. linetags .. string.sub(text, 2)
		else
			text = "{" .. linetags .. "}" .. text
		end
	end

	--[[ Remove any spaces within parenteses within override blocks except for \clip tags ]]
	local comb = function(a,b,c,d,e) 
		if c ~= "\\clip" or d:sub(-1):find("[,%({]") or e:sub(1,1):find("[,%)}]") then return a..b..d..e 
		else return a..b..d..string.char(2)..e end 
	end
    repeat
        text, replaced2 = string.gsub(text, "({[^}\\]*)([^}%s]*(\\[^%(}\\%s]*))%s*(%([^%s%)}]*)%s+([^}]*)", comb)
    until replaced2 == 0
    text, _ = text:gsub(string.char(2)," ")
		
	--[[ Remove all empty override blocks ==> {} ]]
	text = string.gsub(text, "{%s*}", "")

	--[[ Finally, return the cleaned up text ]]
	return text
end
