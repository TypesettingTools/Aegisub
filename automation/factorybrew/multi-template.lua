--[[
 Copyright (c) 2005, Niels Martin Hansen
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

-- Aegisub Automation factory-brewed script
-- "Template-based multiple-lines-per-syllable karaoke effects"

-- Use karaskel.lua for skeleton code
include("karaskel-adv.lua")

-- Define the name of the script
name = "Template-based effects"
-- Define a description of the script
description = "Makes advanced effects using templates embedded into the subtitle script. Please see the help for information on how to use this script."
-- Only one config option here.
-- Everything else is defined in the subtitle script
configuration  = {
	-- Allow the user to specify whether to strip tags or not
	[1] = {
		name = "striptags";
		kind = "bool";
		label = "Strip all tags";
		hint = "Strip all formatting tags apart from the processed karaoke tags?";
		default = false
	},
	[2] = {
		name = "skipunknown";
		kind = "bool";
		label = "Remove unknown lines";
		hint = "Removes all lines without a defined template";
		default = false
	}
}
-- Mandatory values
version, kind= 3, 'basic_ass'

function do_syllable(meta, styles, config, line, syl)
	-- text is the replacement text for the syllable
	-- ktext is the karaoke effect string
	local text, ktext
	
	-- Prepare the stripped or unstripped main text
	if config.striptags then
		text = syl.text_stripped
	else
		text = syl.text
	end
	
	local result = { n=0 }
	
	-- Don't bother with empty syllables
	if syl.text == "" or syl.n == 0 then
		return result
	end
	
	-- Add the variable names to the syllable data
	syl["dur"] = syl.duration
	syl["start"] = syl.start_time
	syl["end"] = syl.end_time
	syl["mid"] = syl.start_time + syl.duration*5
	syl["x"] = syl.center + line.centerleft
	
	-- Prepare the karaoke effect string
	for templateid, template in template_data[line.style] do
		local ktext = template.text
		
		-- do_repeat is global on purpose
		_G.do_repeat = false
	
		repeat
			-- Function for replacing the variables
			local function var_replacer(varname)
				varname = string.lower(varname)
				if syl[varname] ~= nil then
					return syl[varname]
				else
					aegisub.output_debug(string.format("Unknown variable name: %s", varname))
					return "$" .. varname
				end
			end
			-- Replace the variables in the ktext
			ktext = string.gsub(ktext, "$(%a+)", var_replacer)
	
			local skipline = false
			local newline = copy_line(line)
			newline.layer = template.layer
			A = { meta = meta, styles = styles, line = line, syl = syl, newline = newline }

			-- Function for evaluating expressions
			local function expression_evaluator(expression)
				if string.find(expression, "return") then
					chunk, err = loadstring(expression)
				else
					chunk, err = loadstring(string.format("return (%s)", expression))
				end
				if (err) ~= nil then
					aegisub.output_debug(string.format("Error parsing expression:\n%s", expression, err))
					return "%" .. expression .. "%"
				else
					local result = chunk(meta, styles, line, syl, newline)
					if result then
						return result
					else
						skipline = true
						return ""
					end
				end
			end
			-- Find and evaluate expressions
			ktext = string.gsub(ktext, "%%([^%%]*)%%", expression_evaluator)
	
			newline.text = ktext .. text
			if not (kline == "" or skipline) then
				table.insert(result, newline)
			end
		until not do_repeat
	end
	
	return result
end

function do_line(meta, styles, config, line)
	if line.kind == "comment" and line.name == "template" then
		if not template_data[line.style] then
			template_data[line.style] = {}
		end
		table.insert(template_data[line.style], line)
		return { n=0 }
	elseif line.kind == "dialogue" and template_data[line.style] then
		return karaskel.do_line(meta, styles, config, line)
	elseif not config.skipunknown then
		return { n=1, [1]=line }
	else
		return { n=0 }
	end
	if config.workstyle == "" or config.workstyle == line.style then
		return adv_do_line(meta, styles, config, line)
	else
		return { n=1, [1]=line }
	end
end

function process_lines(meta, styles, lines, config)
	template_data = {}
	return karaskel.process_lines(meta, styles, lines, config)
end
