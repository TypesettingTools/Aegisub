-- Automation 4 test file
-- Test that appending lines to a file places the lines in the appropriate
-- sections of the file, creating the sections if needed

script_name = "TEST append lines"
script_description = "Test that appended lines go to the right place"
script_author = "Thomas Goyne"
script_version = "1"

function make_scriptinfo(key, value)
    return {
        class = "info",
        section = "[Script Info]",
        key = key,
        value = value
    }
end

function make_style(name)
    return {
        class = "style",
        section = "[V4+ Styles]",
        name = name,
        fontname = "Arial",
        fontsize = 20,
        color1 = "&H000000&",
        color2 = "&H000000&",
        color3 = "&H000000&",
        color4 = "&H000000&",
        bold = true,
        italic = false,
        underline = false,
        strikeout = false,
        scale_x = 100,
        scale_y = 100,
        spacing = 0,
        angle = 0,
        borderstyle = 0,
        outline = 0,
        shadow = 0,
        align = 5,
        margin_l = 0,
        margin_r = 0,
        margin_t = 0,
        margin_b = 0,
        encoding = 0
    }
end
function make_dialogue(text)
    return {
        class = "dialogue",
        section = "[Events]",
        comment = false,
        layer = 0,
        start_time = 0,
        end_time = 2000,
        style = "Default",
        actor = "",
        margin_l = 0,
        margin_r = 0,
        margin_t = 0,
        margin_b = 0,
        effect = "",
        text = text
    }
end

function make_header(name)
    return {
        class = "head",
        section = name
    }
end

function make_format(fstr, section)
    return {
        class = "format",
        section = section,
        text = fstr
    }
end

function check_field(i, actual, expected, name)
    if actual ~= expected then
        error(i .. ": Expected '" .. expected .. "', got '" .. actual .. "' for " .. name)
    end
end

function check_line(i, line, class, section)
    check_field(i, line.class, class, "class")
    check_field(i, line.section, section, "section")
end

function test(subs)
    subs.deleterange(1, #subs)

    -- verify that everything works with the items added in order
    subs[0] = make_header("[Script Info]")
    subs[0] = make_scriptinfo("Title", "Default Aegisub file")
    subs[0] = make_scriptinfo("ScriptType", "v4.00+")
    subs[0] = make_scriptinfo("WrapStyle", "0");
    subs[0] = make_scriptinfo("ScaledBorderAndShadow", "yes")
    subs[0] = make_scriptinfo("Collisions", "Normal")

    subs[0] = make_header("[V4+ Styles]")
    subs[0] = make_format("Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding", "[V4+ Styles]")
    subs[0] = make_style("Default")

    subs[0] = make_header("[Events]")
    subs[0] = make_format("Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text", "[Events]")
    subs[0] = make_dialogue("")

    check_line(1, subs[1], "head", "[Script Info]")
    check_line(2, subs[2], "info", "[Script Info]")
    check_line(3, subs[3], "info", "[Script Info]")
    check_line(4, subs[4], "info", "[Script Info]")
    check_line(5, subs[5], "info", "[Script Info]")
    check_line(6, subs[6], "info", "[Script Info]")

    check_line(7, subs[7], "head", "[V4+ Styles]")
    check_line(8, subs[8], "format", "[V4+ Styles]")
    check_line(9, subs[9], "style", "[V4+ Styles]")

    check_line(10, subs[10], "head", "[Events]")
    check_line(11, subs[11], "format", "[Events]")
    check_line(12, subs[12], "dialogue", "[Events]")

    subs.deleterange(1, #subs)

    -- test that groups appear in the order they're first used and lines are
    -- put in the right group
    subs[0] = make_scriptinfo("Title", "Script Title")
    subs[0] = make_style("Default")
    subs[0] = make_dialogue("Foo")
    subs[0] = make_style("Default 2")
    subs[0] = make_scriptinfo("ScriptType", "v4.00+")
    subs[0] = make_dialogue("Bar")

    check_line(1, subs[1], "head", "[Script Info]")
    check_line(2, subs[2], "info", "[Script Info]")
    check_line(3, subs[3], "info", "[Script Info]")

    check_line(4, subs[4], "head", "[V4+ Styles]")
    check_line(5, subs[5], "style", "[V4+ Styles]")
    check_line(6, subs[6], "style", "[V4+ Styles]")

    check_line(7, subs[7], "head", "[Events]")
    check_line(8, subs[8], "dialogue", "[Events]")
    check_line(9, subs[9], "dialogue", "[Events]")

    aegisub.set_undo_point("append test")
end

aegisub.register_macro(script_name, script_description, test)
