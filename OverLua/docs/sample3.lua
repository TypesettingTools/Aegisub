--[[

Sample script for OverLua
 - advanced karaoke effect, first version of Mendoi-Conclave Gundam 00 OP 1
 
Given into the public domain.
(You can do anything you want with this file, with no restrictions whatsoever.
 You don't get any warranties of any kind either, though.)
 
Originally authored by Niels Martin Hansen.

While I can't prevent you from it, please don't use this effect script
verbatim or almost-verbatim for own productions. It's mainly intended for
showing techniques, just using it without modifications or with only light
modifications is what I'd consider "cheap".

Be aware that this effect is very slow at rendering, at full 720p resolution
it takes around 3 hours to render on my dual 2.2 GHz Opteron.

This effect is called "OH NOES" by the way. No special meaning to that.

It's best read from bottom to top.

]]


-- Virtual resolution, 720p
local virtual_res_x = 1280
local virtual_res_y = 720
-- Font names
--local latin_font = "Eras Bold ITC"
local latin_font = "Briem Akademi Std Semibold"
local latin_weight = ""
local kanji_font = "DFGSoGei-W9"
-- Font sizes
local romaji_size = 34
local engrish_size = 36
local kanji_size = 30
local tl_size = 36
-- Text positions (vertical only, assumed centered)
local romaji_pos_y = 55
local tl_pos_y = virtual_res_y - 38
local kanji_pos_y = virtual_res_y - 27
local kanji_pos_x = virtual_res_x - 55
local engrish_pos_y = virtual_res_y - 38


timing_input_file = overlua_datastring
assert(timing_input_file, "OH NOES! Missing timing input file.")


-- Here's some mostly standard input file parsing functions

function parsenum(str)
	return tonumber(str) or 0
end
function parse_ass_time(ass)
	local h, m, s, cs = ass:match("(%d+):(%d+):(%d+)%.(%d+)")
	return parsenum(cs)/100 + parsenum(s) + parsenum(m)*60 + parsenum(h)*3600
end

function parse_k_timing(text)
	local syls = {}
	local cleantext = ""
	local i = 1
	for timing, syltext in text:gmatch("{\\k(%d+)}([^{]*)") do
		local syl = {dur = parsenum(timing)/100, text = syltext, i = i}
		local maintext, furitext = syltext:match("(.-)|(.+)")
		-- Note that there is a light support for Auto4 style furigana
		-- in this script, but I haven't maintained it since it ended up being
		-- unused.
		if maintext and furitext and furitext ~= "" then
			syl.text = maintext
			syl.furi = furitext
		end
		table.insert(syls, syl)
		cleantext = cleantext .. syl.text
		i = i + 1
	end
	return syls, cleantext
end

function read_input_file(name)
	for line in io.lines(name) do
		local start_time, end_time, style, fx, text = line:match("Dialogue: 0,(.-),(.-),(.-),,0000,0000,0000,(.-),(.*)")
		if text then
			local ls = {}
			ls.start_time = parse_ass_time(start_time)
			ls.end_time = parse_ass_time(end_time)
			ls.style = style
			ls.fx = fx
			ls.rawtext = text
			ls.kara, ls.cleantext = parse_k_timing(text)
			table.insert(lines, ls)
		end
	end
end

function init()
	if inited then return end
	inited = true
	
	lines = {}
	read_input_file(timing_input_file)
end


-- Calculate size and position of a line and its syllables
-- Only for horizontal lines, not vertical
function calc_line_metrics(ctx, line, font_name, font_size, pos_y)
	if line.pos_x then return end

	ctx.select_font_face(font_name, "", latin_weight)
	ctx.set_font_size(font_size)
	
	line.te = ctx.text_extents(line.cleantext)
	line.fe = ctx.font_extents()
	
	line.pos_x = (virtual_res_x - line.te.width) / 2 - line.te.x_bearing
	line.pos_y = pos_y
	
	if #line.kara < 2 then return end
	
	local curx = line.pos_x
	for i, syl in pairs(line.kara) do
		syl.te = ctx.text_extents(syl.text)
		syl.pos_x = curx
		syl.center_x = curx + syl.te.x_bearing + syl.te.width/2
		syl.center_y = pos_y - line.fe.ascent/2 + line.fe.descent/2
		curx = curx + syl.te.x_advance
		
		if syl.furi then
			ctx.set_font_size(font_size/2)
			syl.furite = ctx.text_extents(syl.furi)
			syl.furife = ctx.font_extents()
			ctx.set_font_size(font_size)
			syl.furi_x = syl.center_x - syl.furite.width/2 - syl.furite.x_bearing
			syl.furi_y = pos_y - line.fe.height
		end
	end
end

-- Paint the image of a line of text to a cairo context
-- Assumes the current path in the context is of the text to be painted
function paint_text(surf, ctx)
	ctx.set_line_join("round")
	ctx.set_source_rgba(0, 0.2, 0.3, 0.8)
	ctx.set_line_width(3)
	ctx.stroke_preserve()
	raster.gaussian_blur(surf, 1.7)
	ctx.set_source_rgba(1, 1, 1, 0.95)
	ctx.fill()
end


-- Render one of the zoomed circles with some parameters
-- width and height are of the source area to be visible in the zoomed image
-- Some of this is a bit hacked, I just changed stuff around until it worked,
-- honestly. Analyse it if you want, it still doesn't fully make sense to me ;)
function make_zoomed_ellipsis(srcsurf, center_x, center_y, width, height)
	local factor = 0.7
	
	local target_width, target_height = math.ceil(width/factor), math.ceil(height/factor)
	
	local target = cairo.image_surface_create(target_width, target_height, "argb32")
	local targetctx = target.create_context()
	
	local src_x, src_y = center_x - width/2, center_y - height/2
	
	-- The basic premise is just taking the source surface, making an upscaling
	-- pattern of it and fill a circle with the correct portion of it.
	-- Actually pretty simple, it's just getting the numbers right.
	local srcpat = cairo.pattern_create_for_surface(srcsurf)
	srcpat.set_extend("none")
	local srcpatmatrix = cairo.matrix_create()
	srcpatmatrix.init_translate(src_x, src_y)
	srcpatmatrix.scale(factor, factor)
	srcpat.set_matrix(srcpatmatrix)
	
	targetctx.scale(target_width, target_height)
	targetctx.arc(0.5, 0.5, 0.5, 0, math.pi*2)
	targetctx.scale(1/target_width, 1/target_height)
	targetctx.set_source(srcpat)
	targetctx.fill()
	
	return target, target_width, target_height
end


-- Duration in seconds for the fade-in/-outs
local fadeinoutdur = 1.2


-- Paint a complete line of karaoke text with all effects, except the
-- zoom circles, to a context. It depends on l.textsurf containing the line
-- image.
-- The main attraction here is the fade-over effect.
function paint_kara_text(f, ctx, t, l)
	local fade, fademask, fadetype
	-- Check if we're fading in?
	if t < l.start_time + fadeinoutdur and l.fx ~= "nofadein" then
		-- Calculate the position of the fade
		fade = 1 - (l.start_time - t + fadeinoutdur/2) / fadeinoutdur
		-- Create a gradient pattern that shows only the relevant part of
		-- the line for the fade.
		fademask = cairo.pattern_create_linear(virtual_res_x*fade, virtual_res_y/2, virtual_res_x*fade - 100, virtual_res_y/2-30)
		fademask.add_color_stop_rgba(0, 1, 1, 1, 0)
		fademask.add_color_stop_rgba(0.05, 1, 1, 1, 1)
		fademask.add_color_stop_rgba(0.3, 1, 1, 1, 0.2)
		fademask.add_color_stop_rgba(1, 1, 1, 1, 1)
		fadetype = "in"
	end
	-- Or fading out?
	if l.end_time - fadeinoutdur <= t and l.fx ~= "last" and l.fx~= "nofadeout" then
		-- Pretty much the same as for fade in, except that a different part of
		-- the line is shown by the produced pattern
		fade = (t - l.end_time + fadeinoutdur/2) / fadeinoutdur
		fademask = cairo.pattern_create_linear(virtual_res_x*fade, virtual_res_y/2, virtual_res_x*fade + 100, virtual_res_y/2+30)
		fademask.add_color_stop_rgba(0, 1, 1, 1, 0)
		fademask.add_color_stop_rgba(0.05, 1, 1, 1, 1)
		fademask.add_color_stop_rgba(0.3, 1, 1, 1, 0.2)
		fademask.add_color_stop_rgba(1, 1, 1, 1, 1)
		fadetype = "out"
	end
	-- Is the line even visible?!
	if not fade and (t < l.start_time or l.end_time <= t) then return end
	
	-- A function that calculates the distance between a point and the fade
	-- The distance is calculated only along the X axis, so it's not the
	-- shortest distance from the point to the "fade line".
	-- Used to determine which side of the fade a point is on.
	local function fadedist(x, y) -- on X axis
		local fade_x_at_y = virtual_res_x*fade - (y - virtual_res_y/2) * 3/10
		if fadetype == "in" then
			return fade_x_at_y - x
		else
			return x - fade_x_at_y
		end
	end
	
	-- We'll be painting the surface with the image of the text
	ctx.set_source_surface(l.textsurf, 0, 0)
	if fade then
		-- So first paint the text with the fading-mask
		ctx.mask(fademask)
		
		-- Now generate a slightly different mask for the bloom effect
		-- This one goes "both ways", it's not restricted to just one direction;
		-- it gets limited later
		local bloommask = cairo.pattern_create_linear(virtual_res_x*fade - 200, virtual_res_y/2-60, virtual_res_x*fade + 200, virtual_res_y/2+60)
		bloommask.add_color_stop_rgba(0, 1, 1, 1, 0)
		bloommask.add_color_stop_rgba(0.5, 1, 1, 1, 1)
		bloommask.add_color_stop_rgba(1, 1, 1, 1, 0)
		local bloom = cairo.image_surface_create(virtual_res_x, virtual_res_y, "argb32")
		local bc = bloom.create_context()
		bc.set_source_surface(l.textsurf, 0, 0)
		bc.mask(fademask)
		-- Ok, this could be done in a faster way I bet... modify the colour of
		-- the bloom effect depending on whether it's a fade in or out,
		-- by running a pixel value mapping program over them.
		if fadetype == "out" then
			raster.pixel_value_map(bloom, "R 0.9 * =R  G 0.1 * =G  B 0.4 * =B")
		else
			raster.pixel_value_map(bloom, "R 0.22 * =R  G 0.45 * =G  B 0.44 * =B")
		end
		-- Now, three times, do an additive blending of a successively more
		-- blurred version of the masked text.
		-- Exploit that the text border is very dark, so it won't contribute
		-- much at all to the overall result.
		-- If the border was brighter a different image of the text would need
		-- to be used instead.
		-- This is what *really* kills the rendering speed!
		ctx.set_operator("add")
		raster.gaussian_blur(bloom, 3)
		ctx.set_source_surface(bloom, 0, 0)
		ctx.mask(bloommask)
		raster.gaussian_blur(bloom, 3)
		ctx.set_source_surface(bloom, 0, 0)
		ctx.mask(bloommask)
		raster.gaussian_blur(bloom, 3)
		ctx.set_source_surface(bloom, 0, 0)
		ctx.mask(bloommask)
		ctx.set_operator("over")
	else
		-- We aren't fading, just do a plain paint of the text image
		ctx.paint()
	end
	
	return fade, fademask, fadetype, fadedist
end


-- Line style processing functions
-- The entries in this table are matched with the line Style fields to pick
-- an appropriate handling function for the line.
stylefunc = {}

-- This is a generic handling function called by other functions
function stylefunc.generic(f, ctx, t, l, font_name, font_size, pos_y)
	-- Fast return for irrelevant lines
	if t < l.start_time - fadeinoutdur/2 then return end
	if l.end_time + fadeinoutdur/2 <= t then return end

	-- Make sure we have the positioning information for the line
	calc_line_metrics(ctx, l, font_name, font_size, pos_y)
	
	-- If it's the first time this line is processed, generate the image of it
	if not l.textsurf then
		-- Create surface for the text image
		local textsurf = cairo.image_surface_create(virtual_res_x, virtual_res_y, "argb32")
		local c = textsurf.create_context()
		
		-- Fill it with a path of the text
		c.select_font_face(font_name, "", latin_weight)
		c.set_font_size(font_size)
		
		c.move_to(l.pos_x, l.pos_y)
		c.text_path(l.cleantext)
		
		for i, syl in pairs(l.kara) do
			if syl.furi then
				c.set_font_size(kanji_size/2)
				c.move_to(syl.furi_x, syl.furi_y)
				c.text_path(syl.furi)
			end
		end
		
		paint_text(textsurf, c)
		
		l.textsurf = textsurf
	end
	
	-- Check if we're on the last line which needs the "fade all out" effect
	if l.fx == "last" and t > l.end_time - 1.5 then
		fade_all_out = (l.end_time - t) / 1.5
	else
		fade_all_out = nil
	end
	
	-- Put the actual text onto the video image
	local fade, fademask, fadetype, fadedist = paint_kara_text(f, ctx, t, l)
	
	-- Search for a currently highlighted syllable in the text
	local sumdur = l.start_time
	local cursyl = -1
	for i, syl in pairs(l.kara) do
		syl.start_time = sumdur
		if t >= sumdur and t < sumdur+syl.dur then
			cursyl = i
		end
		sumdur = sumdur + syl.dur
	end
	
	if cursyl >= 1 then
		-- There is a current syllable
		-- Figure out where to put the zoom circle
		local syl = l.kara[cursyl]
		-- Assume it's at the center of the syllable for now
		local zoompoint = {
			cx = syl.center_x,
			cy = syl.center_y,
			size = math.max(syl.te.width, syl.te.height)
		}
		-- But check if we're time-wise close enough to the previous syllable
		-- (if there is one) to do a transition from it
		local prevsyl
		if cursyl >= 2 then
			local prevsyli = cursyl - 1
			repeat
				prevsyl = l.kara[prevsyli]
				prevsyli = prevsyli - 1
			until (prevsyl.dur > 0)
			if syl.dur > 0.100 and t - syl.start_time < 0.100 then
				local pcx, pcy = prevsyl.center_x, prevsyl.center_y
				local psize = math.max(prevsyl.te.width, prevsyl.te.height)
				local v = (t - syl.start_time) / 0.100
				local iv = 1 - v
				zoompoint.cx = iv * pcx + v * zoompoint.cx
				zoompoint.cy = iv * pcy + v * zoompoint.cy
				zoompoint.size = iv * psize + v * zoompoint.size
			end
		elseif cursyl == 1 and syl.dur > 0.100 and t - syl.start_time < 0.100 then
			zoompoint.size = zoompoint.size * (t - syl.start_time) / 0.100
		end
		zoompoint.size = zoompoint.size * 1.1
		-- Check that we aren't fading over and that the center of the zoom is
		-- not outside the visible part of the line.
		if not fade or fadedist(zoompoint.cx, zoompoint.cy) > 0 then
			-- Insert (enable) the zoom point then
			table.insert(zoompoints, zoompoint)
		end
	end
end

-- The Romaji and Engrish styles are both the same generic thing
function stylefunc.Romaji(f, ctx, t, l)
	stylefunc.generic(f, ctx, t, l, latin_font, romaji_size, romaji_pos_y)
end

-- Engrish was used for the somewhat-English lines in the original lyrics
-- (I.e. not for the translation.)
function stylefunc.Engrish(f, ctx, t, l)
	stylefunc.generic(f, ctx, t, l, latin_font, engrish_size, engrish_pos_y)
end

-- The vertical kanji need a rather different handling
function stylefunc.Kanji(f, ctx, t, l)
	-- Again, check for fast skip
	if t < l.start_time - fadeinoutdur/2 then return end
	if l.end_time + fadeinoutdur/2 <= t then return end

	-- Mostly the same as for the generic handling, except that we also
	-- calculate the metrics here.
	if not l.textsurf then
		local textsurf = cairo.image_surface_create(virtual_res_x, virtual_res_y, "argb32")
		local c = textsurf.create_context()
		
		c.select_font_face("@"..kanji_font)
		c.set_font_size(kanji_size)
		
		l.te = c.text_extents(l.cleantext)
		l.fe = c.font_extents()
		
		l.pos_x = kanji_pos_x
		l.pos_y = (virtual_res_y - l.te.width) / 2 - l.te.x_bearing
		
		local cury = l.pos_y
		for i, syl in pairs(l.kara) do
			syl.te = c.text_extents(syl.text)
			syl.pos_y = cury
			syl.center_y = cury + syl.te.x_bearing + syl.te.width/2
			syl.center_x = kanji_pos_x + l.fe.ascent/2 - l.fe.descent/2
			cury = cury + syl.te.x_advance
		end
		
		c.translate(l.pos_x, l.pos_y)
		c.rotate(math.pi/2)
		c.move_to(0,0)
		c.text_path(l.cleantext)
		
		paint_text(textsurf, c)
		
		l.textsurf = textsurf
	end

	local fade, fademask, fadetype, fadedist = paint_kara_text(f, ctx, t, l)
	
	-- Lots of copy-paste (code re-use!) here, slightly adapted for vertical
	-- text rather than horizontal stuff.
	local sumdur = l.start_time
	local cursyl = -1
	for i, syl in pairs(l.kara) do
		syl.start_time = sumdur
		if t >= sumdur and t < sumdur+syl.dur then
			cursyl = i
		end
		sumdur = sumdur + syl.dur
	end
	
	if cursyl >= 1 then
		local syl = l.kara[cursyl]
		local zoompoint = {
			cx = syl.center_x,
			cy = syl.center_y,
			size = math.max(syl.te.width, syl.te.height)
		}
		local prevsyl
		if cursyl >= 2 then
			local prevsyli = cursyl - 1
			repeat
				prevsyl = l.kara[prevsyli]
				prevsyli = prevsyli - 1
			until (prevsyl.dur > 0)
			if syl.dur > 0.100 and t - syl.start_time < 0.100 then
				local pcx, pcy = prevsyl.center_x, prevsyl.center_y
				local psize = math.max(prevsyl.te.width, prevsyl.te.height)
				local v = (t - syl.start_time) / 0.100
				local iv = 1 - v
				zoompoint.cx = iv * pcx + v * zoompoint.cx
				zoompoint.cy = iv * pcy + v * zoompoint.cy
				zoompoint.size = iv * psize + v * zoompoint.size
			end
		elseif cursyl == 1 and syl.dur > 0.100 and t - syl.start_time < 0.100 then
			zoompoint.size = zoompoint.size * (t - syl.start_time) / 0.100
		end
		zoompoint.size = zoompoint.size * 1.1
		if not fade or fadedist(zoompoint.cx, zoompoint.cy) > 0 then
			table.insert(zoompoints, zoompoint)
		end
	end
end

-- The translation lines get a somewhat simplified handling again.
-- Originally separated out because some translated lines were split into two
-- stacked lines, but that was dropped again.
function stylefunc.TL(f, ctx, t, l)
	if t < l.start_time - fadeinoutdur/2 then return end
	if l.end_time + fadeinoutdur/2 <= t then return end
	
	local line1, line2 = l.rawtext, l.rawtext:find("\\n", 1, true)
	if line2 then
		line1 = l.rawtext:sub(line2+2)
		line2 = l.rawtext:sub(1, line2-1)
	else
		line2 = ""
	end
	
	if not l.textsurf then
		local textsurf = cairo.image_surface_create(virtual_res_x, virtual_res_y, "argb32")
		local c = textsurf.create_context()
		
		c.select_font_face(latin_font, "", latin_weight)
		c.set_font_size(tl_size)
		
		l.te1 = c.text_extents(line1)
		l.te2 = c.text_extents(line2)
		l.fe = c.font_extents()
		
		l.pos1_x = (virtual_res_x - l.te1.width) / 2 - l.te1.x_bearing
		l.pos2_x = (virtual_res_x - l.te2.width) / 2 - l.te2.x_bearing
		l.pos1_y = tl_pos_y
		l.pos2_y = tl_pos_y - l.fe.height
		
		c.move_to(l.pos1_x, l.pos1_y)
		c.text_path(line1)
		c.move_to(l.pos2_x, l.pos2_y)
		c.text_path(line2)
		
		paint_text(textsurf, c)
		
		l.textsurf = textsurf
	end

	paint_kara_text(f, ctx, t, l)
end


-- Paint a zoom circle onto the video
-- zp is one of the zoompoint structures generated in the style functions
function draw_zoompoint(surf, ctx, t, zp)
	if zp.size < 5 then return end

	local zoom, zoom_width, zoom_height = make_zoomed_ellipsis(surf, zp.cx, zp.cy, zp.size*1.2, zp.size*1.2)
	
	local glow = cairo.image_surface_create(zoom_width+50, zoom_height+50, "argb32")
	local gc = glow.create_context()

	-- Hue-rotation
	-- Based on HSL-to-RGB code from Aegisub
	local r, g, b
	local cspeed = 1/5
	local sat = 69
	local q = math.floor((cspeed*t) % 6)
	local qf = ((cspeed*t) % 6 - q) * (255-sat)
	if q == 0 then
		r = 255
		g = sat + qf
		b = sat
	elseif q == 1 then
		r = sat + 255 - qf
		g = 255
		b = sat
	elseif q == 2 then
		r = sat
		g = 255
		b = sat + qf
	elseif q == 3 then
		r = sat
		g = sat + 255 - qf
		b = 255
	elseif q == 4 then
		r = sat + qf
		g = qf
		b = 255
	elseif q == 5 then
		r = 255
		g = sat
		b = qf + 255 - qf
	end
	-- Circle-tail-chaser thing
	-- Just a bunch of increasingly opaque lines drawn from a center
	-- and overlapping enough to create a sense of continuity.
	gc.set_line_width(6)
	for a = 0, 1, 1/zoom_height do
		gc.set_source_rgba(r/255, g/255, b/255, a)
		gc.move_to(zoom_width/2+25, zoom_height/2+25)
		gc.rel_line_to((zoom_width/2+5) * math.sin(-t*8-a*math.pi*2), (zoom_height/2+5) * math.cos(-t*8-a*math.pi*2))
		gc.stroke()
	end
	-- Love gaussian blur!
	raster.gaussian_blur(glow, 2)
	
	-- Use additive blend to put the tail-chaser onto the video
	ctx.set_source_surface(glow, zp.cx-zoom_width/2-25, zp.cy-zoom_height/2-25)
	local oldop = ctx.get_operator()
	ctx.set_operator("add")
	ctx.paint()
	ctx.set_operator(oldop)
	
	-- And regular blend for the zoom circle
	ctx.set_source_surface(zoom, zp.cx-zoom_width/2, zp.cy-zoom_height/2)
	ctx.paint()
end


function render_frame(f, t)
	-- Make sure we're initialised
	init()
	
	-- Clear the list of zoom points
	zoompoints = {}
	
	-- Create a surface and context from the video
	local worksurf = f.create_cairo_surface()
	local workctx = worksurf.create_context()
	-- This should make it possible to render on different resolution videos,
	-- but I don't think it works
	workctx.scale(f.width / virtual_res_x, f.height / virtual_res_y)
	
	-- Run over each input line, processing it
	-- This will draw the main text and transition effects
	for i, line in pairs(lines) do
		if stylefunc[line.style] then
			stylefunc[line.style](worksurf, workctx, t, line)
		end
	end
	
	-- Then go over the zoom points and draw those on top
	-- If this isn't done after all lines have been drawn, lines that are close
	-- to each other could end up overlapping each others' zoom circles.
	for i, zp in pairs(zoompoints) do
		draw_zoompoint(worksurf, workctx, t, zp)
	end
	
	-- If we're fading it all out, make the karaoke less visible by doing
	-- an alpha paint over with the original video frame.
	if fade_all_out then
		local vidsurf = f.create_cairo_surface()
		workctx.set_source_surface(vidsurf, 0, 0)
		workctx.paint_with_alpha(1-fade_all_out)
	end

	-- Finally put the video frame back
	f.overlay_cairo_surface(worksurf, 0, 0)
end
