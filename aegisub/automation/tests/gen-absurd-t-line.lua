script_name = "Absurdness"
script_description = "Benchmark renderers by testing how they react to an absurd number of \\t tags in one line, and an absurd amount of lines each with a single \\t."
script_author = "jfs"

absurd = 1000

ttext = "\\1a&H00&\\2a&H00&\\3a&H00&\\4a&H00&"

function gen_one_absurd(subs, sel)
	local l = subs[sel[1]]
	l.text = ""
	l.start_time = 0
	l.end_time = (absurd+1)*100
	for i = 1, absurd do
		l.text = l.text .. string.format("{\\t(%d,%d,%s)}", i*100, (i+1)*100, ttext)
	end
	l.text = l.text .. "a"
	subs[-sel[1]] = l
	aegisub.set_undo_point("absurdness 1")
end

function gen_absurd_many(subs, sel)
	local l = subs[sel[1]]
	for i = 1, absurd do
		l.start_time = i*100
		l.end_time = (i+1)*100
		l.text = string.format("{\\t(0,100,%s)}a", ttext)
		subs[-sel[1]] = l
	end
	aegisub.set_undo_point("absurdness 2")
end

aegisub.register_macro("Generate absurd line", "Absurd", gen_one_absurd)
aegisub.register_macro("Generate absurdly many lines", "Absurd", gen_absurd_many)
