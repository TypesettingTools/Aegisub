-- Aegisub Automation demonstration script
-- Original written by Niels Martin Hansen
-- Given into the public domain

name = "Configuration demo"
description = "This script allows the user to input some data in the Aegisub Export window. Some of these data are used during the processing, to change the subtitles. (The strings is prepended every dialogue line.)"
configuration = {
    [1] = {
        name = "thelabel";
        kind = "label";
        label = "This is a label control. Just for shows.";
        hint = "Tooltip for label?!?";
        };
    [2] = {
        name = "thestring";
        kind = "text";
        label = "String:";
        hint = "The string to insert at the beginning of each line";
        default = "foobar "
        };
    [3] = {
        name = "theint";
        kind = "int";
        label = "Integer:";
        hint = "An integer number to display in debug output";
        default = 50;
        min = 0;
        max = 100;
        };
    [4] = {
        name = "thefloat";
        kind = "float";
        label = "Float number:";
        hint = "Just a random float number";
        default = 3.1415927;
        };
    [5] = {
        name = "thebool";
        kind = "bool";
        label = "I accept";
        hint = "Check if you accept the terms of the license agreement";
        default = false;
        };
    [6] = {
        name = "thecolour";
        kind = "colour";
        label = "Favourite color:";
        hint = "What color do you want your pantsu?";
        default = "&H8080FF";
        };
    [7] = {
        name = "thestyle";
        kind = "style";
        label = "Style:";
        hint = "Pick a style the effects will apply to, or none to apply to everything";
        default = "";
        }
}

version, kind = 3, 'basic_ass'

function process_lines(meta, styles, lines, config)
    aegisub.output_debug("The string entered is: " .. config.thestring)
    for i = 0, lines.n-1 do
		aegisub.report_progress(i/lines.n*100)
        if lines[i].kind == "dialogue" then
            lines[i].text = config.thestring .. lines[i].text
        end
    end
    return lines
end

