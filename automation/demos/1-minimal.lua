-- Aegisub Automation demonstration script
-- Original written by Niels Martin Hansen
-- Given into the public domain

version = 3
kind = "basic_ass"
name = "Minimal demonstration"
description = "A very minimal demonstration of the strucrure of an Automation script."
configuration = {}

function process_lines(meta, styles, lines, config)
	aegisub.report_progress(50)
	aegisub.output_debug("Test script 1 running")
	aegisub.report_progress(100)
	return lines
end
