local utils = {
  copy = function(tbl)
    return (function()
      local _tbl_0 = { }
      for k, v in pairs(tbl) do
        _tbl_0[k] = v
      end
      return _tbl_0
    end)()
  end,
  deep_copy = function(tbl)
    local seen = { }
    local copy
    copy = function(val)
      if type(tbl) ~= 'table' then
        return val
      end
      if seen[tbl] then
        return seen[val]
      end
      seen[val] = tbl
      return (function()
        local _tbl_0 = { }
        for k, v in pairs(val) do
          _tbl_0[k] = copy(v)
        end
        return _tbl_0
      end)()
    end
    return copy(tbl)
  end,
  ass_color = function(r, g, b)
    return string.format("&H%02X%02X%02X&", b, g, r)
  end,
  ass_alpha = function(a)
    return string.format("&H%02X&", a)
  end,
  ass_style_color = function(r, g, b, a)
    return string.format("&H%02X%02X%02X%02X", a, b, g, r)
  end,
  extract_color = function(s)
    local a, b, g, r
    a, b, g, r = s:match('&H(%x%x)(%x%x)(%x%x)(%x%x)')
    if a then
      return tonumber(r, 16), tonumber(g, 16), tonumber(b, 16), tonumber(a, 16)
    end
    b, g, r = s:match('&H(%x%x)(%x%x)(%x%x)&')
    if b then
      return tonumber(r, 16), tonumber(g, 16), tonumber(b, 16), 0
    end
    a = s:match('&H(%x%x)&')
    if a then
      return 0, 0, 0, tonumber(a, 16)
    end
    r, g, b, a = s:match('#(%x%x)(%x?%x?)(%x?%x?)(%x?%x?)')
    if r then
      return tonumber(r, 16), tonumber(g, 16) or 0, tonumber(b, 16) or 0, tonumber(a, 16) or 0
    end
  end,
  alpha_from_style = function(scolor)
    return ass_alpha(select(4, extract_color(scolor)))
  end,
  color_from_style = function(scolor)
    local r, g, b = extract_color(scolor)
    return ass_color(r or 0, g or 0, b or 0)
  end,
  HSV_to_RGB = function(H, S, V)
    local r, g, b = 0, 0, 0
    if S == 0 then
      r = self:clamp(V * 255, 0, 255)
      g = r
      b = r
    else
      H = math.abs(H) % 360
      local Hi = math.floor(H / 60)
      local f = H / 60.0 - Hi
      local p = V * (1 - S)
      local q = V * (1 - f * S)
      local t = V * (1 - (1 - f) * S)
      if Hi == 0 then
        r = V * 255.0
        g = t * 255.0
        b = p * 255.0
      elseif Hi == 1 then
        r = q * 255.0
        g = V * 255.0
        b = p * 255.0
      elseif Hi == 2 then
        r = p * 255.0
        g = V * 255.0
        b = t * 255.0
      elseif Hi == 3 then
        r = p * 255.0
        g = q * 255.0
        b = V * 255.0
      elseif Hi == 4 then
        r = t * 255.0
        g = p * 255.0
        b = V * 255.0
      elseif Hi == 5 then
        r = V * 255.0
        g = p * 255.0
        b = q * 255.0
      else
        error("math.floor(H % 360 / 60) should be [0, 6), is " .. tostring(Hi) .. "?")
      end
    end
    return r, g, b
  end,
  HSL_to_RGB = function(H, S, L)
    local r, g, b
    H = math.abs(H) % 360
    S = clamp(S, 0, 1)
    L = clamp(L, 0, 1)
    if S == 0 then
      r = L
      g = L
      b = L
    else
      local Q
      if L < 0.5 then
        Q = L * (1.0 + S)
      else
        Q = L + S - (L * S)
      end
      local P = 2.0 * L - Q
      local Hk = H / 360
      local Tr, Tg, Tb
      if Hk < 1 / 3 then
        Tr = Hk + 1 / 3
        Tg = Hk
        Tb = Hk + 2 / 3
      elseif Hk > 2 / 3 then
        Tr = Hk - 2 / 3
        Tg = Hk
        Tb = Hk - 1 / 3
      else
        Tr = Hk + 1 / 3
        Tg = Hk
        Tb = Hk - 1 / 3
      end
      local get_component
      get_component = function(T)
        if T < 1 / 6 then
          return P + ((Q - P) * 6.0 * T)
        elseif 1 / 6 <= T and T < 1 / 2 then
          return Q
        elseif 1 / 2 <= T and T < 2 / 3 then
          return P + ((Q - P) * (2 / 3 - T) * 6.0)
        else
          return P
        end
      end
      r = get_component(Tr)
      g = get_component(Tg)
      b = get_component(Tb)
    end
    return math.floor(r * 255 + 0.5), math.floor(g * 255 + 0.5), math.floor(b * 255 + 0.5)
  end,
  trim = function(s)
    return s:gsub('^%s*(.-)%s*$', '%1')
  end,
  headtail = function(s)
    local a, b, head, tail = s:find('(.-)%s+(.*)')
    if a then
      return head, tail
    else
      return s, ''
    end
  end,
  words = function(s)
    return function()
      if s == '' then
        return 
      end
      local head, tail = string.headtail(s)
      s = tail
      return head
    end
  end,
  clamp = function(val, min, max)
    if val < min then
      return min
    elseif val > max then
      return max
    else
      return val
    end
  end,
  interpolate = function(pct, min, max)
    if pct <= 0 then
      return min
    elseif pct >= 1 then
      return max
    else
      return pct * (max - min) + min
    end
  end,
  interpolate_color = function(pct, first, last)
    local r1, g1, b1 = extract_color(first)
    local r2, g2, b2 = extract_color(last)
    local r, g, b = interpolate(pct, r1, r2), interpolate(pct, g1, g2), interpolate(pct, b1, b2)
    return ass_color(r, g, b)
  end,
  interpolate_alpha = function(pct, first, last)
    return ass_alpha(interpolate(pct, select(4, extract_color(first)), select(4, extract_color(last))))
  end
}
return utils
