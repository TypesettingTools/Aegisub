-- Automation 4 test file
-- Deterministic macro used by the meson test suite to exercise --cli mode

script_name = "CLI test macros"
script_description = "Macros used by the automated tests for Aegisub's CLI mode"
script_author = "Aegisub Project"
script_version = "1"

local clipboard = require 'aegisub.clipboard'

local function append_marker(subs, sel, act)
	aegisub.progress.title("CLI test")
	aegisub.progress.task("Appending markers")

	-- The clipboard functions bounce through the main thread via
	-- dispatch::Main().Sync(), so this exercises the CLI main loop pump
	assert(clipboard.set("cli-test-clipboard"), "clipboard.set failed")
	assert(clipboard.get() == "cli-test-clipboard", "clipboard.get returned the wrong value")

	for n, i in ipairs(sel) do
		local line = subs[i]
		if not line.comment then
			line.text = line.text .. "\\N{cli-test}"
			subs[i] = line
		end
		aegisub.progress.set(100 * n / #sel)
	end
	aegisub.set_undo_point("append cli-test marker")
end

local function count_lines(subs, sel, act)
	local dialogue = 0
	for i = 1, #subs do
		if subs[i].class == "dialogue" then
			dialogue = dialogue + 1
		end
	end
	aegisub.log(0, "dialogue lines: " .. dialogue .. "\n")
end

aegisub.register_macro("CLI Test/Append marker", "Appends a marker to all selected lines", append_marker)
aegisub.register_macro("CLI Test/Count lines", "Logs the number of dialogue lines", count_lines)
