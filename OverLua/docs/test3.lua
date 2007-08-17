function render_frame(f, t)
	local surf = f.create_cairo_surface()
	--raster.gaussian_blur(surf, t)--1+(1-math.cos(t*10))*2)
	--raster.invert(surf)
	--raster.separable_filter(surf, {-1, 3, -1}, 1) 
	--raster.directional_blur(surf, t, t/10)
	raster.radial_blur(surf, 200, 200, t/60)
	raster.pixel_value_map(surf, "0 =R 1 G - =G")
	f.overlay_cairo_surface(surf, 0, 0)
	
	surf = cairo.image_surface_create(200, 200, "rgb24")
	raster.pixel_value_map(surf, "rand =t0 t0 =R t0 =G t0 =B")
	f.overlay_cairo_surface(surf, 20, 20)
end
