-- Automation 4 test file
-- Create a Filter feature that does some kara stuff

script_name = "Automation 4 test 7"
script_description = "Test config dialogs"
script_author = "Niels Martin Hansen"
script_version = "1"

include("utils.lua")

function test7(subtitles, selected_lines, active_line)
	local a, b = aegisub.dialog.display({{class="label", label="Test..."}}, {})
	report_dialog_result(a, b)
	aegisub.progress.set(50)
	a, b = aegisub.dialog.display({{class="edit", name="foo", text=""}}, {"foo", "bar"})
	report_dialog_result(a, b)
end

function report_dialog_result(button, controls)
	aegisub.debug.out("Dialog closed: ")
	if button == false then
		aegisub.debug.out("cancelled\n")
	elseif button == true then
		aegisub.debug.out("clicked Ok\n")
	else
		aegisub.debug.out("clicked '" .. button .. "'\n")
	end
	for key, val in pairs(controls) do
		aegisub.debug.out(key .. ': ' .. val .. '\n')
	end
	aegisub.debug.out(" - - - - -\n")
end


function exporter(subs, config)
	for i = 1, #subs do
		local l = subs[i]
		if l.class == "dialogue" and not l.comment then
			if config.style == "" or l.style == config.style then
				l.text = config.text .. l.text
				subs[i] = l
			end
		end
	end
end

function export_config_dialog(subs, store)
	local styles = {""}
	for i = 1, #subs do
		local l = subs[i]
		if l.class == "style" then
			table.insert(styles, l.name)
		end
	end
	
	return {
		{
			class = "label",
			label = "This will insert a given text in\n" ..
			        "front of all dialogue lines of\n" ..
					"the given style, or every line\n" ..
					"if no specific style is selected.",
			x = 0, y = 0, width = 2, height = 1
		},
		{
			class = "label",
			label = "Text to insert:",
			x = 0, y = 1, width = 1, height = 1
		},
		{
			class = "edit",
			name = "text",
			x = 1, y = 1, width = 1, height = 1
		},
		{
			class = "label",
			label = "Style to apply on:",
			x = 0, y = 2, width = 1, height = 1
		},
		{
			class = "dropdown",
			name = "style",
			value = "",
			items = styles,
			x = 1, y = 2, width = 1, height = 1
		}
	}
end


aegisub.register_macro("Config Dialog 1", "Show a stupid config dialog", test7, nil)
aegisub.register_filter("Export Config", "Test export filter config dialog stuff", 500, exporter, export_config_dialog)
