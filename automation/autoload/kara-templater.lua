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

-- Aegisub Automation 4 Lua karaoke templater tool
-- Parse and apply a karaoke effect written in ASS karaoke template language
-- See help file and wiki for more information on this

script_name = "Karaoke Templater"
script_description = "Macro and export filter to apply karaoke effects using the template language"
script_author = "Niels Martin Hansen"
script_version = 1


include("karaskel.lua")


-- Find and parse/prepare all karaoke template lines
function parse_templates(meta, styles, subs)
	local templates = { once = {}, line = {}, syl = {}, char = {}, furi = {} }
	for l = 1, #subs do
		local l = subs[i]
		if l.class == "dialogue" and l.comment then
			local fx, fxtail = string.headtail(l.effect)
			fx = fx:lower()
			if fx == "code" then
				parse_code(meta, styles, l, templates, fxtail)
			elseif fx == "template" then
				parse_template(meta, styles, l, templates, fxtail)
			end
		end
	end
end

function parse_code(meta, styles, line, templates, mods)
	local template = { code = line.text, loops = 1, all = false }
	local inserted = false

	local rest = mods
	while rest ~= "" do
		local m, t = string.headtail(rest)
		rest = t
		m = m:lower()
		if m == "once" then
			table.insert(templates.once, template)
			inserted = true
		elseif m == "line" then
			table.insert(templates.line, template)
			inserted = true
		elseif m == "syl" then
			table.insert(templates.syl, template)
			inserted = true
		elseif m == "char" then
			table.insert(templates.char, template)
			inserted = true
		elseif m == "furi" then
			table.insert(templates.furi, template)
			inserted = true
		elseif m == "all" then
			template.all = true
		elseif m == "repeat" then
			local times, t = string.headtail(rest)
			template.loops = tonumber(times)
			if not template.loops then
				aegisub.out(3, "Failed reading this repeat-count to a number: %s\nIn template code line: %s\nEffect field: %s\n\n", times, line.text, line.effect)
				template.loops = 1
			else
				rest = t
			end
		end
	end
	
	if not inserted then
		table.insert(templates.once, template)
	end
end

function parse_template(meta, styles, line, templates, mods)
end


-- Apply the templates
function apply_templates(meta, styles, subs, templates)
	-- the environment the templates will run in
	local tenv = {
		-- put in some standard libs
		string = string,
		math = math
	}
	
	-- run all run-once code snippets
	for k, t in pairs(templates.code) do
		
	end
end


-- Main function to do the templating
function filter_apply_templates(subs, config)
	aegisub.progress.task("Collecting header data...")
	local meta, styles = karaskel.collect_head(subs)
	
	aegisub.progress.task("Parsing templates...")
	local templates = parse_templates(meta, styles, subs)
	
	aegisub.progress.task("Applying templates...")
	apply_templates(meta, styles, subs, templates)
end

function macro_apply_templates(subs, sel)
	filter_apply_templates(subs, {ismacro=true, sel=sel})
	aegisub.set_undo_point("apply karaoke template")
end

function macro_can_template(subs)
	-- check if this file has templates in it, don't allow running the macro if it hasn't
	local num_dia = 0
	for i = 1, #subs do
		local l = subs[i]
		if l.class == "dialogue" then
			num_dia = num_dia + 1
			-- test if the line is a template
			if (string.headtail(l.effect)):lower() == "template" then
				return true
			end
			-- don't try forever, this has to be fast
			if num_dia > 50 then
				return false
			end
		end
	end
	return false
end

aegisub.register_macro("Apply karaoke template", "Applies karaoke effects from templates", macro_apply_templates, macro_can_template)
aegisub.register_filter("Karaoke temokate", "Apply karaoke effect templates to the subtitles", 2000, filter_apply_templates)
