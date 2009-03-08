--[[
	assfile.lua - Functions for parsing ASS format subtitles
	Part of OverLua, licensed under GPLv2
]]

module "assfile"


-- Parsing helper functions
local function trim(s)
	return (s:gsub("^[ \t\r\n]*(.-)[ \t\r\n]*$", "%1"))
end
local function num(s)
	return tonumber(trim(s)) or 0
end


-- Which file format we're working with.
-- Changing this changes the behaviour of various parsing functions.
-- Valid values: "SSA4", "ASS", "ASS2", "ASS3", "AS5"
-- Other versions aren't supported.
-- This is usually automatically set by the whole-file parsing functions.
format = "SSA4"


-- Create a solid colour fill style
-- Colours are "cairo compatible", ie. range 0 to 1
-- A colour never stores alpha, that's a separate property
function color(r, g, b, a)
	r, g, b, a = r or 0, g or 0, b or 0, a or 0
	return {class="solid", r=r, g=g, b=b, a=a}
end

-- Parse a colour in SSA, ASS or HTML format
function parse_color(s)
end


-- Base style, this defines the defaults used for undefined fields in read-in styles
default_style = {
	-- Default style has no name
	name = nil,
	-- Well, sensible on Windows
	font_face = "Arial",
	font_size = "24",
	font_language = "C", -- Language to assume text is in, affects some rendering
	font_vertical = false, -- Vertical layout, replaces use of "@-fonts"
	font_bold = false,
	font_italic = false,
	font_underline = false,
	font_overstrike = false,
	-- Fill style
	fill1 = color(1, 1, 1, 1), -- Fill, white
	fill2 = color(1, 0, 0, 1), -- Pre-karaoke fill, red
	fill3 = color(0, 0, 0, 1), -- Outline, black
	fill4 = color(0, 0, 0, 0.5), -- Shadow, semi-transparent black
	-- Border size
	outline = 2,
	shadow = 2,
	opaque_box = false,
	-- Scaling
	scale_x = 1.00,
	scale_y = 1.00,
	-- Spacing
	spacing_char = 0, -- Extra inter-character spacing
	spacing_line = 0, -- Extra inter-line spacing
	baseline_shift = 0, -- Shift baseline this amount against line advance direction
	-- Text positioning
	margin_l = 10,
	margin_r = 10,
	margin_t = 10,
	margin_b = 10,
	align_x = 0.5, -- Bouding box centered between left/right margins
	align_y = 1.0, -- Bounding box placed at bottom of screen
	inner_align_x = 0.5, -- Lines centered inside bounding box
	inner_align_y = 1.0, -- Lines at bottom of bounding box
	break_style = "smart_top", -- "Smart-wrap", earlier lines wider (alt. "smart_bottom", "simple", "none")
	pos_rel = "video", -- Positioning relative to video frame (alt. "screen")
	pos_x = nil, -- Don't override positioning
	pos_y = nil,
	-- Rotation/reshaping
	origin = nil, -- Automatic, at center of text
	angle_x = 0,
	angle_y = 0,
	angle_z = 0,
	shear_x = 0,
	shear_y = 0,
	baseline_transform = nil,
	bounding_box_transform = nil,
	-- Misc.
	blend_mode = "normal", -- Alt. "add", "multiply"
	clip_rect = nil,
	clip_shape = nil,
	blur = 0,
	-- Animation
	fade = nil,
	animation = nil
}


-- Parse a Style line
function read_style(t)
	if format == "AS5" then
		error("Found Style line in an AS5 file, where it's unsupported.\nLine: %s", t)
	
	elseif format == "SSA4" then
		local name, font_face, font_size, color1, color2, color3, color4,
			bold, italic, underline, overstrike, border, outline, shadow,
			align, margin_l, margin_r, margin_v, alpha, encoding
			= t:match("^.-:(.-),(.-),(.-),(.-),(.-),(.-),(.-),(.-),(.-),(.-),(.-),(.-),(.-),(.-),(.-),(.-),(.-),(.-),(.-),(.-)$")
		name = trim(name)
		font_face = trim(name)
		font_size = num(font_size)
	end
end


-- Parse a StyleEx line
function read_style_ex(t)
	if format ~= "ASS3" and format ~= "AS5" then
		error("Found StyleEx line in format that doesn't support it.\nFormat is \"%s\".\nLine is: %s", format, t)
	end
end


-- Merge two styles by "overlaying" a style upon a base
-- Produces a new style
-- Fields where the overlay has a defined value, the output style has the value of overlay,
-- but in fields where overlay is nil, the output has the base value.
-- (Pretty much "out.field = overlay.field or base.field".)
-- Animation lists are merged however, but this may produce weird results
-- if the animations overlap in non-trivial ways. (Just pretend this says "undefined behaviour.")
function merge_styles(base, overlay)
end
