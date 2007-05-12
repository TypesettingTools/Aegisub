--[[
 Copyright (c) 2007, Niels Martin Hansen, Rodrigo Braz Monteiro
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   * Neither the name of the Aegisub Group nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
]]

include("utils.lua")
include("unicode.lua")

-- Make sure karaskel table exists
if not karaskel then
	karaskel = {}
end

-- Collect styles and metadata from the subs
function karaskel.collect_head(subs, generate_furigana)
	local meta = { res_x = 0, res_y = 0 }
	local styles = { n = 0 }
	
	if not karaskel.furigana_scale then
		karaskel.furigana_scale = 0.5
	end
	
	local i = 1
	while i < #subs do
		if aegisub.progress.is_cancelled() then error("User cancelled") end
		local l = subs[i]
		
		if l.class == "style" then
			-- Store styles into the style table
			styles.n = styles.n + 1
			styles[styles.n] = l
			styles[l.name] = l
			l.margin_v = l.margin_t -- convenience
			
			-- And also generate furigana styles if wanted
			if generate_furigana and not l.name:match("furigana") then
				aegisub.debug.out(5, "Creating furigana style for style: " .. l.name)
				local fs = table.copy(l)
				fs.fontsize = l.fontsize * karaskel.furigana_scale
				fs.outline = l.outline * karaskel.furigana_scale
				fs.shadow = l.shadow * karaskel.furigana_scale
				fs.name = l.name .. "-furigana"
				
				styles.n = styles.n + 1
				styles[styles.n] = fs
				styles[fs.name] = fs
				-- TODO: also actually insert into file
			end
			
		elseif l.class == "info" then
			-- Also look for script resolution
			local k = l.key:lower()
			if k == "playresx" then
				meta.res_x = math.floor(l.value)
			elseif k == "playresy" then
				meta.res_y = math.floor(l.value)
			end
		end
		
		i = i + 1
	end
	
	-- Fix missing resolution data
	if meta.res_x == 0 and meta_res_y == 0 then
		meta.res_x = 384
		meta.res_y = 288
	elseif meta.res_x == 0 then
		-- This is braindead, but it's how TextSub does things...
		if meta.res_y == 1024 then
			meta.res_x = 1280
		else
			meta.res_x = meta.res_y / 3 * 4
		end
	elseif meta.res_y == 0 then
		-- As if 1280x960 didn't exist
		if meta.res_x == 1280 then
			meta.res_y = 1024
		else
			meta.res_y = meta.res_x * 3 / 4
		end
	end
	
	return meta, styles
end


-- Pre-process line, determining stripped text, karaoke data and splitting off furigana data
-- Modifies the object passed for line
function karaskel.preproc_line_text(meta, styles, line)
	-- Assume line is class=dialogue
	local kara = aegisub.parse_karaoke_data(line)
	line.kara = { n = 0 }
	line.furi = { n = 0 }
	
	line.text_stripped = ""
	line.duration = line.end_time - line.start_time

	local worksyl = { }
	local cur_inline_fx = ""
	for i = 0, #kara do
		local syl = kara[i]
		
		-- Detect any inline-fx tags
		local inline_fx = syl.text:match("%{.*\\%-(.-)[}\\]")
		if inline_fx then
			cur_inline_fx = inline_fx
		end
		
	end
end


-- Pre-calculate positioning information for the given line, also layouting furigana text if needed
-- Modifies the object passed for line
function karaskel.preproc_line_pos(meta, styles, line)
end


-- Precalc some info on a line
-- Modifies the line parameter
function karaskel.preproc_line(subs, meta, styles, line)
	-- Assume line is class=dialogue
	local kara = aegisub.parse_karaoke_data(line)
	line.kara = { n = 0 }
	line.furi = { n = 0 }
	
	if styles[line.style] then
		line.styleref = styles[line.style]
	else
		aegisub.debug.out(2, "WARNING: Style not found: " .. line.style .. "\n")
		line.styleref = styles[1]
	end
	
	line.text_stripped = ""
	line.duration = line.end_time - line.start_time
	
	local curx = 0
	local worksyl = { }
	for i = 0, #kara do
		local syl = kara[i]
		
		-- Spaces at the start and end of the syllable are best ignored
		local prespace, syltext, postspace = syl.text_stripped:match("^([ \t]*)(.-)([ \t]*)$")

		local prefix = syltext:sub(1,unicode.charwidth(syltext,1))
		if prefix ~= "#" and prefix ~= "＃" and i > 0 then
			line.kara[line.kara.n] = worksyl
			line.kara.n = line.kara.n + 1
			worksyl = { }
		end

		-- Check if there is a chance of furigana
		-- FIXME: multi-highlights aren't being generated, are they? At least not with syllables with a lone # in
		if syltext:find("|") or syltext:find("｜") then
			syltext = syltext:gsub("｜", "|")
			local maintext, furitext = syl:match("^(.-)|(.-)$")
			syltext = maintext
			
			local furi = { }
			furi.syl = worksyl
			
			local prefix = furitext:sub(1,unicode.charwidth(furitext,1))
			if prefix == "!" or prefix == "！" then
				furi.isbreak = true
				furi.spillback = false
			elseif prefix == "<" or prefix == "＜" then
				furi.isbreak = true
				furi.spillback = true
			else
				furi.isbreak = false
				furi.spillback = false
			end
			if furi.isbreak then
				furitext = furitext:sub(unicode.charwidth(furitext,1)+1)
			end
			
			furi.start_time = syl.start_time
			furi.end_time = syl.end_time
			furi.duration = syl.duration
			furi.text = furitext
			
			line.furi.n = line.furi.n + 1
			line.furi[line.furi.n] = furi
		end
		
		-- If this is the start of a highlight group, do regular processing
		if prefix ~= "#" and prefix ~= "＃" then
			-- Update stripped line-text
			line.text_stripped = line.text_stripped .. syl.text_stripped
			
			-- Copy data from syl to worksyl
			worksyl.text = syl.text
			worksyl.duration = syl.duration
			worksyl.kdur = syl.duration / 10
			worksyl.start_time = syl.start_time
			worksyl.end_time = syl.end_time
			worksyl.tag = syl.tag
			worksyl.line = line
			worksyl.style = line.styleref
			
			-- And add new data to worksyl
			worksyl.i = line.kara.n
			worksyl.text_stripped = syltext
			worksyl.width = aegisub.text_extents(line.styleref, syltext)
			curx = curx + aegisub.text_extents(line.styleref, prespace)
			worksyl.left = curx
			worksyl.center = curx + worksyl.width/2
			worksyl.right = curx + worksyl.width
			curx = curx + worksyl.width + aegisub.text_extents(line.styleref, postspace)
			
			-- TODO: inlinefx here
		end
		
		-- And in either case, add highlight data
		local hl = {
			start_time = worksyl.start_time,
			end_time = worksyl.end_time,
			duration = worksyl.duration
		}
		worksyl.highlights = { n = 1, [1] = hl }
	end
	
	-- Add last syllable
	line.kara[line.kara.n] = worksyl
	
	-- Full line sizes
	line.width, line.height, line.descent, line.extlead = aegisub.text_extents(line.styleref, line.text_stripped)
	-- Effective margins
	line.margin_v = line.margin_t
	line.eff_margin_l = ((line.margin_l > 0) and line.margin_l) or line.styleref.margin_l
	line.eff_margin_r = ((line.margin_r > 0) and line.margin_r) or line.styleref.margin_r
	line.eff_margin_t = ((line.margin_t > 0) and line.margin_t) or line.styleref.margin_t
	line.eff_margin_b = ((line.margin_b > 0) and line.margin_b) or line.styleref.margin_b
	line.eff_margin_v = ((line.margin_v > 0) and line.margin_v) or line.styleref.margin_v
	-- And positioning
	if line.styleref.align == 1 or line.styleref.align == 4 or line.styleref.align == 7 then
		-- Left aligned
		line.left = line.eff_margin_l
		line.center = line.left + line.width / 2
		line.right = line.left + line.width
		line.x = line.left
		line.halign = "left"
	elseif line.styleref.align == 2 or line.styleref.align == 5 or line.styleref.align == 8 then
		-- Centered
		line.left = (meta.res_x - line.eff_margin_l - line.eff_margin_r - line.width) / 2 + line.eff_margin_l
		line.center = line.left + line.width / 2
		line.right = line.left + line.width
		line.x = line.center
		line.halign = "center"
	elseif line.styleref.align == 3 or line.styleref.align == 6 or line.styleref.align == 9 then
		-- Right aligned
		line.left = meta.res_x - line.eff_margin_r - line.width
		line.center = line.left + line.width / 2
		line.right = line.left + line.width
		line.x = line.right
		line.halign = "right"
	end
	line.hcenter = line.center
	if line.styleref.align >=1 and line.styleref.align <= 3 then
		-- Bottom aligned
		line.bottom = meta.res_y - line.eff_margin_b
		line.middle = line.bottom - line.height / 2
		line.top = line.bottom - line.height
		line.y = line.bottom
		line.valign = "bottom"
	elseif line.styleref.align >= 4 and line.styleref.align <= 6 then
		-- Mid aligned
		line.top = (meta.res_y - line.eff_margin_t - line.eff_margin_b) / 2 + line.eff_margin_t
		line.middle = line.top + line.height / 2
		line.bottom = line.top + line.height
		line.y = line.middle
		line.valign = "middle"
	elseif line.styleref.align >= 7 and line.styleref.align <= 9 then
		-- Top aligned
		line.top = line.eff_margin_t
		line.middle = line.top + line.height / 2
		line.bottom = line.top + line.height
		line.y = line.top
		line.valign = "top"
	end
	line.vcenter = line.middle
	
	-- Generate furigana style
	local furistyle = table.copy(line.styleref)
	furistyle.fontsize = furistyle.fontsize / 2
	furistyle.outline = furistyle.outline / 2
	
	-- Layout furigana
	for i = 1, line.furi.n do
	end
end


-- An actual "skeleton" function
-- Parses the first word out of the Effect field of each dialogue line and runs "fx_"..effect on that line
-- Lines with empty Effect field run fx_none
-- Lines with unimplemented effects are left alone
-- If the effect function returns true, the original line is kept in output,
-- otherwise the original line is converted to a comment
-- General prototype of an fx function: function(subs, meta, styles, line, fxdata)
-- fxdata are extra data after the effect name in the Effect field
function karaskel.fx_library_main(subs)
	aegisub.progress.task("Collecting header info")
	meta, styles = karaskel.collect_head(subs)
	
	aegisub.progress.task("Processing subs")
	local i, maxi = 1, #subs
	while i <= maxi do
		aegisub.progress.set(i/maxi*100)
		local l = subs[i]
		
		if l.class == "dialogue" then
			aegisub.progress.task(l.text)
			karaskel.preproc_line(subs, meta, styles, l)
			local keep = true
			local fx, fxdata = string.headtail(l.effect)
			if fx == "" then fx = "none" end
			if _G["fx_" .. fx] then
				-- note to casual readers: _G is a special global variable that points to the global environment
				-- specifically, _G["_G"] == _G
				keep = _G["fx_" .. fx](subs, meta, styles, l, fxdata)
			end
			if not keep then
				l = subs[i]
				l.comment = true
				subs[i] = l
			end
		end
		
		i = i + 1
	end
end
-- Register an fx_library type karaoke
karaskel.fx_library_registered = false
function karaskel.use_fx_library(macrotoo)
	if karaskel.fx_library_registered then return end
	aegisub.register_filter(script_name or "fx_library", script_description or "Apply karaoke effects (fx_library skeleton)", 2000, karaskel.fx_library_main)
	if macrotoo then
		local function fxlibmacro(subs)
			karaskel.fx_library_main(subs)
			aegisub.set_undo_point(script_name or "karaoke effect")
		end
		aegisub.register_macro(script_name or "fx_library", script_description or "Apply karaoke effects (fx_library skeleton)", fxlibmacro)
	end
end
