function render_frame(f, t)
	local w, h = f.width, f.height
	for x = 0, 20 do
		for y = 0, 20 do
			f[y*w+x] = {x*10,y*10,255}
		end
	end
	for x = 21, 40 do
		for y = 0, h-1 do
			local r, g, b = f(x,y)
			f[y*w+x] = {255-r, 255-g, 255-b}
		end
	end
end
