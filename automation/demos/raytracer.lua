-- This script is given to the public domain
-- It was originally intended as an april fool's joke in 2007, though it never really caught on.
-- You're encouraged to experiment with this if you have plenty of time.
-- Even if it's made as a joke, it does show that "anything is possible with ASS, although some things are insane to try."

script_name = "Raytracer"
script_description = "Reads subtitles as a scene description and raytraces the scene"
script_author = "jfs"
script_version = tostring(math.pi)

include("utils.lua")

max_iter = 3

function raytrace(subs)
	aegisub.progress.task("Reading scene...")
	local lights, tris, camera, xres, yres = read_scene(subs)
	
	aegisub.progress.task("Raytracing...")
	local curp, totalp = 0, xres*yres
	for y = 0, yres-1 do
		aegisub.progress.task(string.format("Raytracing, line %d/%d...", y+1, yres))
		for x = 0, xres-1 do
			aegisub.progress.set(curp/totalp*100)
			local l = trace_point(x, y, (x+0.5)/xres, (y+0.5)/yres, lights, tris, camera)
			if l then
				subs.append(l)
			end
			curp = curp + 1
		end
	end
	
	aegisub.progress.task("Done.")
	aegisub.progress.set(100)
end


function trace_point(px, py, x, y, lights, tris, camera)
	-- fixme, assume a camera here ignoring defined one
	local vec = vector.norm( { 2*x-1, 1-2*y, -1 } )
	
	local r, g, b = trace_vec({0,0,-1}, vec, lights, tris, 0)
	if not r then
		return nil
	end
	
	r, g, b = clamp(r, 0, 255), clamp(g, 0, 255), clamp(b, 0, 255)
	
	-- todo, make line
	local l = {
		class = "dialogue",
		section = "Events",
		comment = false,
		layer = 0,
		start_time = 0,
		end_time = 3600*1000, -- one hour
		style = "p",
		actor = "",
		margin_l = 0,
		margin_r = 0,
		margin_t = 0,
		margin_b = 0,
		effect = "",
		text = string.format("{\\pos(%d,%d)\\1c&H%02x%02x%02x&\\p1}m 0 0 l 1 0 1 1 0 1", px, py, r, g, b)
	}
	return l
end


function trace_vec(org, vec, lights, tris, iter)
	if iter > max_iter then
		return 0, 0, 0
	end
	
	local hit = find_intersect(org, vec, tris)
	if not hit then
		return nil
	end
	
	-- got intersection, calculate lighting
	local r, g, b = hit.t.c.r*10, hit.t.c.g*10, hit.t.c.b*10
	local ray_cos_theta = vector.dot(hit.t.n, vec)
	hit.p = hit.t.p[1]
	hit.p = vector.add(hit.p, vector.scale(vector.sub(hit.t.p[2], hit.t.p[1]), hit.u))
	hit.p = vector.add(hit.p, vector.scale(vector.sub(hit.t.p[3], hit.t.p[1]), hit.v))
	for i, l in pairs(lights) do
		-- shadow ray
		local lvec = vector.sub(l.p, hit.p)
		local shadow = find_intersect(hit.p, lvec, tris)
		if not shadow or (shadow and (shadow.dist < 0 or shadow.dist > 1)) then
			-- not in shadow
			local lvecs = vector.len(lvec)
			-- diffuse component
			local light_cos_theta = math.abs(vector.dot(hit.t.n, lvec))
			-- specular component
			local cos_alpha = vector.dot(vector.sub(vector.scale(hit.t.n, 2*light_cos_theta), lvec), vec)
			local cos_n_alpha = cos_alpha^3 -- arbitrary constant for now
			-- add up
			r = r + l.c.r*hit.t.c.r * (light_cos_theta*0.6 + cos_n_alpha*0.4) / math.max(lvecs,1)
			g = g + l.c.g*hit.t.c.g * (light_cos_theta*0.6 + cos_n_alpha*0.4) / math.max(lvecs,1)
			b = b + l.c.b*hit.t.c.b * (light_cos_theta*0.6 + cos_n_alpha*0.4) / math.max(lvecs,1)
		end
	end
	
	-- reflection
	local rvec = vector.sub(vector.scale(hit.t.n, 2*vector.dot(hit.t.n, vec)), vec)
	local rr, rg, rb = trace_vec(hit.p, rvec, lights, tris, iter+1)
	if not rr then
		rr, rg, rb = 0, 0, 0
	end
	r = r*0.75 + rr*0.25
	g = g*0.75 + rg*0.25
	b = b*0.75 + rb*0.25
	
	return r, g, b
end


function find_intersect(org, vec, tris)
	local intersec = nil
	-- find closest intersection
	for i, t in pairs(tris) do
		local dist, u, v = intersect_triangle(org, vec, t)
		if dist and dist > 0 then
			if not intersec or intersec.dist > dist then
				intersec = {dist=dist, u=u, v=v, t=t}
			end
		end
	end
	return intersec
end


function intersect_triangle(org, vec, triangle)
	-- taken from http://www.graphics.cornell.edu/pubs/1997/MT97.html
	-- find vectors for two edges sharing point 1
	local edge1, edge2 = vector.sub(triangle.p[2], triangle.p[1]), vector.sub(triangle.p[3], triangle.p[1])
	
	-- begin calculating determinant - also used to calculate U parameter
	local pvec = vector.cross(vec, edge2)
	-- if determinant is near zero, ray lies in plane of triangle
	local det = vector.dot(edge1, pvec)
	if det > -0.00001 and det < 0.00001 then
		-- parallel to plane
		return nil
	end
	local inv_det = 1 / det
	
	-- calculate distance from point 1 to ray origin
	local tvec = vector.sub(org, triangle.p[1])
	
	-- calculate U parameter and test bounds
	local u = vector.dot(tvec, pvec) * inv_det
	if u < 0 or u > 1 then
		-- crosses plane but outside triangle
		return nil
	end
	
	-- prepare to test V parameter
	local qvec = vector.cross(tvec, edge1)
	-- calculate V parameter and test bounds
	local v = vector.dot(vec, qvec) * inv_det
	if v < 0 or (u+v) > 1 then
		-- crosses plane but outside triangle
		return nil
	end
	
	-- calculate distance, ray intersects triangle
	local dist = vector.dot(triangle.p[3], qvec)
	
	return dist, u, v
end


function read_scene(subs)
	local lights = {}
	local tris = {}
	local camera = { pos = {0,0,-1}, up = {0,1,0}, plane } -- fixme
	local xres, yres = 384, 288

	local style = {
			class = "style",
			section = "V4+ Styles",
			name = "p",
			fontname = "Arial",
			fontsize = "20",
			color1 = "&H00000000&",
			color2 = "&H00000000&",
			color3 = "&H00000000&",
			color4 = "&H00000000&",
			bold = false,
			italic = false,
			underline = false,
			strikeout = false,
			scale_x = 100,
			scale_y = 100,
			spacing = 0,
			angle = 0,
			borderstyle = 0,
			outline = 0,
			shadow = 0,
			align = 5,
			margin_l = 0,
			margin_r = 0,
			margin_t = 0,
			margin_b = 0,
			encoding = 0
		}
	
	local i, maxi = 1, #subs
	local replaced_style = false
	while i < maxi do
		aegisub.progress.set(i / maxi * 100)
		local l = subs[i]
		if l.class == "dialogue" then
			parse_line(l, lights, tris, camera)
			subs.delete(i)
			maxi = maxi - 1
		elseif l.class == "style" then
			if replaced_style then
				subs.delete(i)
				maxi = maxi - 1
			else
				style.section = l.section
				subs[i] = style
				replaced_style = true
				i = i + 1
			end
		elseif l.class == "info" then
			local k = l.key:lower()
			if k == "playresx" then
				xres = math.floor(l.value)
			elseif k == "playresy" then
				yres = math.floor(l.value)
			end
			i = i + 1
		else
			i = i + 1
		end
	end
	
	return lights, tris, camera, xres, yres
end


function parse_line(line, lights, tris, camera)
	local val, rest = string.headtail(line.text)
	
	if val == "light" then
		local pos, color = {}, {}
		val, rest = string.headtail(rest)
		pos[1] = tonumber(val)
		val, rest = string.headtail(rest)
		pos[2] = tonumber(val)
		val, rest = string.headtail(rest)
		pos[3] = tonumber(val)
		
		-- these work as intensity values so they should probably be high
		val, rest = string.headtail(rest)
		color.r = tonumber(val) or 0
		val, rest = string.headtail(rest)
		color.g = tonumber(val) or 0
		val, rest = string.headtail(rest)
		color.b = tonumber(val) or 0
		
		local light = {
			p = pos,
			c = color
		}
		table.insert(lights, light)
		
	elseif val == "tri" then
		local coord1, coord2, coord3, color = {}, {}, {}, {}

		val, rest = string.headtail(rest)
		coord1[1] = tonumber(val)
		val, rest = string.headtail(rest)
		coord1[2] = tonumber(val)
		val, rest = string.headtail(rest)
		coord1[3] = tonumber(val)

		val, rest = string.headtail(rest)
		coord2[1] = tonumber(val)
		val, rest = string.headtail(rest)
		coord2[2] = tonumber(val)
		val, rest = string.headtail(rest)
		coord2[3] = tonumber(val)
		
		val, rest = string.headtail(rest)
		coord3[1] = tonumber(val)
		val, rest = string.headtail(rest)
		coord3[2] = tonumber(val)
		val, rest = string.headtail(rest)
		coord3[3] = tonumber(val)
		
		-- these work as reflectivity values so they should be in range 0..1
		val, rest = string.headtail(rest)
		color.r = tonumber(val) or 0
		val, rest = string.headtail(rest)
		color.g = tonumber(val) or 0
		val, rest = string.headtail(rest)
		color.b = tonumber(val) or 0
		
		local t = {
			p = {coord1, coord2, coord3},
			n = vector.norm(vector.normal(coord1, coord2, coord3)),
			c = color
			}
		
		table.insert(tris, t)
		
	elseif val == "camera" then
		-- fixme, redefine
		val, rest = string.headtail(rest)
		camera.pos[1] = tonumber(val)
		val, rest = string.headtail(rest)
		camera.pos[2] = tonumber(val)
		val, rest = string.headtail(rest)
		camera.pos[3] = tonumber(val)

		val, rest = string.headtail(rest)
		camera.plane[1][1] = tonumber(val)
		val, rest = string.headtail(rest)
		camera.plane[1][2] = tonumber(val)
		val, rest = string.headtail(rest)
		camera.plane[1][3] = tonumber(val)
		
		val, rest = string.headtail(rest)
		camera.plane[2][1] = tonumber(val)
		val, rest = string.headtail(rest)
		camera.plane[2][2] = tonumber(val)
		val, rest = string.headtail(rest)
		camera.plane[2][3] = tonumber(val)
		
		camera.start_time = line.start_time
		camera.end_time = line.end_time
		
	else
		-- unknown, ignore
	end
end


vector = {}

vector.null = {0,0,0}

function vector.add(v1, v2)
	local r = {}
	r[1] = v1[1] + v2[1]
	r[2] = v1[2] + v2[2]
	r[3] = v1[3] + v2[3]
	return r
end

function vector.sub(v1, v2) -- v1 minus v2
	local r = {}
	r[1] = v1[1] - v2[1]
	r[2] = v1[2] - v2[2]
	r[3] = v1[3] - v2[3]
	return r
end

function vector.scale(v, s)
	local r = {}
	r[1] = v[1] * s
	r[2] = v[2] * s
	r[3] = v[3] * s
	return r
end

function vector.len(v)
	return math.sqrt(v[1]*v[1] + v[2]*v[2] + v[3]*v[3])
end

function vector.norm(v)
	local r, il = {}, 1/vector.len(v)
	r[1] = v[1]*il
	r[2] = v[2]*il
	r[3] = v[3]*il
	return r
end

function vector.dot(v1, v2)
	return v1[1]*v2[1] + v1[2]*v2[2] + v1[3]*v2[3]
end

function vector.cross(v1, v2)
	local r = {}
	r[1] = v1[2]*v2[3] - v1[3]*v2[2]
	r[2] = v1[1]*v2[3] - v1[3]*v2[1]
	r[3] = v1[1]*v2[2] - v1[2]*v2[1]
	return r
end

function vector.normal(p1, p2, p3)
	return vector.cross(vector.sub(p2, p1), vector.sub(p3, p1))
end


function raytrace_macro(subs)
	raytrace(subs)
	aegisub.set_undo_point("raytracing")
end

aegisub.register_macro("Raytrace!", "Raytrace the scene", raytrace_macro)
aegisub.register_filter("Raytrace", "Raytrace the scene", 2000, raytrace)
