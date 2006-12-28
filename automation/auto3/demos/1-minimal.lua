-- Aegisub Automation demonstration script
-- Original written by Niels Martin Hansen
-- Given into the public domain

-- Comment lines (like this line) start with two hyphens, these are ignored when the script is interpreted.
-- This script does not do anything useful, it merely displays a message. Think of it as a kind of "hello world" script.

-- The following lines define some values identifying the script
-- This script is written for Automation version 3
version = 3
-- And it's a "basic ass" processing script. There are no other kinds of scripts though... but this is still required.
kind = "basic_ass"
-- This is the name of the script that shows up in Aegisub
name = "Minimal demonstration"
-- This is a longer description of the script
description = "A very minimal demonstration of the strucrure of an Automation script."
-- This defines any configuration that can be done of the script. This is demonstrated in demo 5.
-- The empty braces denote an empty table (associative array)
configuration = {}

-- This is the main function of the script.
-- This function is run whenever the script is run
function process_lines(meta, styles, lines, config)
	-- The function aegisub-report_progress changes the position of the progress bar shown while the script is running
	-- The value given is in percent.
	aegisub.report_progress(50)
	-- The aegisub,.output_debug function shows some text
	aegisub.output_debug("Test script 1 running")
	aegisub.report_progress(100)
	-- It is very important that the process_lines function returns an array of lines, otherwise an error will occur
	-- If an empty array is returned, the entire file is cleared!
	return lines
end
