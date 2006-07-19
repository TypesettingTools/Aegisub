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
-- "Basic \k replacer"

-- Use karaskel.lua for skeleton code
include("karaskel.lua")

-- Define the name of the script
name = "Basic \\k replacer"
-- Define a description of the script
description = "Makes basic karaoke effects. Replace all \\k tags (and variations) with a custom string, where some variables can be substituted in."
-- Define the script variables that can be configured graphically
-- This is the primary point of the script, being able to configure it graphically
configuration  = {
	-- First a label to descript what special variables can be used
	[1] = {
		name = "label1";
		kind = "label";
		label = [[Variable-names are prefixed with $,
expressions are enclosed in % pairs.
Variables:
  $START = Start-time of syllable (ms)
  $END = End-time of syllable (ms)
  $MID = Time midways through the syllable (ms)
  $DUR = Duration of syllable (cs)
Calculation example:
  \t($start,%$start+$dur*2%,\fscx110)
  \t(%$start+$dur*2%,$end,\fscx90)]];
		hint = ""
		-- No "default", since a label doesn't have a value
	},
	-- Then a text field to input the string to replace \k's with
	-- Make the default a "NOP" string
	[2] = {
		name = "k_repstr";
		kind = "text";
		label = "\\k replacement";
		hint = "The string to replace \\k tags with. Should start and end with { } characters.";
		default = "{\\k$DUR}"
	},
	-- Allow the user to specify whether to strip tags or not
	[3] = {
		name = "striptags";
		kind = "bool";
		label = "Strip all tags";
		hint = "Strip all formatting tags apart from the processed karaoke tags?";
		default = false
	},
	[4] = {
		name = "workstyle";
		kind = "style";
		label = "Line style";
		hint = "Only apply the effect to lines with this style. Empty means apply to all lines.";
		default = ""
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
	
	if syl.n == 0 then
		return text
	end
	
	-- Add the variable names to the syllable data
	syl["dur"] = syl.duration
	syl["start"] = syl.start_time
	syl["end"] = syl.end_time
	syl["mid"] = syl.start_time + syl.duration*5
	
	ktext = config.k_repstr
	
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
	
	-- Function for evaluating expressions
	local function expression_evaluator(expression)
		chunk, err = loadstring(string.format("return (%s)", expression))
		if (err) ~= nil then
			aegisub.output_debug(string.format("Error parsing expression:\n%s", expression, err))
			return "%" .. expression .. "%"
		else
			return chunk()
		end
	end
	-- Find and evaluate expressions
	ktext = string.gsub(ktext, "%%([^%%]*)%%", expression_evaluator)
	
	return ktext .. text
end

function do_line(meta, styles, config, line)
	if config.workstyle == "" or config.workstyle == line.style then
		return karaskel.do_line(meta, styles, config, line)
	else
		return { n=1, [1]=line }
	end
end
