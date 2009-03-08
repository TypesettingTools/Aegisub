-- This script is given to the public domain.
-- You can use and modify the effect freely, also without credit, although I would appreciate some.

include("karaskel.lua")

script_name = "Future Windy Blur"
script_description = "Highlights blown away by the winds of change."
script_author = "jfs"
script_version = "1.4"

function new_windy_blur(subs)
	aegisub.progress.task("Getting header data...")
	local meta, styles = karaskel.collect_head(subs)
	
	aegisub.progress.task("Applying effect...")
	local i, ai, maxi, maxai = 1, 1, #subs, #subs
	while i <= maxi do
		aegisub.progress.task(string.format("Applying effect (%d/%d)...", ai, maxai))
		aegisub.progress.set((ai-1)/maxai*100)
		local l = subs[i]
		if l.class == "dialogue" and
				not l.comment then
			karaskel.preproc_line(subs, meta, styles, l)
			do_fx(subs, meta, l)
			maxi = maxi - 1
			subs.delete(i)
		else
			i = i + 1
		end
		ai = ai + 1
	end
	aegisub.progress.task("Finished!")
	aegisub.progress.set(100)
end

function do_fx(subs, meta, line)
	local l = table.copy(line)
	l.text = string.format("{\\pos(%d,%d)\\an5\\1c%s\\1a&HFF&\\3a&HFF&\\4a&HFF&\\t(0,200,\\1a&H00&\\3a&H00&\\4a&H00&)\\t(%d,%d,\\1a&HFF&\\3a&HFF&\\4a&HFF&)}%s", line.center, line.middle, line.styleref.color2, line.duration+200, line.duration+400, line.text_stripped)
	l.start_time = l.start_time - 200
	l.end_time = l.end_time + 200
	l.layer = 1
	subs.append(l)
	
	for i = 1, line.kara.n do
		local syl = line.kara[i]
		if syl.duration > 0 then
			local l = table.copy(line)
			l.start_time = line.start_time + syl.start_time
			if syl.duration < 750 then
				l.end_time = l.start_time + 1500
			else
				l.end_time = l.start_time + syl.duration * 1.5
			end
			l.layer = 2
			local temp = string.format("{\\be1\\move(%d,%d,%%d,%%d)\\an5\\1a&Hd0\\bord0\\shad0\\t(0.6,\\1a&HFF&)}%s", line.left+syl.center, line.middle, syl.text_stripped)
			for j = -8, 8 do
				l.text = string.format(temp, line.left+syl.center+math.cos(math.rad(j*4))*30, line.middle+math.sin(math.rad(j*4))*30)
				subs.append(l)
			end
		end
	end
end

aegisub.register_filter("Future Windy Blur", "", 2000, new_windy_blur)
