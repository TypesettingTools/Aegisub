local tr = aegisub.gettext
script_name = tr("Select overlaps")
script_description = tr("Select lines which begin while another non-comment line is active")
script_author = "Thomas Goyne"
script_version = "2"
local select_overlaps
select_overlaps = function(subs)
  local dialogue = { }
  for i, line in ipairs(subs) do
    if line.class == "dialogue" then
      line.i = i
      table.insert(dialogue, line)
    end
  end
  table.sort(dialogue, function(a, b)
    return a.start_time < b.start_time or (a.start_time == b.start_time and a.i < b.i)
  end)
  local end_time = 0
  local overlaps = { }
  local _list_0 = dialogue
  for _index_0 = 1, #_list_0 do
    local line = _list_0[_index_0]
    if line.start_time >= end_time then
      end_time = line.start_time
    else
      table.insert(overlaps, line.i)
    end
  end
  return overlaps
end
return aegisub.register_macro(script_name, script_description, select_overlaps)
