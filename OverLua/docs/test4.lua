function render_frame(f, t)
	-- Some data
	local basex, basey = 100, 100
	local thetext = overlua_datastring or "OverLua R0xx0rz?"

	-- Create a new blank surface to draw on
	local surf = cairo.image_surface_create(f.width, f.height, "argb32")
	-- And a context to go with it. Drawing happens through the context.
	local c = surf.create_context()
	
	-- The source video is 16:9 anamorphic, so create a scaling matrix
	local scaling = cairo.matrix_create()
	scaling.init_scale(f.width/853, f.height/480)
	-- And set it for the surface
	c.set_matrix(scaling)
	
	-- Create a surface of the video frame...
	local fs = f.create_cairo_surface()
	-- .. and blur it a bit
	raster.gaussian_blur(fs, 2)
	-- Then create a pattern from it, that can be used for filling shapes with
	local fspattern = cairo.pattern_create_for_surface(fs)
	-- Also apply the scaling matrix to the pattern
	fspattern.set_matrix(scaling)
	
	-- Prepare a few things
	c.select_font_face("Gill Sans Std UltraBold", "", "bold")
	c.set_font_size(42)
	c.set_line_width(4)
	c.set_line_join("round")
	
	-- First stroke the text a bit above and to the left of main
	c.move_to(basex-2, basey-2)
	c.set_source_rgba(0.8, 0.8, 1, 0.8)
	c.text_path(thetext)
	c.stroke()
	
	-- Then a bit below and to the right, so it overlaps the first stroking a bit
	c.move_to(basex+2, basey+2)
	c.set_source_rgba(0.4, 0.4, 0.5, 0.8)
	c.text_path(thetext)
	c.stroke()
	
	-- And blur them both together, also softening the border
	raster.gaussian_blur(surf, 3)
	
	-- Now prepare the main text fill and the real location
	c.move_to(basex, basey)
	c.text_path(thetext)
	-- First a thing black outline
	c.set_source_rgba(0, 0, 0, 1)
	c.set_line_width(2)
	c.stroke_preserve() -- _preserve version to keep the outline
	-- Then use the blurred video frame as fill source
	c.set_source(fspattern)
	c.fill_preserve()
	-- And create a very dark colour
	-- Adding this on top of the first fill will create a reddening effect
	c.set_source_rgba(0.1, 0, 0, 1)
	c.set_operator("add")
	c.fill()
	
	-- Finally draw the surface onto the video
	f.overlay_cairo_surface(surf, 0, 0)
end
