--[[
 Copyright (c) 2005-2006, Niels Martin Hansen, Rodrigo Braz Monteiro
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

-- Compatibility hatch
if aegisub.lua_automation_version < 4 then
	include "utils.auto3"
	return
end

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
-- This will do infinite recursion if there's any circular references. (Eg. if you try to copy _G)
function table.copy_deep(srctab)
	-- Table to hold subtables already copied, to avoid circular references causing infinite recursion
	local circular = {}
	local function do_copy(oldtab)
		local newtab = {}
		for key, val in pairs(newtab) do
			if type(val) == "table" then
				if not circular[val] then
					circular[val] = do_copy(val)
				end
				newtab[key] = circular[val]
			else
				newtab[key] = val
			end
		end
	end
	return do_copy(srctab)
end

-- Generates ASS hexadecimal string from R,G,B integer components, in &HBBGGRR& format
function ass_color(r,g,b)
	return string.format("&H%02X%02X%02X&",b,g,r)
end

-- Converts HSV (Hue, Saturation, Value)  to RGB
function HSV_to_RGB(H,S,V)
	local r,g,b;

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
		end
	end
	
	r = math.floor(r)
	g = math.floor(g)
	b = math.floor(b)
	return r,g,b
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
