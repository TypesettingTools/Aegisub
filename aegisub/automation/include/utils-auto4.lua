--[[
 Copyright (c) 2005-2010, Niels Martin Hansen, Rodrigo Braz Monteiro
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

-- Make a shallow copy of a table
function table.copy(oldtab)
	local newtab = {}
	for key, val in pairs(oldtab) do
		newtab[key] = val
	end
	return newtab
end
-- Compability
copy_line = table.copy

-- Make a deep copy of a table
-- Retains equality of table references inside the copy and handles self-referencing structures
function table.copy_deep(srctab)
	-- Table to hold subtables already copied, to avoid circular references causing infinite recursion
	local circular = {}
	local function do_copy(oldtab)
		-- Check if we know the source already
		if circular[oldtab] then
			-- Use already-made copy
			return circular[oldtab]
		else
			-- Prepare a new table to copy into
			local newtab = {}
			-- Register it as known
			circular[oldtab] = newtab
			-- Copy fields
			for key, val in pairs(oldtab) do
				-- Copy tables recursively, everything else normally
				if type(val) == "table" then
					newtab[key] = do_copy(val)
				else
					newtab[key] = val
				end
			end
			return newtab
		end
	end
	return do_copy(srctab)
end

-- Generates ASS hexadecimal string from R,G,B integer components, in &HBBGGRR& format
function ass_color(r,g,b)
	return string.format("&H%02X%02X%02X&",b,g,r)
end
-- Format an alpha-string for \Xa style overrides
function ass_alpha(a)
	return string.format("&H%02X&", a)
end
-- Format an ABGR string for use in style definitions (these don't end with & either)
function ass_style_color(r,g,b,a)
	return string.format("&H%02X%02X%02X%02X",a,b,g,r)
end

-- Extract colour components of an ASS colour
function extract_color(s)
	local a, b, g, r
	
	-- Try a style first
	a, b, g, r = s:match("&H(%x%x)(%x%x)(%x%x)(%x%x)")
	if a then
		return tonumber(r, 16), tonumber(g, 16), tonumber(b, 16), tonumber(a, 16)
	end
	
	-- Then a colour override
	b, g, r = s:match("&H(%x%x)(%x%x)(%x%x)&")
	if b then
		return tonumber(r, 16), tonumber(g, 16), tonumber(b, 16), 0
	end
	
	-- Then an alpha override
	a = s:match("&H(%x%x)&")
	if a then
		return 0, 0, 0, tonumber(a, 16)
	end
	
	-- Ok how about HTML format then?
	r, g, b, a = s:match("#(%x%x)(%x%x)?(%x%x)?(%x%x)?")
	if r then
		return tonumber(r or 0, 16), tonumber(g or 0, 16), tonumber(b or 0, 16), tonumber(a or 0, 16)
	end
	
	-- Failed...
	return nil
end

-- Create an alpha override code from a style definition colour code
function alpha_from_style(scolor)
	local r, g, b, a = extract_color(scolor)
	return ass_alpha(a or 0)
end

-- Create an colour override code from a style definition colour code
function color_from_style(scolor)
	local r, g, b = extract_color(scolor)
	return ass_color(r or 0, g or 0, b or 0)
end

-- Converts HSV (Hue, Saturation, Value)  to RGB
function HSV_to_RGB(H,S,V)
	local r,g,b = 0,0,0

	-- Saturation is zero, make grey
	if S == 0 then
		r = V*255
		if r < 0 then
			r = 0
		end
		if r > 255 then
			r = 255
		end
		g = r
		b = r
		
	-- Else, calculate color
	else
		-- Calculate subvalues
		H = H % 360 -- Put H in range [0,360)
		local Hi = math.floor(H/60)
		local f = H/60.0 - Hi
		local p = V*(1-S)
		local q = V*(1-f*S)
		local t = V*(1-(1-f)*S)
		
		-- Do math based on hue index
		if Hi == 0 then
			r = V*255.0
			g = t*255.0
			b = p*255.0
		elseif Hi == 1 then
			r = q*255.0
			g = V*255.0
			b = p*255.0
		elseif Hi == 2 then
			r = p*255.0
			g = V*255.0
			b = t*255.0
		elseif Hi == 3 then
			r = p*255.0
			g = q*255.0
			b = V*255.0
		elseif Hi == 4 then
			r = t*255.0
			g = p*255.0
			b = V*255.0
		elseif Hi == 5 then
			r = V*255.0
			g = p*255.0
			b = q*255.0
		else
			aegisub.debug.out(2, "RGB_to_HSV: Hi got an unexpected value: %d\n\n", Hi)
		end
	end
	
	return r,g,b
end

-- Convert HSL (Hue, Saturation, Luminance) to RGB
-- Contributed by Gundamn
function HSL_to_RGB(H, S, L)
	local r, g, b;
	
	-- Make sure input is in range
	H = H % 360
	S = clamp(S, 0, 1)
	L = clamp(L, 0, 1)

	if S == 0 then
		-- Simple case if saturation is 0, all grey
		r = L
		g = L
		b = L
		
	else
		-- More common case, saturated colour
		if L < 0.5 then
			Q = L * (1.0 + S)
		else
			Q = L + S - (L * S)
		end

		local P = 2.0 * L - Q

		local Hk = H / 360

		local Tr, Tg, Tb
		if Hk < 1/3 then
			Tr = Hk + 1/3
			Tg = Hk
			Tb = Hk + 2/3
		elseif Hk > 2/3 then
			Tr = Hk - 2/3
			Tg = Hk
			Tb = Hk - 1/3
		else
			Tr = Hk + 1/3
			Tg = Hk
			Tb = Hk - 1/3
		end
		
		local function get_component(T)
			if T < 1/6 then
				return P + ((Q - P) * 6.0 * T)
			elseif 1/6 <= T and T < 1/2 then
				return Q
			elseif 1/2 <= T and T < 2/3 then
				return P + ((Q - P) * (2/3 - T) * 6.0)
			else
				return P
			end
		end
		
		r = get_component(Tr)
		g = get_component(Tg)
		b = get_component(Tb)

	end

	return math.floor(r*255+0.5), math.floor(g*255+0.5), math.floor(b*255+0.5)
end

-- Removes spaces at the start and end of string
function string.trim(s)
	return (string.gsub(s, "^%s*(.-)%s*$", "%1"))
end

-- Get the "head" and "tail" of a string, treating it as a sequence of words separated by one or more space-characters
function string.headtail(s)
	local a, b, head, tail = string.find(s, "(.-)%s+(.*)")
	if a then
		return head, tail
	else
		return s, ""
	end
end

-- Iterator function for headtail
function string.words(s)
	local t = s
	local function wordloop()
		if t == "" then
			return nil
		end
		local head, tail = string.headtail(t)
		t = tail
		return head
	end
	return wordloop, nil, nil
end

-- Clamp a number value to a range
function clamp(val, min, max)
	if val < min then
		return min
	elseif val > max then
		return max
	else
		return val
	end
end

-- Interpolate between two numbers
function interpolate(pct, min, max)
	if pct <= 0 then
		return min
	elseif pct >= 1 then
		return max
	else
		return pct * (max - min) + min
	end
end

-- Interpolate between two colour values, given in either style definition or style override format
-- Return in style override format
function interpolate_color(pct, first, last)
	local r1, g1, b1 = extract_color(first)
	local r2, g2, b2 = extract_color(last)
	local r, g, b = interpolate(pct, r1, r2), interpolate(pct, g1, g2), interpolate(pct, b1, b2)
	return ass_color(r, g, b)
end

-- Interpolate between two alpha values, given either in style override or as part as a style definition colour
-- Return in style override format
function interpolate_alpha(pct, first, last)
	local r1, g1, b1, a1 = extract_color(first)
	local r2, g2, b2, a2 = extract_color(last)
	return ass_alpha(interpolate(pct, a1, a2))
end
