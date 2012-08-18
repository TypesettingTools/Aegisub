-- Copyright (c) 2010, Thomas Goyne <plorkyeran@aegisub.org>
--
-- Permission to use, copy, modify, and distribute this software for any
-- purpose with or without fee is hereby granted, provided that the above
-- copyright notice and this permission notice appear in all copies.
--
-- THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
-- WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
-- MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
-- ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
-- WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
-- ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
-- OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
--
-- $Id$

local tr = aegisub.gettext

script_name = tr"Select overlaps"
script_description = tr"Select lines which begin while another non-comment line is active"
script_author = "Thomas Goyne"
script_version = "1"

function select_overlaps(subs)
    -- filter subtitles lines to just dialogue lines and sort them by time
    local dialogue = {}
    for i = 1,#subs do
        local line = subs[i]
        if line.class == "dialogue" then
            line.i = i - 1
            table.insert(dialogue, line)
        end
    end
    table.sort(dialogue, function(a,b) return a.start_time < b.start_time end)

    local end_time = 0
    local overlaps = {}
    for i,line in ipairs(dialogue) do
        if line.start_time >= end_time then
            end_time = line.end_time
        else
            table.insert(overlaps, line.i)
        end
    end
    return overlaps
end

aegisub.register_macro(script_name, script_description, select_overlaps)
