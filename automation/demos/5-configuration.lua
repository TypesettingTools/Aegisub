-- Aegisub Automation demonstration script
-- Original written by Niels Martin Hansen
-- Given into the public domain

name = "Configuration demo"
description = "This script allows the user to input some data in the Aegisub Export window. Some of these data are used during the processing, to change the subtitles. (The strings is prepended every dialogue line.)"
version, kind = 3, 'basic_ass'
-- The configuration table defines a number of controls to show in the config window
-- The controls must be numbered from 1 and up with no "holes", the number of controls is automatically detected
configuration = {
    [1] = {
		-- There are some mandatory fields for evry control defined
		-- 'name' is the internal name of the control, it's used to refer to the value the user entered later on
        name = "thelabel";
		-- 'kind' is the class of control, this is a label which only displays text and can't be edited
        kind = "label";
		-- 'label' is  a short description of the control, displayed ot the left of it.
		-- In the case of a label control, it's the entire text displayed
        label = "This is a label control. Just for shows.";
		-- 'hint' is a tooltip for the control. It doesn't actually work for label controls though
        hint = "Tooltip for label?!?";
        };
    [2] = {
        name = "thestring";
		-- the "text" class is a regular text input field
        kind = "text";
        label = "String:";
        hint = "The string to insert at the beginning of each line";
		-- The 'default' is required for everything but 'label' controls, it simply specifies the default value
        default = "foobar "
        };
    [3] = {
        name = "theint";
		-- 'int' is a text control that only allows inputting whole numbers (integers)
        kind = "int";
        label = "Integer:";
        hint = "An integer number to display in debug output";
        default = 50;
		-- If the 'min' and 'max' fields are specified, the control will get a "spin control" attached,
		-- which can be used to change the value with the mouse
        min = 0;
        max = 100;
        };
    [4] = {
        name = "thefloat";
		-- 'float' is like int, except you can also put decimal numbers in  it
        kind = "float";
        label = "Float number:";
        hint = "Just a random float number";
        default = 3.1415927;
		-- There are no working spin controls for floats, unfortunately
        };
    [5] = {
        name = "thebool";
		-- 'bool' makes a checkbox, which can either be checked or unchecked
        kind = "bool";
        label = "I accept";
        hint = "Check if you accept the terms of the license agreement";
		-- false means unchecked, true means checked
        default = false;
        };
    [6] = {
        name = "thecolour";
		-- 'colour' is currently just a text edit box which allows you to input an ASS format color
		-- The idea was to have some kind of graphical control, but that probably won't happen in auto3
        kind = "colour";
        label = "Favourite color:";
        hint = "What color do you want your pantsu?";
        default = "&H8080FF";
        };
    [7] = {
        name = "thestyle";
		-- 'style' produces a drop down list, where the user can select one of the styles in the file, or no style at all
        kind = "style";
        label = "Style:";
        hint = "Pick a style the effects will apply to, or none to apply to everything";
		-- An empty string means that nothing is selected
		-- Be careful about assuming any style names that might be available in the subtitle file
        default = "";
        }
}

function process_lines(meta, styles, lines, config)
	-- All the values selected by the user are stored in the 'config' variable
	-- For example, control 2 in this example has name "thestring", so you can access the value input by config.thestring
    aegisub.output_debug("The string entered is: " .. config.thestring)
    for i = 0, lines.n-1 do
		aegisub.report_progress(i/lines.n*100)
        if lines[i].kind == "dialogue" then
			-- So loop over all dialogue lines and append thestring to them all
            lines[i].text = config.thestring .. lines[i].text
        end
    end
    return lines
end

