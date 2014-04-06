-- Automation 4 test file
-- Test the unicode.lua include

script_name = "Automation 4 test 7"
script_description = "Test unicode.lua include"
script_author = "Niels Martin Hansen"
script_version = "1"

include("unicode.lua")

function test8(subtitles, selected_lines, active_line)
	local s = "ōkii テスト aåǄਵ℡랩ﾑ﹌"
	
	aegisub.debug.out("Test string is: " .. s .. "\n")
	
	aegisub.debug.out("Width of first character is: " .. unicode.charwidth(s, 1) .. "\n")
	
	local len = unicode.len(s)
	aegisub.debug.out("Byte length is: " .. s:len() .. "\n")
	aegisub.debug.out("Unicode length is: " .. len .. "\n")
	
	for c, i in unicode.chars(s) do
		local cp = unicode.codepoint(c)
		aegisub.debug.out(string.format("Character %d is '%s' with codepoint %d\n", i, c, cp))
	end
	
	aegisub.debug.out("Done!")
end

aegisub.register_macro("Test Unicode", "Tests the Unicode include file", test8, nil)
