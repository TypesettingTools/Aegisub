script_name = "Automation 4 test 9"
script_description = "Test debug out function"
script_author = "Niels Martin Hansen"
script_version = "2"


function test9(subtitles, selected_lines, active_line)
	aegisub.debug.out("Only string argument\n")
	aegisub.debug.out("Hello %s!\n", "format string world")
	aegisub.log("Now going to log 7 strings with trace levels 0 to 6:\n")
	for i = 0, 6 do
		aegisub.log(i, "Trace level %d...\n", i)
	end
	aegisub.debug.out(3, "Finished!")
end

aegisub.register_macro("Test debug out", "Tests the aegisub.debug.out function", test9)
