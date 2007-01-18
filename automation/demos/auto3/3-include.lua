-- Aegisub Automation demonstration script
-- Original written by Niels Martin Hansen
-- Given into the public domain

-- This script demonstrated using include scripts in Automation.
-- You include a file simply by calling the include function with a filename.
-- Depending on whether the filename contains a path or not, it is interpreted in slightly different ways,
-- see the documentation for more information.
include("utils.lua")

-- The usual info
name = "Include demo"
description = "Simple demonstration of the include function."
-- Here several values are set in one line
version, kind, configuration = 3, 'basic_ass', {}

function process_lines(meta, styles, lines, config)
	-- Copy the first line (line 0) and store it as as a new last line
	-- The last line has index lines.n-1, so lines.n is one past the last line
	lines[lines.n] = copy_line(lines[0])
	-- Make sure to increment 'n', so the correct number of lines are read out
	lines.n = lines.n + 1
	-- Then return the modified lines table
	return lines
end
