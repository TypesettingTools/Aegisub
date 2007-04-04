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

-- Aegisub Automation 4 Lua karaoke skeleton

-- Compatibility hatch
if aegisub.lua_automation_version < 4 then
	include("karaskel.auto3")
	return
end

include("utils.lua")
include("unicode.lua")

-- Make sure karaskel table exists
if not karaskel then
	karaskel = {}
end

-- Collect styles and metadata from the subs
function karaskel.collect_head(subs)
	local meta = { res_x = 0, res_y = 0 }
	local styles = { n = 0 }
	
	for i = 1, #subs do
		local l = subs[i]
		if l.class == "style" then
			styles.n = styles.n + 1
			styles[styles.n] = l
			styles[l.name] = l
			l.margin_v = l.margin_t
		elseif l.class == "info" then
			local k = l.key:lower()
			if k == "playresx" then
				meta.res_x = math.floor(l.value)
			elseif k == "playresy" then
				meta.res_y = math.floor(l.value)
			end
		end
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
	elseif line.styleref.align == 2 or line.styleref.align == 5 or line.styleref.align == 8 then
		-- Centered
		line.left = (meta.res_x - line.eff_margin_l - line.eff_margin_r - line.width) / 2 + line.eff_margin_l
		line.center = line.left + line.width / 2
		line.right = line.left + line.width
		line.x = line.center
	elseif line.styleref.align == 3 or line.styleref.align == 6 or line.styleref.align == 9 then
		-- Right aligned
		line.left = meta.res_x - line.eff_margin_r - line.width
		line.center = line.left + line.width / 2
		line.right = line.left + line.width
		line.x = line.right
	end
	line.hcenter = line.center
	if line.styleref.align >=1 and line.styleref.align <= 3 then
		-- Bottom aligned
		line.bottom = meta.res_y - line.eff_margin_b
		line.middle = line.bottom - line.height / 2
		line.top = line.bottom - line.height
		line.y = line.bottom
	elseif line.styleref.align >= 4 and line.styleref.align <= 6 then
		-- Mid aligned
		line.top = (meta.res_y - line.eff_margin_t - line.eff_margin_b) / 2 + line.eff_margin_t
		line.middle = line.top + line.height / 2
		line.bottom = line.top + line.height
		line.y = line.middle
	elseif line.styleref.align >= 7 and line.styleref.align <= 9 then
		-- Top aligned
		line.top = line.eff_margin_t
		line.middle = line.top + line.height / 2
		line.bottom = line.top + line.height
		line.y = line.top
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
