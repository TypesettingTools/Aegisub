local unicode
unicode = {
  charwidth = function(s, i)
    local b = s:byte(i or 1)
    if not b then
      return 1
    elseif b < 128 then
      return 1
    elseif b < 224 then
      return 2
    elseif b < 240 then
      return 3
    else
      return 4
    end
  end,
  chars = function(s)
    local curchar, i = 0, 1
    return function()
      if i > s:len() then
        return 
      end
      local j = i
      curchar = curchar + 1
      i = i + unicode.charwidth(s, i)
      return s:sub(j, i - 1), curchar
    end
  end,
  len = function(s)
    local n = 0
    for c in unicode.chars(s) do
      n = n + 1
    end
    return n
  end,
  codepoint = function(s)
    local b = s:byte(1)
    if b < 128 then
      return b
    end
    local res, w
    if b < 224 then
      res = b - 192
      w = 2
    elseif b < 240 then
      res = b - 224
      w = 3
    else
      res = b - 240
      w = 4
    end
    for i = 2, w do
      res = res * 64 + s:byte(i) - 128
    end
    return res
  end
}
return unicode
