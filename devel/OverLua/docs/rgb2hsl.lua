--- Convert RGB colour to HSL (hue/saturation/luminance)
-- The output image will have H instead of R,
-- S instead of G and L instead of B.

function render_frame(f, t)
	local w, h = f.width, f.height
	for y = 0, h-1 do
		for x = 0, w-1 do
			local r, g, b = f(x, y)
			local h, s, l = RGB2HSL(r, g, b)
			f[y*w+x] = {h, s, l}
		end
	end
end

function clip_colorval(v)
	if v < 0 then
		return 0
	elseif v > 255 then
		return 255
	else
		return math.floor(v)
	end
end

function RGB2HSL(R, G, B)
	local r, g, b = R/255, G/255, B/255
	local h, s, l
	
	local minrgb, maxrgb = math.min(r, math.min(g, b)), math.max(r, math.max(g, b))
	
	l = (minrgb + maxrgb) / 2
	
	if minrgb == maxrgb then
		h, s = 0, 0
	else
		if l < 0.5 then
			s = (maxrgb - minrgb) / (maxrgb + minrgb)
		else
			s = (maxrgb - minrgb) / (2 - maxrgb - minrgb)
		end
		if r == maxrgb then
			h = (g - b) / (maxrgb - minrgb) + 0
		elseif g == maxrgb then
			h = (b - r) / (maxrgb - minrgb) + 2
		else
			h = (r - g) / (maxrgb - minrgb) + 4
		end
	end
	
	if h < 0 then h = h + 6 end
	if h >= 6 then h = h - 6 end
	
	return clip_colorval(h*255/6), clip_colorval(s*255), clip_colorval(l*255)
end
