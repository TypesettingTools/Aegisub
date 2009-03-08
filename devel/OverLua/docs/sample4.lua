--[[

Sample script for OverLua
 - advanced karaoke effect, Prism Ark OP kara effect for Anime-Share Fansubs
 
Given into the public domain.
(You can do anything you want with this file, with no restrictions whatsoever.
 You don't get any warranties of any kind either, though.)
 
Originally authored by Niels Martin Hansen.

Not an extremely advanced effect, but showcases improved parsing of ASS files
and using information from the Style lines for the styling information.

Pretty fast to render at SD resolutions.

As for other effects, please consider that it's not much fun to just re-use
an effect someone else wrote, especially not verbatim. If you elect to use
this sample for something, I ask you to do something original with it. I
can't force you, but please :)

I'm leaving several sections of this script mostly unexplained, because I've
for a large part copied those from the Gundam 00 OP 1 effect (sample3) I did
a few days before this one.
Please see sample3.lua for explanations of those, if you need them.

]]

---- START CONFIGURATION ----

-- Duration of line fade-in/outs, in seconds
local line_fade_duration = 0.5
-- Minimum duration of highlights, also seconds
local syl_highlight_duration = 0.5

---- END CONFIGURATION ----


-- Trim spaces from beginning and end of string
function string.trim(s)
	return (string.gsub(s, "^%s*(.-)%s*$", "%1"))
end


-- Script and video resolutions
local sres_x, sres_y
local vres_x, vres_y

-- Stuff read from style definitions
local font_name = {}
local font_size = {}
local font_bold = {}
local font_italic = {}
local pos_v = {}
local vertical = {}
local color1, color2, color3, color4 = {}, {}, {}, {}

-- Input lines
local lines = {}


-- Read input file
function read_field(ass_line, num)
	local val, rest = ass_line:match("(.-),(.*)")
	if not rest then
		return ass_line, ""
	elseif num > 1 then
		return val, read_field(rest, num-1)
	else
		return val, rest
	end
end
function parsenum(str)
	return tonumber(str) or 0
end
function parse_ass_time(ass)
	local h, m, s, cs = ass:match("(%d+):(%d+):(%d+)%.(%d+)")
	return parsenum(cs)/100 + parsenum(s) + parsenum(m)*60 + parsenum(h)*3600
end
function parse_style_color(color)
	local res = {r = 0, g = 0, b = 0, a = 0}
	local a, b, g, r = color:match("&H(%x%x)(%x%x)(%x%x)(%x%x)")
	res.r = tonumber(r, 16) / 255
	res.g = tonumber(g, 16) / 255
	res.b = tonumber(b, 16) / 255
	res.a = 1 - tonumber(a, 16) / 255 -- Alpha has inverse meaning in ASS and cairo
	return res
end

function parse_k_timing(text, start_time)
	local syls = {}
	local cleantext = ""
	local i = 1
	local curtime = start_time
	for timing, syltext in text:gmatch("{\\k(%d+)}([^{]*)") do
		local syl = {}
		syl.dur = parsenum(timing)/100
		syl.text = syltext
		syl.i = i
		syl.start_time = curtime
		syl.end_time = curtime + syl.dur
		table.insert(syls, syl)
		cleantext = cleantext .. syl.text
		i = i + 1
		curtime = curtime + syl.dur
	end
	if cleantext == "" then
		cleantext = text
	end
	return syls, cleantext
end

function read_input_file(name)
	for line in io.lines(name) do
		-- Try PlayResX/PlayResY
		local playresx = line:match("^PlayResX: (.*)")
		if playresx then
			sres_x = parsenum(playresx)
		end
		local playresy = line:match("^PlayResY: (.*)")
		if playresy then
			sres_y = parsenum(playresy)
		end
	
		-- Try dialogue line
		-- Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text
		local dialogue_line = line:match("^Dialogue:(.*)")
		if dialogue_line then
			local layer, start_time, end_time, style, actor, margin_l, margin_r, margin_v, effect, text = read_field(dialogue_line, 9)
			local ls = {}
			ls.layer = parsenum(layer)
			ls.start_time = parse_ass_time(start_time)
			ls.end_time = parse_ass_time(end_time)
			ls.style = style:trim()
			ls.actor = actor:trim()
			ls.effect = effect:trim()
			ls.rawtext = text
			ls.kara, ls.cleantext = parse_k_timing(text, ls.start_time)
			
			table.insert(lines, ls)
		end

		-- Try style line
		-- Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding
		local style_line = line:match("^Style:(.*)")
		if style_line then
			local name, font, size, c1, c2, c3, c4, bold, italic, underline, overstrike, scalex, scaley, spacing, angle, borderstyle, outline, shadow, alignment, margin_l, margin_r, margin_v, encoding = read_field(style_line, 22)
			-- Direct data
			name = name:trim()
			font_name[name] = font:trim()
			font_size[name] = parsenum(size)
			color1[name] = parse_style_color(c1)
			color2[name] = parse_style_color(c2)
			color3[name] = parse_style_color(c3)
			color4[name] = parse_style_color(c4)
			font_bold[name] = (parsenum(bold) ~= 0) and "bold" or ""
			font_italic[name] = (parsenum(italic) ~= 0) and "italic" or ""

			-- Derived data
			if font:match("@") then
				vertical[name] = true
			end
			alignment = parsenum(alignment)
			if alignment <= 3 then
				if vertical[name] then
					pos_v[name] = sres_x - parsenum(margin_v)
				else
					pos_v[name] = sres_y - parsenum(margin_v)
				end
			elseif alignment <= 6 then
				if vertical[name] then
					pos_v[name] = sres_x / 2
				else
					pos_v[name] = sres_y / 2
				end
			else
				pos_v[name] = parsenum(margin_v)
			end
		end
	end
end

function init(f)
	if inited then return end
	inited = true
	
	vres_x = f.width
	vres_y = f.height
	read_input_file(overlua_datastring)
end


-- Mask for noise over background
local noisemask, noisemaskctx, noisemaskfilled
-- Additional images to overlay the frame
local frame_overlays = {}


-- Calculate size and position of a line and its syllables
function calc_line_metrics(ctx, line)
	if line.pos_x then return end

	assert(font_name[line.style], "No font name for style " .. line.style)
	ctx.select_font_face(font_name[line.style], font_italic[line.style], font_bold[line.style])
	ctx.set_font_size(font_size[line.style])
	
	line.te = ctx.text_extents(line.cleantext)
	line.fe = ctx.font_extents()
	
	if vertical[line.style] then
		line.pos_x = pos_v[line.style]
		line.pos_y = (sres_y - line.te.width) / 2 - line.te.x_bearing
	else
		line.pos_x = (sres_x - line.te.width) / 2 - line.te.x_bearing
		line.pos_y = pos_v[line.style]
	end
	
	if #line.kara < 2 then return end
	
	local curx = line.pos_x
	local cury = line.pos_y
	for i, syl in pairs(line.kara) do
		syl.te = ctx.text_extents(syl.text)
		if vertical[line.style] then
			syl.pos_x = line.pos_x
			syl.pos_y = cury
			syl.center_x = syl.pos_x + syl.te.x_bearing + syl.te.width/2
			syl.center_y = cury - line.fe.ascent/2 + line.fe.descent/2
		else
			syl.pos_x = curx
			syl.pos_y = line.pos_y
			syl.center_x = curx + syl.te.x_bearing + syl.te.width/2
			syl.center_y = syl.pos_y - line.fe.ascent/2 + line.fe.descent/2
		end
		curx = curx + syl.te.x_advance
		cury = cury + syl.te.x_advance
	end
end


-- Style handling functions
local stylefunc = {}

function stylefunc.generic(t, line)
	if not line.textsurf then
		line.textsurf = cairo.image_surface_create(sres_x, sres_y, "argb32")
		local c = line.textsurf.create_context()

		c.select_font_face(font_name[line.style], font_italic[line.style], font_bold[line.style])
		c.set_font_size(font_size[line.style])
		
		if vertical[line.style] then
			c.translate(line.pos_x, line.pos_y)
			c.rotate(math.pi/2)
			c.move_to(0,0)
		else
			c.move_to(line.pos_x, line.pos_y)
		end
		c.text_path(line.cleantext)
		
		local c1, c3 = color1[line.style], color3[line.style]
		c.set_source_rgba(c1.r, c1.g, c1.b, c1.a)
		c.set_line_join("round")
		c.set_line_width(4)
		c.stroke_preserve()
		c.set_source_rgba(c3.r, c3.g, c3.b, c3.a)
		c.fill()
	end
	
	-- Fade-factor (alpha for line)
	local fade = 0
	
	if t < line.start_time and t >= line.start_time - line_fade_duration then
		fade = 1 - (line.start_time - t) / line_fade_duration
	elseif t >= line.end_time and t < line.end_time + line_fade_duration then
		fade = 1 - (t - line.end_time) / line_fade_duration
	elseif t >= line.start_time and t < line.end_time then
		fade = 1
	else
		fade = 0
	end
	
	if fade > 0 then
		local lo = {} -- line overlay
		lo.surf = line.textsurf
		lo.x, lo.y = 0, 0
		lo.alpha = 0.85 * fade
		lo.operator = "over"
		lo.name = "line"
		table.insert(frame_overlays, lo)
		
		noisemaskctx.set_source_surface(line.textsurf, 0, 0)
		noisemaskctx.paint_with_alpha(fade)
		noisemaskfilled = true
	end
	
	for i, syl in pairs(line.kara) do
		if syl.end_time < syl.start_time + syl_highlight_duration then
			syl.end_time = syl.start_time + syl_highlight_duration
		end
		
		if t >= syl.start_time and t < syl.end_time then
			local sw, sh = syl.te.width*3, line.fe.height*3
			if vertical[line.style] then
				sw, sh = sh, sw
			end
			
			local fade = (syl.end_time - t) / (syl.end_time - syl.start_time)
			
			local surf = cairo.image_surface_create(sw, sh, "argb32")
			local ctx = surf.create_context()
			
			ctx.select_font_face(font_name[line.style], font_italic[line.style], font_bold[line.style])
			ctx.set_font_size(font_size[line.style]*2)
			
			local te, fe = ctx.text_extents(syl.text), ctx.font_extents()
			
			local rx, ry = (sw - te.width) / 2 + te.x_bearing, (sh - fe.height) / 2 + fe.ascent
			assert(not vertical[line.style], "Can't handle vertical kara in syllable highlight code - poke jfs if you need this")
			
			ctx.move_to(rx, ry)
			ctx.text_path(syl.text)
			
			local path = ctx.copy_path()
			local function modpath(x, y)
				local cx = math.sin(y/sh*math.pi)
				local cy = math.sin(x/sw*math.pi)
				cx = cx * x + (1-cx)/2*sw
				cy = cy * y + (1-cy)/2*sh
				return fade*x+(1-fade)*cx, fade*y+(1-fade)*cy
			end
			path.map_coords(modpath)
			ctx.new_path()
			ctx.append_path(path)
			
			local c2, c3 = color2[line.style], color3[line.style]
			
			for b = 8, 1, -3 do
				local bs = cairo.image_surface_create(sw, sh, "argb32")
				local bc = bs.create_context()
				bc.set_source_rgba(c2.r, c2.g, c2.b, c2.a)
				bc.append_path(path)
				bc.fill()
				raster.gaussian_blur(bs, b)
				local bo = {}
				bo.surf = bs
				bo.x = syl.center_x - sw/2
				bo.y = syl.center_y - sh/2
				bo.alpha = fade
				bo.operator = "add"
				bo.name = "blur " .. b
				table.insert(frame_overlays, bo)
			end
			
			ctx.set_source_rgba(c3.r, c3.g, c3.b, c3.a)
			ctx.set_line_join("round")
			ctx.set_operator("over")
			ctx.set_line_width(3*fade)
			ctx.stroke_preserve()
			ctx.set_operator("dest_out")
			ctx.fill()
			raster.box_blur(surf, 3, 2)
			
			local so = {}
			so.surf = surf
			so.x = syl.center_x - sw/2
			so.y = syl.center_y - sh/2
			so.alpha = 1
			so.operator = "over"
			so.name = string.format("bord %s %.1f %.1f (%.1f,%.1f)", syl.text, sw, sh, rx, ry)
			table.insert(frame_overlays, so)
		end
	end
end

stylefunc.Romaji = stylefunc.generic
stylefunc.Kanji = stylefunc.generic
stylefunc.English = stylefunc.generic


-- Main rendering function
function render_frame(f, t)
	init(f)
	
	local surf = f.create_cairo_surface()
	local ctx = surf.create_context()
	ctx.scale(vres_x/sres_x, vres_y/sres_y)
	
	-- The line rendering functions add the mask of the line they rendered to
	-- this image. It will be used to draw the glow around all lines.
	-- It has to be done in this way to avoid the glows from nearby lines to
	-- interfere and produce double effect.
	noisemask = cairo.image_surface_create(sres_x, sres_y, "argb32")
	noisemaskctx = noisemask.create_context()
	-- Set to true as soon as anything is put into the noise mask
	-- This is merely an optimisation to avoid doing anything when there aren't
	-- any lines on screen.
	noisemaskfilled = false
	-- List of images to overlay on the video frame, after the noise mask.
	frame_overlays = {}
	
	for i, line in pairs(lines) do
		if stylefunc[line.style] then
			calc_line_metrics(ctx, line)
			stylefunc[line.style](t, line)
		end
	end
	
	if noisemaskfilled then
		-- Greenish and jittery version of the frame
		local noiseimg = f.create_cairo_surface()
		raster.box_blur(noiseimg, 5, 2)
		raster.pixel_value_map(noiseimg, "G rand 0.4 * + =G G 1 - 1 G ifgtz =G")
		-- Blurred version of the noisemask
		raster.gaussian_blur(noisemask, 8)
		-- Mask additive paint the noise mask: only show the area near the text
		-- and have it do interesting things with the video.
		ctx.set_source_surface(noiseimg, 0, 0)
		ctx.set_operator("add")
		ctx.mask_surface(noisemask, 0, 0)
	end
	
	-- Paint generated overlays onto the video.
	for i, o in pairs(frame_overlays) do
		ctx.set_source_surface(o.surf, o.x, o.y)
		ctx.set_operator(o.operator)
		ctx.paint_with_alpha(o.alpha)
	end
	
	f.overlay_cairo_surface(surf, 0, 0)
end

