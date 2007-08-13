function render_frame(f, t)
	local surf = f.create_cairo_surface()
	--raster.gaussian_blur(surf, t)--1+(1-math.cos(t*10))*2)
	--raster.invert(surf)
	raster.separable_filter(surf, {-1, 3, -1}, 1) 
	f.overlay_cairo_surface(surf, 0, 0)
end
