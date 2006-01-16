-- Aegisub Automation demonstration script
-- Original written by Niels Martin Hansen
-- Given into the public domain

include("utils.lua")

name = "Include demo"
description = "Simple demonstration of the include function."

version, kind, configuration = 3, 'basic_ass', {}

function process_lines(meta, styles, lines, config)
	lines[lines.n] = copy_line(lines[0])
	lines.n = lines.n + 1
	return lines
end
