-- Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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

local tr = aegisub.gettext

script_name = tr"Clean Script Info"
script_description = tr"Removes all but the absolutely required fields from the Script Info section. You might want to run this on files that you plan to distribute in original form."
script_author = "Thomas Goyne"
script_version = "1"

function clean_script_info(subs)
    local keep_keys = {
        ["collisions"] = true,
        ["playresx"] = true,
        ["playresy"] = true,
        ["scaledborderandshadow"] = true,
        ["scripttype"] = true,
        ["video colorspace"] = true,
        ["ycbcr matrix"] = true,
        ["wrapstyle"] = true
    }

    local deleted = 0
    for i = 1, #subs do
        local idx = i - deleted
        if subs[idx].class == "info" and not keep_keys[subs[idx].key:lower()] then
            subs.delete(idx)
            deleted = deleted + 1
        end
    end
end

function clean_script_info_macro(subs)
    clean_script_info(subs)
    aegisub.set_undo_point(script_name)
end

aegisub.register_macro(script_name, script_description, clean_script_info_macro)
aegisub.register_filter(script_name, script_description, 0, clean_script_info)


