-- Copyright (c) 2013, Thomas Goyne <plorkyeran@aegisub.org>
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

tr = aegisub.gettext

export script_name = tr"Select overlaps"
export script_description = tr"Select lines which begin while another non-comment line is active"
export script_author = "Thomas Goyne"
export script_version = "2"

select_overlaps = (subs) ->
    -- filter subtitles lines to just dialogue lines and sort them by time
    dialogue = {}
    for i, line in ipairs subs
        if line.class == "dialogue"
            line.i = i
            table.insert dialogue, line
    table.sort dialogue, (a,b) ->
        a.start_time < b.start_time or (a.start_time == b.start_time and a.i < b.i)

    end_time = 0
    overlaps = {}
    for line in *dialogue
        if line.start_time >= end_time
            end_time = line.end_time
        else
            table.insert overlaps, line.i
    overlaps

aegisub.register_macro script_name, script_description, select_overlaps
