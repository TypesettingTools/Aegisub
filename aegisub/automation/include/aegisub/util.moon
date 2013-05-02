-- Copyright (c) 2005-2010, Niels Martin Hansen, Rodrigo Braz Monteiro
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

sformat = string.format

-- Make a shallow copy of a table
copy = (tbl) -> {k, v for k, v in pairs tbl}

-- Make a deep copy of a table
-- Retains equality of table references inside the copy and handles self-referencing structures
deep_copy = (tbl) ->
  seen = {}
  copy = (val) ->
    return val if type(tbl) != 'table'
    return seen[val] if seen[tbl]
    seen[val] = tbl
    {k, copy(v) for k, v in pairs val}
  copy tbl

-- Generates ASS hexadecimal string from R, G, B integer components, in &HBBGGRR& format
ass_color = (r, g, b) -> sformat "&H%02X%02X%02X&", b, g, r
-- Format an alpha-string for \Xa style overrides
ass_alpha = (a) -> sformat "&H%02X&", a
-- Format an ABGR string for use in style definitions (these don't end with & either)
ass_style_color = (r, g, b, a) -> sformat "&H%02X%02X%02X%02X", a, b, g, r

-- Extract colour components of an ASS colour
extract_color = (s) ->
  local a, b, g, r

  -- Try a style first
  a, b, g, r = s\match '&H(%x%x)(%x%x)(%x%x)(%x%x)'
  if a then
    return tonumber(r, 16), tonumber(g, 16), tonumber(b, 16), tonumber(a, 16)

  -- Then a colour override
  b, g, r = s\match '&H(%x%x)(%x%x)(%x%x)&'
  if b then
    return tonumber(r, 16), tonumber(g, 16), tonumber(b, 16), 0

  -- Then an alpha override
  a = s\match '&H(%x%x)&'
  if a then
    return 0, 0, 0, tonumber(a, 16)

  -- Ok how about HTML format then?
  r, g, b, a = s\match '#(%x%x)(%x?%x?)(%x?%x?)(%x?%x?)'
  if r then
    return tonumber(r, 16), tonumber(g, 16) or 0, tonumber(b, 16) or 0, tonumber(a, 16) or 0

-- Create an alpha override code from a style definition colour code
alpha_from_style = (scolor) -> ass_alpha select 4, extract_color scolor

-- Create an colour override code from a style definition colour code
color_from_style = (scolor) ->
  r, g, b = extract_color scolor
  ass_color r or 0, g or 0, b or 0

-- Converts HSV (Hue, Saturation, Value) to RGB
HSV_to_RGB = (H, S, V) ->
  r, g, b = 0, 0, 0

  -- Saturation is zero, make grey
  if S == 0
    r = @clamp(V*255, 0, 255)
    g = r
    b = r

  -- Else, calculate color
  else
    -- Calculate subvalues
    H = math.abs(H) % 360 -- Put H in range [0, 360)
    Hi = math.floor(H/60)
    f = H/60.0 - Hi
    p = V*(1-S)
    q = V*(1-f*S)
    t = V*(1-(1-f)*S)

    -- Do math based on hue index
    if Hi == 0
      r = V*255.0
      g = t*255.0
      b = p*255.0
    elseif Hi == 1
      r = q*255.0
      g = V*255.0
      b = p*255.0
    elseif Hi == 2
      r = p*255.0
      g = V*255.0
      b = t*255.0
    elseif Hi == 3
      r = p*255.0
      g = q*255.0
      b = V*255.0
    elseif Hi == 4
      r = t*255.0
      g = p*255.0
      b = V*255.0
    elseif Hi == 5
      r = V*255.0
      g = p*255.0
      b = q*255.0
    else
      error "math.floor(H % 360 / 60) should be [0, 6), is #{Hi}?"

  return r, g, b

-- Convert HSL (Hue, Saturation, Luminance) to RGB
-- Contributed by Gundamn
HSL_to_RGB = (H, S, L) ->
  local r, g, b

  -- Make sure input is in range
  H = math.abs(H) % 360
  S = clamp(S, 0, 1)
  L = clamp(L, 0, 1)

  if S == 0 -- Simple case if saturation is 0, all grey
    r = L
    g = L
    b = L
  else
    -- More common case, saturated colour
    Q = if L < 0.5
      L * (1.0 + S)
    else
      L + S - (L * S)

    P = 2.0 * L - Q

    Hk = H / 360

    local Tr, Tg, Tb
    if Hk < 1/3
      Tr = Hk + 1/3
      Tg = Hk
      Tb = Hk + 2/3
    elseif Hk > 2/3
      Tr = Hk - 2/3
      Tg = Hk
      Tb = Hk - 1/3
    else
      Tr = Hk + 1/3
      Tg = Hk
      Tb = Hk - 1/3

    get_component = (T) ->
      if T < 1/6
        P + ((Q - P) * 6.0 * T)
      elseif 1/6 <= T and T < 1/2
        Q
      elseif 1/2 <= T and T < 2/3
        P + ((Q - P) * (2/3 - T) * 6.0)
      else
        P

    r = get_component(Tr)
    g = get_component(Tg)
    b = get_component(Tb)

  return math.floor(r*255+0.5), math.floor(g*255+0.5), math.floor(b*255+0.5)

-- Removes spaces at the start and end of string
trim = (s) -> s\gsub '^%s*(.-)%s*$', '%1'

-- Get the 'head' and 'tail' of a string, treating it as a sequence of words separated by one or more space-characters
headtail = (s) ->
  a, b, head, tail = s\find '(.-)%s+(.*)'
  if a then head, tail else s, ''

-- Iterator function for headtail
words = (s) -> ->
  return if s == ''
  head, tail = headtail s
  s = tail
  head

-- Clamp a number value to a range
clamp = (val, min, max) ->
  if val < min then min elseif val > max then max else val

-- Interpolate between two numbers
interpolate = (pct, min, max) ->
  if pct <= 0 then min elseif pct >= 1 then max else pct * (max - min) + min

-- Interpolate between two colour values, given in either style definition or style override format
-- Return in style override format
interpolate_color = (pct, first, last) ->
  r1, g1, b1 = extract_color first
  r2, g2, b2 = extract_color last
  r, g, b = interpolate(pct, r1, r2), interpolate(pct, g1, g2), interpolate(pct, b1, b2)
  ass_color r, g, b

-- Interpolate between two alpha values, given either in style override or as part as a style definition colour
-- Return in style override format
interpolate_alpha = (pct, first, last) ->
  ass_alpha interpolate pct, select(4, extract_color first), select(4, extract_color last)

{ :copy, :deep_copy, :ass_color, :ass_alpha, :ass_style_color,
  :extract_color, :alpha_from_style, :color_from_style, :HSV_to_RGB,
  :HSL_to_RGB, :trim, :headtail, :words, :clamp, :interpolate,
  :interpolate_color, :interpolate_alpha }
