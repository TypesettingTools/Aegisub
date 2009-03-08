--[[

Sample script for OverLua
 - demonstrate basic reading in an ASS subtitle file and rendering its lines on the video
 
Given into the public domain.
(You can do anything you want with this file, with no restrictions whatsoever.
 You don't get any warranties of any kind either, though.)
 
Originally authored by Niels Martin Hansen.

]]

-- overlua_datastring is the string given to data= argument in Avisynth filter invocation
timing_input_file = overlua_datastring
-- You may want to get these from somewhere else
-- Or just stick to hardcoding all styling information
font_name = "Arial"
font_size = 24

-- A little check that an input file was actually given
assert(timing_input_file, "Missing input file")


-- An easier way to convert a string to a number and be guaranteed to get something useful
function parsenum(str)
	return tonumber(str) or 0
end
-- Convert an ASS timestamp into a number of seconds
function parse_ass_time(ass)
	local h, m, s, cs = ass:match("(%d+):(%d+):(%d+)%.(%d+)")
	return parsenum(cs)/100 + parsenum(s) + parsenum(m)*60 + parsenum(h)*3600
end

-- Not used here, but do some basic \k timing parsing
function parse_k_timing(text)
	local syls = {}
	local i = 1
	for timing, syltext in text:gmatch("{\\k(%d+)}([^{]*)") do
		local syl = {dur = parsenum(timing)/100, text = syltext, i = i}
		table.insert(syls, syl)
		i = i + 1
	end
	return syls
end

-- Really stupid ASS parser
function read_input_file(name)
	for line in io.lines(name) do
		-- You can catch Style lines in the same way if you want
		local start_time, end_time, style, fx, text = line:match("Dialogue: 0,(.-),(.-),(.-),.-,0000,0000,0000,(.-),(.*)")
		if text then
			local ls = {}
			ls.start_time = parse_ass_time(start_time)
			ls.end_time = parse_ass_time(end_time)
			ls.style = style
			ls.fx = fx
			ls.rawtext = text
			ls.kara = parse_k_timing(text)
			-- Clear out override blocks
			ls.cleantext = text:gsub("{.-}", "")
			table.insert(lines, ls)
		end
	end
end

-- Initialisation function, intended to be run on the first frame
-- I think it's better to delay initialisation until first frame is requested
-- rather than doing it immediately at load time.
-- I believe much encoding software might be more graceful about that.
function init()
	if inited then return end
	inited = true
	
	lines = {}
	read_input_file(timing_input_file)
end


-- Actual work function, this is what's called by OverLua for each frame
-- f is a video frame object, what we'll be drawing to
-- t is the timestamp of the frame, given in seconds (floating point)
function render_frame(f, t)
	-- Just call init() every time, init itself makes sure it doesn't init more than once
	init()
	
	-- Find lines to be drawn
	-- NOTE: This structure is very simplistic.
	-- Advanced karaoke effects will probably need to build an object/effect list
	-- as initialisation and then instead go over the list of objects/effects per frame,
	-- instead of going over lines.
	-- (Ie. preprocessing in the style of ASS karaoke generation.)
	for i, line in pairs(lines) do
		if line.start_time <= t and line.end_time > t then
			-- Initial position of line
			local x = 0
			local y = f.height - 25
			
			-- Initialise drawing surface and context
			local surf = cairo.image_surface_create(f.width, f.height, "argb32")
			local c = surf.create_context()
			
			-- Select our font
			c.select_font_face(font_name)
			c.set_font_size(font_size)
			
			-- Get sizing information for font and line text
			if not line.te then line.te = c.text_extents(line.cleantext); line.fe = c.font_extents() end
			-- Calculate centered position
			x = (f.width - line.te.width) / 2 - line.te.x_bearing
			
			-- Produce text path
			c.move_to(x, y)
			c.text_path(line.cleantext)
			
			-- Draw a black border
			-- It will be centered on the text outline
			c.set_line_width(4)
			c.set_line_join("round") -- default is "miter"
			c.set_source_rgba(0, 0, 0, 1)
			-- "preserve" to keep the text path in the context
			c.stroke_preserve()
			-- Apply a little box blur to the border, effectively \be1
			raster.box_blur(surf, 3)
			
			-- Draw a white fill - this will draw over the inner part of the border
			c.set_source_rgba(1, 1, 1, 1)
			c.fill()

			-- Overlay on video
			f.overlay_cairo_surface(surf, 0, 0)
		end
	end
end
